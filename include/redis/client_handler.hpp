#ifndef CLIENTHANDLER_HPP
#define CLIENTHANDLER_HPP

#include <mutex>
#include <thread>
#include "./redis_parser.hpp"

/*
Summary: Client Handler Function
*/
class ClientHandler {
public: 
  ClientHandler(DatabaseHandler& db_handler) : db_handler(db_handler) {
    bool is_replica = false;
  }
  std::thread client_handler_thread(int client_fd);
private:
  DatabaseHandler& db_handler;
  bool is_replica;
  void client_handler(int client_fd);
  void response_handler(int client_fd, std::string recv_str);
};

#endif //CLIENTHANDLER_HPP