#include <random>
#include <sys/socket.h>
#include <iostream>
#include <arpa/inet.h>
#include <regex>
#include <cstring>
#include <thread>

#include "../include/redis/server_info.hpp"
#include "../include/redis/globals.h"
#include "../include/redis/utils.hpp"

#define REPLID_LEN 40
#define LOCALHOST "127.0.0.1"

std::string ServerInfo::get_role(){
    return role;
}
int ServerInfo::get_connected_slaves(){
    return connected_slaves;
}
std::string ServerInfo::get_master_replid(){
    return master_replid;
}
int ServerInfo::get_master_repl_offset(){
    return master_repl_offset;
}
int ServerInfo::get_second_repl_offset()
{
    return second_repl_offset;
}
int ServerInfo::get_repl_backlog_active(){
    return repl_backlog_active;
}
int ServerInfo::get_repl_backlog_size(){
    return repl_backlog_size;
}
int ServerInfo::get_repl_backlog_first_byte_offset(){
    return repl_backlog_first_byte_offset;
}
int ServerInfo::get_repl_backlog_histlen(){
    return repl_backlog_histlen;
}
void ServerInfo::update_role(std::string role){
    this->role.assign(role);
}
void ServerInfo::update_connected_slaves(int connected_slaves){
    this->connected_slaves = connected_slaves;
}

//Server Handler Functions

void ServerInfo::get_master_ip_port(const std::string& args){
    const std::regex pattern("(\b(?:\\d{1,3}\\.){3}\\d{1,3}\b|localhost)\\s+(\\d{1,5})");
    std::smatch matches;
    std::regex_search(args, matches, pattern);
    if (!matches.empty()) {
        if (matches[1].compare("localhost") == 0){
            this->master_ip = LOCALHOST;
        }
        else {
            this->master_ip = matches.str(1);
        }
        this->master_port = std::stoi(matches.str(2));
    }
}

void ServerInfo::connect_to_master(){
    if (this->role == "slave"){
        this->master_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (this->master_fd < 0){
            std::cerr << "Error: Failed to create master server socket.\n";
        }
        // Since the tester restarts your program quite often, setting SO_REUSEADDR
        // ensures that we don't run into 'Address already in use' errors
        int reuse = 1;
        if (setsockopt(this->master_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
            std::cerr << "setsockopt failed\n";
        }
        std::cout << "Connecting to master at " << this->master_ip << ":" << this->master_port << std::endl;
        struct sockaddr_in master_addr;
        master_addr.sin_family = AF_INET;
        master_addr.sin_addr.s_addr = INADDR_ANY;
        master_addr.sin_port = htons(this->master_port);
        inet_pton(AF_INET, this->master_ip.c_str(), &master_addr.sin_addr);
        if (connect(this->master_fd, (struct sockaddr *) &master_addr, sizeof(master_addr)) != 0) 
            std::cerr << "Failed to connect to master \n";
    }
}
void ServerInfo::send_handshake(){
    if (this->role == "slave"){
        // Sending a PING to Master
        std::string handshake_message = "*1\r\n$4\r\nPING\r\n";
        send(this->master_fd, handshake_message.c_str(), handshake_message.size(), 0);
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        // Verifying a PONG is received
        // if(get_recv_str(this->master_fd) != "+PONG\r\n"){
        //     std::cerr << "Error: Incorrect/No reply for PING from master to replica.";
        // }
        // Sending REPLCONF command 1 to Master
        handshake_message = "*3\r\n$8\r\nREPLCONF\r\n$14\r\nlistening-port\r\n$4\r\n" + 
                                        std::to_string(config::port) + "\r\n";
        send(this->master_fd, handshake_message.c_str(), handshake_message.size(), 0);
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        // Veryfying +OK\r\n is received
        // if(get_recv_str(this->master_fd) != "+OK\r\n"){
        //     std::cerr << "Error: Incorrect/No reply for REPLCONF from master to replica.";
        // }
        // Sending REPLCONF command 2 to Master
        handshake_message = "*3\r\n$8\r\nREPLCONF\r\n$4\r\ncapa\r\n$6\r\npsync2\r\n";
        send(this->master_fd, handshake_message.c_str(), handshake_message.size(), 0);
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        // Veryfying +OK\r\n is received
        // if(get_recv_str(this->master_fd) != "+OK\r\n"){
        //     std::cerr << "Error: Incorrect/No reply for REPLCONF from master to replica.";
        // }
        // Sending PSYNC command to Master
        // Telling Master that replica has no data at all and request for Full Resync
        handshake_message = "*3\r\n$5\r\nPSYNC\r\n$1\r\n?\r\n$2\r\n-1\r\n";
        send(this->master_fd, handshake_message.c_str(), handshake_message.size(), 0);
    }
}

void ServerInfo::add_connected_slave(int fd){
    connected_slaves_fd.push_back(fd);
}

int ServerInfo::get_connected_slave_fd(int index){
    return connected_slaves_fd[index];
}

const int ServerInfo::get_master_fd(){
    return master_fd;
}

//Helper Functions
std::string ServerInfo::generate_master_replid(){
    const std::string CHARACTERS = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
    std::random_device rd;
    std::mt19937 generator(rd());
    std::uniform_int_distribution<> distribution(0, CHARACTERS.size() - 1);
    
    std::string random_string;
    for (int i = 0; i < REPLID_LEN; i++){
        random_string += CHARACTERS[distribution(generator)];
    }
    return random_string;
}