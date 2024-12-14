#include <sstream>
#include <stdexcept>
#include <iostream>
#include <unistd.h>
#include <regex>

#include "./redis_parser.hpp"
#include "./globals.h"
#include "./utils.hpp"


std::string RedisParser::parseRESPCommand(const std::string& input) {
    // std::cout << "Received Bytes: " << input << std::endl;
    size_t pos = 0;

    // Parse array indicator, e.g., "*2\r\n"
    if (input[pos] != '*') {
        return "-ERR protocol error: expected array";
    }
    pos++;

    // Read number of elements
    size_t elements_end = input.find("\r\n", pos);
    int num_elements = std::stoi(input.substr(pos, elements_end - pos));
    pos = elements_end + 2;

    // Parse command bulk string, e.g., "$4\r\nECHO\r\n"
    std::string command = parseBulkString(input, pos);

    // Convert command to uppercase to make it case-insensitive
    for (auto& c : command) c = std::toupper(c);

    if (command == "ECHO"){
        return parseECHOCommand(input, pos);
    }
    else if (command == "PING"){
        return parsePINGCommand(input, pos);
    }
    else if (command == "SET"){
        return parseSETCommand(input, pos, num_elements);
    }
    else if (command == "GET"){
        return parseGETCommand(input, pos);
    }
    else if (command == "CONFIG"){
        return parseCONFIGGETCommand(input, pos, num_elements);
    }
    else if (command == "KEYS"){
        return parseKEYSCommand(input, pos);
    }
    else if (command == "INFO"){
        return parseINFOCommand(input, pos);
    }
    else {
        return "-ERR unknown command '" + command + "'";
    }

    
}


// Parses an INFO Command
std::string RedisParser::parseINFOCommand(const std::string& input, size_t& pos){
    std::string command = parseBulkString(input, pos);
    for (auto& c : command) c = std::toupper(c);
    std::string response = "";
    if (command == "REPLICATION"){
        ServerInfo server_info = db_handler.get_server_info();
        response = response + "role:" + server_info.get_role() + "\r";
        response = response + "connected_slaves:" + std::to_string(server_info.get_connected_slaves()) + "\r";
        response = response + "master_replid:" + server_info.get_master_replid() + "\r";
        response = response + "master_repl_offset:" + std::to_string(server_info.get_master_repl_offset()) + "\r";
        response = response + "second_repl_offset:" + std::to_string(server_info.get_second_repl_offset()) + "\r";
        response = response + "repl_backlog_active:" + std::to_string(server_info.get_repl_backlog_active()) + "\r";
        response = response + "repl_backlog_size:" + std::to_string(server_info.get_repl_backlog_size()) + "\r";
        response = response + "repl_backlog_first_byte_offset:" + std::to_string(server_info.get_repl_backlog_first_byte_offset()) + "\r";
        response = response + "repl_backlog_histlen:" + std::to_string(server_info.get_repl_backlog_histlen()) + "\r";
    }
    return create_string_reponse(response);
}

// Parses a KEYS Command
std::string RedisParser::parseKEYSCommand(const std::string& input, size_t& pos){
    std::string to_match = parseBulkString(input, pos);
    std::string key_to_match = "";
    std::regex pattern;
    int len = to_match.size();
    if (to_match[len-1] == '*'){
        key_to_match = to_match.substr(0,len-1);
        pattern = std::regex("(" + key_to_match + ")(.*)");
    }
    else {
        std::regex pattern(to_match);
    }
    std::vector<std::string> response_array;
    for (auto pair: db_handler.database){
        std::string key = pair.first;
        if (std::regex_match(key, pattern)){
            response_array.push_back(key);
        }
    }
    return create_array_reponse(response_array);
}



// Parses a CONFIG GET Command
std::string RedisParser::parseCONFIGGETCommand(const std::string& input, size_t& pos, int num_elements){
    std::string command = parseBulkString(input, pos);
    for (auto& c : command) c = std::toupper(c);
    if (command == "GET"){
        std::string config_variable = parseBulkString(input, pos);
        if (config_variable == "dir"){
            return create_array_reponse({config_variable, config::dir});
        }
        else if (config_variable == "dbfile"){
            return create_array_reponse({config_variable, config::dbfilename});
        }
    }
    else{
        std::cerr << "Error: Config Followed by unknown Command" << command << std::endl;
    }
    return "";
}

// Parses a GET Command
std::string RedisParser::parseGETCommand(const std::string& input, size_t& pos){
    std::string argument = parseBulkString(input, pos);
    std::string response = db_handler.get(argument);
    if (response != NULL_RESPONSE){
        return create_string_reponse(response);
    }
    return response;
}

//Parses a SET Command
std::string RedisParser::parseSETCommand(const std::string& input, size_t& pos, 
                                        int num_elements){
    std::string argument1 = parseBulkString(input, pos);
    std::string argument2 = parseBulkString(input, pos);
    long expiry_time = 0;
    // TODO: Implement error handling for cases that px was incorrect 
    // or the time value is not provided
    // Currently Assumed that PX and expiry time are valid commands provided
    if (num_elements > 3){
        std::string expiry_command = parseBulkString(input, pos);
        std::string expiry_value = parseBulkString(input, pos);
        auto current_time = get_current_time_milli();
        expiry_time = current_time + std::stoi(expiry_value);
    }
    db_handler.set(argument1, argument2, expiry_time);
    return "+OK\r\n";
}

// Parses an ECHO Command
std::string RedisParser::parseECHOCommand(const std::string& input, size_t& pos){
    // Parse argument bulk string, e.g., "$9\r\nraspberry\r\n"
    std::string argument = parseBulkString(input, pos);

    // Formulate the RESP response for the ECHO command (bulk string format)
    return create_string_reponse(argument);
}


// Parses a PING Command
std::string RedisParser::parsePINGCommand(const std::string& input, size_t& pos){
    return "+PONG\r\n";
}


// Parses a bulk string in RESP format, e.g., "$9\r\nraspberry\r\n"
std::string RedisParser::parseBulkString(const std::string& input, size_t& pos) {
    if (input[pos] != '$') {
        throw std::runtime_error("protocol error: expected bulk string");
    }
    pos++;

    // Find length of the bulk string
    size_t length_end = input.find("\r\n", pos);
    int length = std::stoi(input.substr(pos, length_end - pos));
    pos = length_end + 2;

    // Extract the bulk string value
    std::string bulk_string = input.substr(pos, length);
    pos += length + 2; // Move position past the bulk string and trailing "\r\n"

    return bulk_string;
}



// Helper Functions


// Create array response for the client
std::string RedisParser::create_array_reponse(const std::vector<std::string>& response_arr){
    std::string response = "";
    for (int i = 0; i < response_arr.size(); i++){
        response += create_string_reponse(response_arr[i]);
    }
    return "*" + std::to_string(response_arr.size()) + "\r\n" + response;
}

// Create the response for the client
std::string RedisParser::create_string_reponse(const std::string& response){
    return "$" + std::to_string(response.size()) + "\r\n" + response + "\r\n"; 
}

