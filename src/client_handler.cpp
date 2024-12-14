#include <iostream>
#include <cstring>
#include <string>
#include <sys/socket.h>

#include "./client_handler.hpp"
#include "./globals.h"
/*
Summary: Client Handler Function
*/
void ClientHandler::client_handler(int client_fd){
    RedisParser parser(db_handler);
    std::string buf;
    char rec_buf[RECV_BUFFER_SIZE];
    while (true){
        memset(rec_buf, 0, RECV_BUFFER_SIZE);
        // wait for the client to send data
        int bytesReceived = recv(client_fd, rec_buf, RECV_BUFFER_SIZE, 0);
        if (bytesReceived == -1){
            std::cerr << "Error in recv(). Quitting..." << std::endl;
            break;
        }
        if (bytesReceived == 0){
            std::cout << "Client disconnected..." << std::endl;
            break;
        }
        std::string recv_str(rec_buf, 0, bytesReceived);
        // std::cout << "Received from client: " << recv_str;
        buf = parser.parseRESPCommand(recv_str);
        // Send the pong message to client
        send(client_fd, &buf[0], buf.size(), 0);
    }
}

std::thread ClientHandler::client_handler_thread(int client_fd){
    return std::thread([this, client_fd] {ClientHandler::client_handler(client_fd);});
}