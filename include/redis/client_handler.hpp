#ifndef CLIENTHANDLER_HPP
#define CLIENTHANDLER_HPP

#include <mutex>
#include <thread>
#include <vector>
#include <queue>
#include <condition_variable>
#include <functional>

#include "./redis_parser.hpp"
#include "./non_blocking_queue.hpp"

constexpr int THREAD_POOL_SIZE = 4;

/*
Summary: Client Handler Function
*/
class ClientHandler {
public: 
  ClientHandler(DatabaseHandler& db_handler) : db_handler(db_handler),
                                               is_replica(false) {
    for (int i = 1; i <= THREAD_POOL_SIZE; i++) {
      workers.emplace_back(&ClientHandler::worker_thread, this);
      std::cout << "running " << i << " workers" << std::endl;
    }             
  }

  ~ClientHandler() {
    std::cout<< "Client Handler Destructor Called" << std::endl;
    for (std::thread& worker : workers) {
      worker.join();
    }
  }

  std::thread client_handler_thread(int client_fd);
private:
  DatabaseHandler& db_handler;
  bool is_replica;
  std::vector<std::thread> workers;
  std::queue<std::function<void()>> task_queue;
  std::condition_variable cv;
  std::mutex mtx;
  void worker_thread();
  void client_handler(int client_fd);
  void response_handler(int client_fd, std::string recv_str);
  void enqueueTask(const std::function<void()>& task);
};

#endif //CLIENTHANDLER_HPP