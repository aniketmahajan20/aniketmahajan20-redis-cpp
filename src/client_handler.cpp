#include <iostream>
#include <cstring>
#include <string>
#include <sys/socket.h>

#include "../include/redis/client_handler.hpp"
#include "../include/redis/globals.h"
/*
Summary: Client Handler Function
*/
void ClientHandler::client_handler(int client_fd){
    char rec_buf[RECV_BUFFER_SIZE];
    std::string recv_str;
    while (true){
        memset(rec_buf, 0, RECV_BUFFER_SIZE);
        // wait for the client to send data
        int bytesReceived = recv(client_fd, rec_buf, RECV_BUFFER_SIZE, 0);
        // if (bytesReceived == -1){
        //     std::cerr << "Error in recv(). Quitting..." << std::endl;
        //     break;
        // }
        if (bytesReceived == 0){
            std::cout << "Client disconnected..." << std::endl;
            break;
        }
        recv_str = std::string(rec_buf, 0, bytesReceived);

        // std::cout << "Received Bytes: " << recv_str << std::endl;
        this->enqueueTask([this, client_fd, recv_str]() {
            response_handler(client_fd, recv_str);
        });
        std::cout << "Task Enqueued" << std::endl;
        // std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

// Declared as a friend function in RedisParser. 
// Therefore has access to private members of that class
void ClientHandler::response_handler(int client_fd, std::string recv_str){
    RedisParser* parser = new RedisParser(db_handler);
    parser->is_communicating = true;                                // Set the is_communicating to true
    std::string response;
    parser->client_fd = client_fd;
    // Start the Parser Thread
    std::thread t = parser->parseRESPCommand_thread(client_fd, recv_str);
    // TODO: Add condition variables to stop the CPU load of looping and checking flags 
    // use condition variables to wait for the response to be ready
    while(parser->is_communicating){
        if (parser->response_ready){
            parser->response_ready = false;
            response = parser->response_buf;
            parser->clear_response_buf();
            parser->response_sent = true;
        }
        if (!response.empty()){
            send(client_fd, &response[0], response.size(), 0);
            response.clear();
        }
    }
    std::cout << "Communication Over" << std::endl;
    t.join();
    delete parser;
}

std::thread ClientHandler::client_handler_thread(int client_fd){
    return std::thread([this, client_fd] {ClientHandler::client_handler(client_fd);});
}


void ClientHandler::worker_thread() {
    int timer = 0;
    while (true) {
        timer++;
        std::function<void()> task;
        std::unique_lock<std::mutex> lock(mtx);
        // Wait until there is a task
        cv.wait(lock, [this] { return !task_queue.empty(); });
        if (task_queue.dequeue(&task)) {
            std::cout << "A Task found. NOw executing it" << std::endl;
            task(); // Execute the taskß
        } else {
            if (timer%1000 == 0){
                std::cout << "I'm in worker thread and checked a 1000 times aready, but now task is found" << std::endl;
            }
            continue; // No tasks, yield to avoid busy-waiting
        }
    }
}

void ClientHandler::enqueueTask(const std::function<void()>& task) {
    {
        std::lock_guard<std::mutex> lock(mtx);
        task_queue.enqueue(task);
    }
    this->cv.notify_one();
}
