#ifndef GLOBALS_H
#define GLOBAL_H

#include <string>

#define NULL_RESPONSE "$-1\r\n"
#define DEFUALT_PORT 6379
#define RECV_BUFFER_SIZE 4096

namespace config{
    extern std::string dir;
    extern std::string dbfilename;
    extern int port;
}


#endif //GLOBALS_H