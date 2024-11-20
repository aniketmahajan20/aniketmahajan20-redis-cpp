#include "./globals.h"
#include <iostream>

namespace config{
    std::string dir = "";
    std::string dbfilename = "";



    void parseCommandLineArgs(int argc, char* argv[]){
    for (int i = 0; i < argc; i++){
        std::string arg = argv[i];
        if (arg == "--dir" && i+1 < argc){
            dir = argv[++i];
        }
        if (arg == "--dbfilename" && i+1 < argc){
            dbfilename = argv[++i];
        }
    }
    }
}
