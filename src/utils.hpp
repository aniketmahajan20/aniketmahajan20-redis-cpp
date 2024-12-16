#ifndef UTILS_HPP
#define UTILS_HPP

#include <fstream>
#include <iostream>
#include <cstdint>
#include <chrono>
#include <string>

#include "./server_info.hpp"

uint64_t reverseEndianness(uint64_t value);
void print_file_hex(std::ifstream& file);
long long get_current_time_milli();
std::string get_recv_str(int file_descriptor);
void parseCommandLineArgs(int argc, char* argv[], ServerInfo& server_info);
// Create the response for the client
std::string create_string_reponse(const std::string& response);
// Create array response for the client
std::string create_array_reponse(const std::vector<std::string>& response_arr);
std::string read_rdb_file_binary(std::ifstream& file);
// Parses a bulk string in RESP format, e.g., "$9\r\nraspberry\r\n"
std::string parseBulkString(const std::string& input, size_t& pos);
std::string uint64ToString(uint64_t value);

#endif //UTILS_HPP