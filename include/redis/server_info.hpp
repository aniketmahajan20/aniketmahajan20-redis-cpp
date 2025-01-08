#ifndef SERVERINFO_HPP
#define SERVERINFO_HPP

#include <string>
#include <vector>

class ServerInfo {
public:
    ServerInfo() {
        role = "master";
        connected_slaves = 0;
        master_ip = "";
        master_port = 0;
        master_replid = this->generate_master_replid();
        master_repl_offset = 0;
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
    void update_role(std::string role);
    void update_connected_slaves(int connected_slaves);
    void send_handshake();
    void get_master_ip_port(const std::string& args);
    void add_connected_slave(int fd);
    int get_connected_slave_fd(int index);
private:
    std::string role;
    int connected_slaves;
    std::string master_ip;
    int master_port;
    std::string master_replid;
    int master_repl_offset;
    int second_repl_offset;
    int repl_backlog_active;
    int repl_backlog_size;
    int repl_backlog_first_byte_offset;
    int repl_backlog_histlen;
    std::vector<int> connected_slaves_fd;
    std::string generate_master_replid();
};

#endif //SERVERINFO_HPP