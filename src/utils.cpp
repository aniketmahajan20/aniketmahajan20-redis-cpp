#include <arpa/inet.h>

#include "./utils.hpp"
#include "./globals.h"

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

// Get the received String 
std::string get_recv_str(int file_descriptor){
    char rec_buf[RECV_BUFFER_SIZE];
    int bytesReceived = recv(file_descriptor, rec_buf, RECV_BUFFER_SIZE, 0);
    std::string recv_str(rec_buf, 0, bytesReceived);
    return recv_str;
}

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
            server_info.update_info("slave");
            server_info.get_master_ip_port(argv[++i]);
            server_info.send_handshake();
        }
    }
}
