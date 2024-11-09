#include <sstream>
#include <stdexcept>
#include <iostream>
#include "redis_parser.hpp"

std::string RedisParser::parseRESPCommand(const std::string& input) {
    size_t pos = 0;

    // Parse array indicator, e.g., "*2\r\n"
    if (input[pos] != '*') {
        return "-ERR protocol error: expected array";
    }
    pos++;

    // Read number of elements
    size_t elements_end = input.find("\r\n", pos);
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
    else {
        return "-ERR unknown command '" + command + "'";
    }

    
}


// Parses an ECHO Command
std::string RedisParser::parseECHOCommand(const std::string& input, size_t& pos){
    // Parse argument bulk string, e.g., "$9\r\nraspberry\r\n"
    std::string argument = parseBulkString(input, pos);

    // Formulate the RESP response for the ECHO command (bulk string format)
    return "$" + std::to_string(argument.size()) + "\r\n" + argument + "\r\n";
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
