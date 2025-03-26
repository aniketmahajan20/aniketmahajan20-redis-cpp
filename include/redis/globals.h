#ifndef GLOBALS_H
#define GLOBALS_H

#include <string>

#define NULL_RESPONSE "$-1\r\n"
#define DEFUALT_PORT 6379
#define RECV_BUFFER_SIZE 4096
#define RDB_FILE_PATH "./empty.rdb"

namespace config{
    extern std::string dir;
    extern std::string dbfilename;
    extern int port;
}


#endif //GLOBALS_H