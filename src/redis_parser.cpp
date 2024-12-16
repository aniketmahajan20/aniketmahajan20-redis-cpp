#include <stdexcept>
#include <iostream>
#include <unistd.h>
#include <regex>
#include <fstream>
#include <thread>

#include "../include/redis/redis_parser.hpp"
#include "../include/redis/globals.h"
#include "../include/redis/utils.hpp"

std::thread RedisParser::parseRESPCommand_thread(const std::string& input){
    return std::thread([this, input] {RedisParser::parseRESPCommand(input);});
}

void RedisParser::parseRESPCommand(const std::string& input) {
    // std::cout << "Received Bytes: " << input << std::endl;
    size_t pos = 0;

    // Parse array indicator, e.g., "*2\r\n"
    if (input[pos] != '*') {
        this->response_buf = "-ERR protocol error: expected array";
        response_ready = true;
        this->communication_over();
        return;
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
    else if (command == "REPLCONF"){
        return parseREPLCONFCommand(input, pos);
    }
    else if (command == "PSYNC"){
        return parsePSYNCCommand(input, pos);
    }
    else {
        this->response_buf = "-ERR unknown command '" + command + "'";
        response_ready = true;
        this->communication_over();
        return;
    }    
}

// Parses a PSYNC Command
void RedisParser::parsePSYNCCommand(const std::string& input, size_t& pos){
    ServerInfo server_info = db_handler.get_server_info();
    // Hardcoding the response as suggested by the requirements
    this->response_buf = "+FULLRESYNC " + server_info.get_master_replid() + " 0\r\n";
    this->response_ready = true;
    this->wait_till_reponse_sent();
    // Send an empty file to the replica database
    std::ifstream file(RDB_FILE_PATH, std::ios::binary);
    std::string response = read_rdb_file_binary(file);
    this->response_buf = "$" + std::to_string(response.size()) + "\r\n" + response;
    this->response_ready = true;
    this->communication_over();
}


// Parses a REPLCONF Command
void RedisParser::parseREPLCONFCommand(const std::string& input, size_t& pos){
    this->response_buf = OK_RESPONSE;
    response_ready = true;
    this->communication_over();
    return;
}

// Parses an INFO Command
void RedisParser::parseINFOCommand(const std::string& input, size_t& pos){
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
    this->response_buf = create_string_reponse(response);
    response_ready = true;
    this->communication_over();
    return;
}

// Parses a KEYS Command
void RedisParser::parseKEYSCommand(const std::string& input, size_t& pos){
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
    this->response_buf = create_array_reponse(response_array);
    response_ready = true;
    this->communication_over();
    return;
}



// Parses a CONFIG GET Command
void RedisParser::parseCONFIGGETCommand(const std::string& input, size_t& pos, int num_elements){
    std::string command = parseBulkString(input, pos);
    for (auto& c : command) c = std::toupper(c);
    if (command == "GET"){
        std::string config_variable = parseBulkString(input, pos);
        if (config_variable == "dir"){
            this->response_buf = create_array_reponse({config_variable, config::dir});
            response_ready = true;
            this->communication_over();
            return;
        }
        else if (config_variable == "dbfile"){
            this->response_buf = create_array_reponse({config_variable, config::dbfilename});
            response_ready = true;
            this->communication_over();
            return;
        }
    }
    else{
        std::cerr << "Error: Config Followed by unknown Command" << command << std::endl;
    }
    this->response_buf = "";
    response_ready = true;
    this->communication_over();
    return;
}

// Parses a GET Command
void RedisParser::parseGETCommand(const std::string& input, size_t& pos){
    std::string argument = parseBulkString(input, pos);
    std::string response = db_handler.get(argument);
    if (response != NULL_RESPONSE){
        this->response_buf = create_string_reponse(response);
        response_ready = true;
        return;
    }
    this->response_buf = response;
    response_ready = true;
    this->communication_over();
    return;
}

//Parses a SET Command
void RedisParser::parseSETCommand(const std::string& input, size_t& pos, 
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
    this->response_buf = OK_RESPONSE;
    response_ready = true;
    this->communication_over();
    return;
}

// Parses an ECHO Command
void RedisParser::parseECHOCommand(const std::string& input, size_t& pos){
    // Parse argument bulk string, e.g., "$9\r\nraspberry\r\n"
    std::string argument = parseBulkString(input, pos);

    // Formulate the RESP response for the ECHO command (bulk string format)
    this->response_buf = create_string_reponse(argument);
    response_ready = true;
    this->communication_over();
    return;
}


// Parses a PING Command
void RedisParser::parsePINGCommand(const std::string& input, size_t& pos){
    this->response_buf = PING_RESPONSE;
    response_ready = true;
    this->communication_over();
    return;
}


//Helper Functions

// Function to clear the reponse buffer
void RedisParser::clear_response_buf(){
    this->response_buf.clear();
}

// Wait till the response is sent by the client Handler
void RedisParser::wait_till_reponse_sent(){
    while(!this->response_sent) {}
    this->response_sent = false;
}

void RedisParser::communication_over(){
    this->wait_till_reponse_sent();
    this->is_communicating = false;
}