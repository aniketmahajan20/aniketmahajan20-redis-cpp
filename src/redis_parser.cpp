#include <stdexcept>
#include <iostream>
#include <unistd.h>
#include <regex>
#include <fstream>
#include <thread>

#include "../include/redis/redis_parser.hpp"
#include "../include/redis/globals.h"
#include "../include/redis/utils.hpp"
#include <sys/socket.h>

// void RedisParser::send_response(){
//     // send(this->client_fd, &this->response_buf[0], this->response_buf.size(), 0);
//     return;
// }

std::thread RedisParser::parseRESPCommand_thread(int client_fd, const std::string& input){
    return std::thread([this, client_fd, input] {RedisParser::parseRESPCommand(client_fd, input, 0);});
}

void RedisParser::parseRESPCommand(int client_fd, const std::string& input, size_t pos = 0) {
    // Wait till in communication loop
    // TODO: Make this more efficient so there is less busy waiting
    if (!this->handler_in_communication_loop){
        std::unique_lock<std::mutex> lock(this->handler_in_communication_loop_mtx);
        this->handler_in_communication_loop_cv.wait(lock);
    }

    // std::cout << "Received Bytes: " << input << std::endl;
    // Parse array indicator, e.g., "*2\r\n"
    if (input[pos] != '*') {
        // this->enqueueMessage("-ERR protocol error: expected array");
        this->communication_over();
        // response_ready = true;
        // std::this_thread::sleep_for(std::chrono::milliseconds(50));
        return;
    }
    pos++;
    // std::cout << "In Parse RESP Command" << std::endl;
    // Read number of elements
    size_t elements_end = input.find("\r\n", pos);
    int num_elements = std::stoi(input.substr(pos, elements_end - pos));
    pos = elements_end + 2;

    // Parse command bulk string, e.g., "$4\r\nECHO\r\n"
    std::string command = parseBulkString(input, pos);

    // Convert command to uppercase to make it case-insensitive
    std::transform(command.begin(), command.end(), command.begin(), ::toupper);

    if (command == "ECHO"){
        return parseECHOCommand(input, pos);
    }
    else if (command == "PING"){
        return parsePINGCommand();
    }
    else if (command == "SET"){
        return parseSETCommand(input, pos, num_elements);
    }
    else if (command == "GET"){
        return parseGETCommand(input, pos);
    }
    else if (command == "CONFIG"){
        return parseCONFIGGETCommand(input, pos);
    }
    else if (command == "KEYS"){
        return parseKEYSCommand(input, pos);
    }
    else if (command == "INFO"){
        return parseINFOCommand(input, pos);
    }
    else if (command == "REPLCONF"){
        return parseREPLCONFCommand();
    }
    else if (command == "PSYNC"){
        return parsePSYNCCommand(client_fd);
    }
    else { 
        this->enqueueMessage(NULL_RESPONSE);
        std::cout << "-ERR unknown command '" + command + "'" << std::endl;
        this->communication_over();
        return;
    }    
}

// Parses a PSYNC Command
void RedisParser::parsePSYNCCommand(int client_fd){
    ServerInfo& server_info = db_handler.get_server_info();
    // If the server is a master, then add the slave to connected slaves count
    if (server_info.get_role() == "master"){
        server_info.update_connected_slaves(server_info.get_connected_slaves() + 1);
        server_info.add_connected_slave(client_fd);
    }
    // Hardcoding the response as suggested by the requirements
    this->enqueueMessage("+FULLRESYNC " + server_info.get_master_replid() + " 0\r\n");
    // this->send_response();
    // Send an empty file to the replica database
    std::ifstream file(RDB_FILE_PATH, std::ios::binary);
    std::string response = read_rdb_file_binary(file);
    this->enqueueMessage("$" + std::to_string(response.size()) + "\r\n" + response);
    this->communication_over();
    // this->response_ready = true;
}


// Parses a REPLCONF Command
void RedisParser::parseREPLCONFCommand(){
    this->enqueueMessage(OK_RESPONSE);
    this->communication_over();
    return;
}

// Parses an INFO Command
void RedisParser::parseINFOCommand(const std::string& input, size_t& pos){
    std::string command = parseBulkString(input, pos);
    for (auto& c : command) c = std::toupper(c);
    std::string response = "";
    if (command == "REPLICATION"){
        ServerInfo& server_info = db_handler.get_server_info();
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
    this->enqueueMessage(create_string_reponse(response));
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
        pattern = std::regex(to_match);
    }
    std::vector<std::string> response_array;
    for (auto pair: db_handler.database){
        std::string key = pair.first;
        if (std::regex_match(key, pattern)){
            response_array.push_back(key);
        }
    }
    this->enqueueMessage(create_array_reponse(response_array));
    this->communication_over();
    return;
}



