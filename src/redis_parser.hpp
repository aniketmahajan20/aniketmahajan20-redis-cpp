// RedisParser.hpp
#ifndef REDISPARSER_HPP
#define REDISPARSER_HPP

#include <string>
#include <unordered_map>
#include <mutex>
#include <vector>
#include <chrono>

#include "./database_handler.hpp"

#define PING_RESPONSE "+PONG\r\n"
#define OK_RESPONSE   "+OK\r\n"

class RedisParser {
public:
    RedisParser(DatabaseHandler& db_handler) : db_handler(db_handler) {}
    // Parses a Redis command and returns the response
    std::string parseRESPCommand(const std::string& input);

private:
    DatabaseHandler& db_handler;
    // Parses a REPLCONF Command
    std::string parseREPLCONFCommand(const std::string& input, size_t& pos);
    // Parses an INFO Command
    std::string parseINFOCommand(const std::string& input, size_t& pos);
    // Parses a KEYS Command
    std::string parseKEYSCommand(const std::string& input, size_t& pos);
    // Parses a CONFIG GET Command
    std::string parseCONFIGGETCommand(const std::string& input, size_t& pos, int num_elements);
    // Parses a GET Command
    std::string parseGETCommand(const std::string& input, size_t& pos);
    // Parses a SET command
    std::string parseSETCommand(const std::string& input, size_t& pos, 
                                int num_elements);
    // Parses an echo command
    std::string parseECHOCommand(const std::string& input, size_t& pos);
    // Parses an PING command
    std::string parsePINGCommand(const std::string& input, size_t& pos);
    // Parses a bulk string in RESP format, e.g., "$9\r\nraspberry\r\n"
    std::string parseBulkString(const std::string& input, size_t& pos);

    // Helper Functions
    // Create the response for the client
    std::string create_string_reponse(const std::string& response);
    // Create array response for the client
    std::string create_array_reponse(const std::vector<std::string>& response_arr);
};

#endif // REDISPARSER_HPP
