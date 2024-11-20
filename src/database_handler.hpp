#ifndef DATABASE_HPP
#define DATABASE_HPP

#include <string>
#include <unordered_map>
#include <mutex>

struct DBValue {
    std::string value;
    long expiry_time;
    DBValue() : value(""), expiry_time(0) {}
    DBValue(std::string val) : value(val), expiry_time(0) {}
    DBValue(std::string val, long time) : value(val), expiry_time(time) {}
};

class DatabaseHandler {
private:
    
    std::mutex database_mutex;
public:
    std::unordered_map<std::string, DBValue> database;
    
    // Set Key-Value pair in the Database and Expiry time if given
    void set(std::string key, std::string value,long expiry_time);

    // Set Key-Value pair in the Database and Expiry time if give
    // Note: NO lock is used when serially reading the RDB file and 
    // populating the database
    void set_without_lock(std::string key, std::string value,long expiry_time);

    // Function to get the value based on the key
    std::string get(std::string key);
};
#endif //DATABASE_HPP   