#ifndef UTILS_HPP
#define UTILS_HPP

#include <fstream>
#include <iostream>
#include <cstdint>
#include <chrono>
#include "./server_info.hpp"

uint64_t reverseEndianness(uint64_t value);
void print_file_hex(std::ifstream& file);
long long get_current_time_milli();
void parseCommandLineArgs(int argc, char* argv[], ServerInfo& server_info);

#endif //UTILS_HPP