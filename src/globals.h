#ifndef GLOBALS_H
#define GLOBAL_H

#include <string>

#define NULL_RESPONSE "$-1\r\n"

namespace config{
    extern std::string dir;
    extern std::string dbfilename;

    void parseCommandLineArgs(int argc, char* argv[]);
}


#endif //GLOBALS_H