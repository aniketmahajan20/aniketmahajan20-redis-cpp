#include <mutex>
#include <thread>
#include "./redis_parser.hpp"

#define BUFFER_SIZE 4096
/*
Summary: Client Handler Function
*/
class ClientHandler {
private:
  std::mutex database_mutex;
public: 
  void client_handler(int client_fd);
  std::thread client_handler_thread(int client_fd);
};