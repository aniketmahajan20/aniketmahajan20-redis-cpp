#include <mutex>
#include <thread>
#include "./redis_parser.hpp"

#define BUFFER_SIZE 4096
/*
Summary: Client Handler Function
*/
class ClientHandler {
public: 
  ClientHandler(DatabaseHandler& db_handler) : db_handler(db_handler) {}
  void client_handler(int client_fd);
  std::thread client_handler_thread(int client_fd);
private:
  DatabaseHandler& db_handler;
};