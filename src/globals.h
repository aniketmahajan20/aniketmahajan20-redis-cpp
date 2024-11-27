#ifndef GLOBALS_H
#define GLOBAL_H

#include <string>

#define NULL_RESPONSE "$-1\r\n"
#define DEFUALT_PORT 6379

namespace config{
    extern std::string dir;
    extern std::string dbfilename;
    extern int port;

    void parseCommandLineArgs(int argc, char* argv[]);
}


#endif //GLOBALS_H