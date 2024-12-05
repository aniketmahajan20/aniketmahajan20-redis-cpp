#ifndef SERVERINFO_HPP
#define SERVERINFO_HPP

#include <string>
#include <vector>

class ServerInfo {
public:
    ServerInfo() {
        role = "master";
        connected_slaves = -1;
        master_replid = "";
        second_repl_offset = -1;
        repl_backlog_active = -1;
        repl_backlog_size = -1;
        repl_backlog_first_byte_offset = -1;
        repl_backlog_histlen = -1;
    }
    std::string get_role();
    int get_connected_slaves();
    std::string get_master_replid();
    int get_master_repl_offset();
    int get_second_repl_offset();
    int get_repl_backlog_active();
    int get_repl_backlog_size();
    int get_repl_backlog_first_byte_offset();
    int get_repl_backlog_histlen();
    void updateInfo(std::string role);
    void updateInfo(std::string role, int connected_slaves);
private:
    std::string role;
    int connected_slaves;
    std::string master_replid;
    int master_repl_offset;
    int second_repl_offset;
    int repl_backlog_active;
    int repl_backlog_size;
    int repl_backlog_first_byte_offset;
    int repl_backlog_histlen;
};

#endif //SERVERINFO_HPP