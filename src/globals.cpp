#include "./globals.h"

namespace config{
    std::string dir;
    std::string dbfilename;
    int port = DEFUALT_PORT;

    void parseCommandLineArgs(int argc, char* argv[], ServerInfo& server_info){
        for (int i = 0; i < argc; i++){
            std::string arg = argv[i];
            if (arg == "--dir" && i+1 < argc){
                dir = argv[++i];
            }
            if (arg == "--dbfilename" && i+1 < argc){
                dbfilename = argv[++i];
            }
            if (arg == "--port" && i+1 < argc){
                port = std::stoi(argv[++i]);
            }
            if (arg == "--replicaof" && i+1 < argc){
                server_info.updateInfo("slave");
            }
        }
    }
}
