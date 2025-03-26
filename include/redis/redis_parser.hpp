// RedisParser.hpp
#ifndef REDISPARSER_HPP
#define REDISPARSER_HPP

#include <string>
#include <unordered_map>
#include <mutex>
#include <vector>
#include <chrono>

#include "./database_handler.hpp"
#include "./client_handler.hpp"

#define PING_RESPONSE "+PONG\r\n"
#define OK_RESPONSE   "+OK\r\n"

class RedisParser {
public:
    RedisParser(DatabaseHandler& db_handler) : db_handler(db_handler) {
        this->is_communicating = false;
        this->response_ready = false;
        this->response_sent = false;
    }
    // Returns a thread of parseRESPCommand function
    std::thread parseRESPCommand_thread(int client_fd, const std::string& input);

private:
    DatabaseHandler& db_handler;
    std::atomic<bool> is_communicating;
    std::atomic<bool> response_ready;
    std::atomic<bool> response_sent;
    std::string response_buf;
    int client_fd;
    // TODO: Add mutexes for all the flags
    // Parses a PSYNC Command
    void parsePSYNCCommand(int client_fd);
    // Parses a REPLCONF Command
    void parseREPLCONFCommand();
    // Parses an INFO Command
    void parseINFOCommand(const std::string& input, size_t& pos);
    // Parses a KEYS Command
    void parseKEYSCommand(const std::string& input, size_t& pos);
    // Parses a CONFIG GET Command
    void parseCONFIGGETCommand(const std::string& input, size_t& pos);
    // Parses a GET Command
    void parseGETCommand(const std::string& input, size_t& pos);
    // Parses a SET command
    void parseSETCommand(const std::string& input, size_t& pos, 
                                int num_elements);
    // Parses an echo command
    void parseECHOCommand(const std::string& input, size_t& pos);
    // Parses an PING command
    void parsePINGCommand();
    // Parses a Redis command and returns the response
    void parseRESPCommand(int client_fd, const std::string& input);
    // declaring as friend function to access private members of this class
    friend class ClientHandler;

    //Helper functions
    void wait_till_reponse_sent();
    void clear_response_buf();
    void communication_over();
    void send_response();
};

#endif // REDISPARSER_HPP
