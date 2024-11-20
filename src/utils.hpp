#ifndef UTILS_HPP
#define UTILS_HPP

#include <fstream>
#include <iostream>
#include <cstdint>
#include <chrono>

uint64_t reverseEndianness(uint64_t value);
void print_file_hex(std::ifstream& file);
long long get_current_time_milli();

#endif //UTILS_HPP