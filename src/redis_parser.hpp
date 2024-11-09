// RedisParser.hpp
#ifndef REDISPARSER_HPP
#define REDISPARSER_HPP

#include <string>
#include <unordered_map>

class RedisParser {
public:
    // Parses a Redis command and returns the response
    std::string parseRESPCommand(const std::string& input);

private:
    std::unordered_map<std::string, std::string> database;
    // Parses a GET Command
    std::string parseGETCommand(const std::string& input, size_t& pos);
    // Parses a SET command
    std::string parseSETCommand(const std::string& input, size_t& pos);
    // Parses an echo command
    std::string parseECHOCommand(const std::string& input, size_t& pos);
    // Parses an PING command
    std::string parsePINGCommand(const std::string& input, size_t& pos);
    // Parses a bulk string in RESP format, e.g., "$9\r\nraspberry\r\n"
    std::string parseBulkString(const std::string& input, size_t& pos);
};

#endif // REDISPARSER_HPP
