// RedisParser.hpp
#ifndef REDISPARSER_HPP
#define REDISPARSER_HPP

#include <string>
#include <unordered_map>
#include <mutex>
#include <vector>


struct DBValue {
    std::string value;
    long expiry_time;
    DBValue() : value(""), expiry_time(0) {}
    DBValue(std::string val) : value(val), expiry_time(0) {}
    DBValue(std::string val, long time) : value(val), expiry_time(time) {}
};

class RedisParser {
public:
    // Parses a Redis command and returns the response
    std::string parseRESPCommand(const std::string& input, std::mutex& database_mutex);

private:
    std::unordered_map<std::string, DBValue> database;
    // Parses a CONFIG GET Command
    std::string parseCONFIGGETCommand(const std::string& input, size_t& pos, int num_elements);
    // Parses a GET Command
    std::string parseGETCommand(const std::string& input, size_t& pos);
    // Parses a SET command
    std::string parseSETCommand(const std::string& input, size_t& pos, 
                                int num_elements, std::mutex& database_mutex);
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
    // Get the current time in milliseconds
    inline long long get_current_time_milli();
};

#endif // REDISPARSER_HPP
