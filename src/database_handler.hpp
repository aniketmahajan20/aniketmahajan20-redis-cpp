#ifndef DATABASE_HPP
#define DATABASE_HPP

#include <string>
#include <unordered_map>
#include <mutex>

#include "./server_info.hpp"

struct DBValue {
    std::string value;
    long expiry_time;
    DBValue() : value(""), expiry_time(0) {}
    DBValue(std::string val) : value(val), expiry_time(0) {}
    DBValue(std::string val, long time) : value(val), expiry_time(time) {}
};

class DatabaseHandler {
private:
    ServerInfo& server_info;
    std::mutex database_mutex;
public:
    DatabaseHandler(ServerInfo& server_info): server_info(server_info) {}
    std::unordered_map<std::string, DBValue> database;
    
    // Set Key-Value pair in the Database and Expiry time if given
    void set(std::string key, std::string value,long expiry_time);

    // Set Key-Value pair in the Database and Expiry time if give
    // Note: NO lock is used when serially reading the RDB file and 
    // populating the database
    void set_without_lock(std::string key, std::string value,long expiry_time);

    //Function to access the ServerInfo
    ServerInfo get_server_info();
    
    // Function to get the value based on the key
    std::string get(std::string key);
};
#endif //DATABASE_HPP   