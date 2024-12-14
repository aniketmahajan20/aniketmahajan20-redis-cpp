#ifndef GLOBALS_H
#define GLOBAL_H

#include <string>

#include "./server_info.hpp"

#define NULL_RESPONSE "$-1\r\n"
#define DEFUALT_PORT 6379
#define RECV_BUFFER_SIZE 4096

namespace config{
    extern std::string dir;
    extern std::string dbfilename;
    extern int port;

    void parseCommandLineArgs(int argc, char* argv[], ServerInfo& server_info);
}


#endif //GLOBALS_H