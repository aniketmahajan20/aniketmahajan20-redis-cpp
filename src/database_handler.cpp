#include "./database_handler.hpp"
#include "./utils.hpp"
#include "./globals.h"


void DatabaseHandler::set(std::string key, std::string value, 
                            long expiry_time){
    database_mutex.lock();      
    database[key] = DBValue(value, expiry_time);                     
    database_mutex.unlock();
}

void DatabaseHandler::set_without_lock(std::string key, std::string value, 
                            long expiry_time){     
    database[key] = DBValue(value, expiry_time);                     
}

std::string DatabaseHandler::get(std::string key){
    if (database.find(key) != database.end()){
        DBValue response_struct = database[key];
        if (response_struct.expiry_time == 0 || get_current_time_milli() < response_struct.expiry_time){
            std::string response = response_struct.value;
            return response;
        }
        else{
            database.erase(key);
            return NULL_RESPONSE;
        }
    }
    else {
        return NULL_RESPONSE;
    }
}