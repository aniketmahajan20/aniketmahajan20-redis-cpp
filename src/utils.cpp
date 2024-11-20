#include "./utils.hpp"

uint64_t reverseEndianness(uint64_t value) {
    uint64_t reversed = 0;
    for (int i = 0; i < 8; ++i) {
        reversed <<= 8;                     // Make room for the next byte
        reversed |= (value & 0xFF);         // Extract the least significant byte
        value >>= 8;                        // Shift to the next byte
    }
    return reversed;
}

void print_file_hex(std::ifstream& file){
    while(file){
        uint64_t byte;
        file.read(reinterpret_cast<char*>(&byte), sizeof(byte));
        uint64_t reverseByte = reverseEndianness(byte);
        std::cout << std::hex << reverseByte << std::endl;
    }
}

// Get the current time in milliseconds
long long get_current_time_milli(){
    auto now = std::chrono::system_clock::now();
    auto duration = now.time_since_epoch();
    auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
    return milliseconds;
}