// Parses a CONFIG GET Command
void RedisParser::parseCONFIGGETCommand(const std::string& input, size_t& pos){
    std::string command = parseBulkString(input, pos);
    for (auto& c : command) c = std::toupper(c);
    if (command == "GET"){
        std::string config_variable = parseBulkString(input, pos);
        if (config_variable == "dir"){
            this->enqueueMessage(create_array_reponse({config_variable, config::dir}));
            this->communication_over();
            return;
        }
        else if (config_variable == "dbfile"){
            this->enqueueMessage(create_array_reponse({config_variable, config::dbfilename}));
            this->communication_over();
            return;
        }
    }
    else{
        std::cerr << "Error: Config Followed by unknown Command" << command << std::endl;
    }
    this->enqueueMessage(NULL_RESPONSE);
    this->communication_over();
    return;
}

// Parses a GET Command
void RedisParser::parseGETCommand(const std::string& input, size_t& pos){
    // std::cout<< "IN GET COMMAND" << std::endl;
    std::string argument = parseBulkString(input, pos);
    std::string response = db_handler.get(argument);
    if (response != NULL_RESPONSE){
        this->enqueueMessage(create_string_reponse(response));
        this->communication_over();
        return;
    }
    this->enqueueMessage(response);
    this->communication_over();
    return;
}

//Parses a SET Command
void RedisParser::parseSETCommand(const std::string& input, size_t& pos, 
                                        int num_elements){
    // std::cout << "Parsing Command:" << input << std::endl;
    std::string argument1 = parseBulkString(input, pos);
    std::string argument2 = parseBulkString(input, pos);
    long expiry_time = 0;
    // TODO: Implement error handling for cases that px was incorrect 
    // or the time value is not provided
    // Currently Assumed that PX and expiry time are valid commands provided
    // std::cout << "In SET Command" << std::endl;
    if (num_elements > 3){
        std::string expiry_command = parseBulkString(input, pos);
        std::string expiry_value = parseBulkString(input, pos);
        auto current_time = get_current_time_milli();
        expiry_time = current_time + std::stoi(expiry_value);
    }
    std::cout << "Setting Key: " << argument1 << " Value: " << argument2 << " Expiry Time: " << expiry_time << std::endl;
    db_handler.set(argument1, argument2, expiry_time);
    // Wait to finish communication, otherwise parser is deleted before the response is sent
    this->enqueueMessage(OK_RESPONSE);
    if (input[pos] == '*'){
        this->parseRESPCommand(this->client_fd, input, pos);
    }
    // Propagate the SET command to all connected slaves
    ServerInfo& server_info = db_handler.get_server_info();
    if (server_info.get_role() == "master"){
        for (int i = 0; i < server_info.get_connected_slaves(); i++){
            int fd = server_info.get_connected_slave_fd(i);
            send(fd, &input[0], input.size(), 0);
        }
    }
    this->communication_over();
    return;
}

// Parses an ECHO Command
void RedisParser::parseECHOCommand(const std::string& input, size_t& pos){
    // Parse argument bulk string, e.g., "$9\r\nraspberry\r\n"
    std::string argument = parseBulkString(input, pos);

    // Formulate the RESP response for the ECHO command (bulk string format)
    this->enqueueMessage(create_string_reponse(argument));
    this->communication_over();
    return;
}


// Parses a PING Command
void RedisParser::parsePINGCommand(){
    this->enqueueMessage(PING_RESPONSE);
    this->communication_over();
}


//Helper Functions

//Function to enqueue a message to the response buffer
void RedisParser::enqueueMessage(const std::string message){
    {
        std::lock_guard<std::mutex> lock(response_buf_mtx);
        response_buf.push(message);
    }
    this->response_buf_cv.notify_one();
    this->wait_till_reponse_sent();
}

// Function to clear the reponse buffer
// void RedisParser::clear_response_buf(){
//     this->response_buf.clear();
// }

// Wait till the response is sent by the client Handler
void RedisParser::wait_till_reponse_sent(){
    if(!this->response_sent){
        std::unique_lock<std::mutex> lock(this->response_sent_mtx);
        this->response_sent_cv.wait(lock);
        this->response_sent = false;
    }
}

void RedisParser::communication_over(){
    // this->wait_till_reponse_sent();
    this->is_communicating = false;
}