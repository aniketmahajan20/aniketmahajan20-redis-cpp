#include <arpa/inet.h>

#include "../include/redis/utils.hpp"
#include "../include/redis/globals.h"

uint64_t reverseEndianness(uint64_t value) {
    uint64_t reversed = 0;
    for (int i = 0; i < 8; ++i) {
        reversed <<= 8;                     // Make room for the next byte
        reversed |= (value & 0xFF);         // Extract the least significant byte
        value >>= 8;                        // Shift to the next byte
    }
    return reversed;
}

// Prints the RDB file in hex
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

// Get the received String 
std::string get_recv_str(int file_descriptor){
    char rec_buf[RECV_BUFFER_SIZE];
    int bytesReceived = recv(file_descriptor, rec_buf, RECV_BUFFER_SIZE, 0);
    std::string recv_str(rec_buf, 0, bytesReceived);
    return recv_str;
}

// Utility function to parse the command line arguments
void parseCommandLineArgs(int argc, char* argv[], ServerInfo& server_info){
    for (int i = 0; i < argc; i++){
        std::string arg = argv[i];
        if (arg == "--dir" && i+1 < argc){
            config::dir = argv[++i];
        }
        if (arg == "--dbfilename" && i+1 < argc){
            config::dbfilename = argv[++i];
        }
        if (arg == "--port" && i+1 < argc){
            config::port = std::stoi(argv[++i]);
        }
        if (arg == "--replicaof" && i+1 < argc){
            server_info.update_role("slave");
            server_info.get_master_ip_port(argv[++i]);
            server_info.connect_to_master();
        }
    }
}

// Create the response for the client
std::string create_string_reponse(const std::string& response){
    return "$" + std::to_string(response.size()) + "\r\n" + response + "\r\n"; 
}

// Create array response for the client
std::string create_array_reponse(const std::vector<std::string>& response_arr){
    std::string response = "";
    for (size_t i = 0; i < response_arr.size(); i++){
        response += create_string_reponse(response_arr[i]);
    }
    return "*" + std::to_string(response_arr.size()) + "\r\n" + response;
}

std::string read_rdb_file_binary(std::ifstream& file){
    std::string binary_contents;
    while(file){
        uint64_t byte;
        file.read(reinterpret_cast<char*>(&byte), sizeof(byte));
        uint64_t reverseByte = reverseEndianness(byte);
        binary_contents += uint64ToString(reverseByte);
        // binary_contents += std::bitset<64>(reverseByte).to_string();
    }
    return binary_contents;
}

// Parses a bulk string in RESP format, e.g., "$9\r\nraspberry\r\n"
std::string parseBulkString(const std::string& input, size_t& pos) {
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

// converts uint64_t value to a string with characters for each byte
std::string uint64ToString(uint64_t value) {
    std::string result;

    // Extract each byte from the uint64_t (big-endian order)
    for (int i = sizeof(uint64_t) - 1; i >= 0; --i) {
        char byte = static_cast<char>((value >> (i * 8)) & 0xFF);
        result += byte; // Append byte as a character to the string
    }
    return result;
}