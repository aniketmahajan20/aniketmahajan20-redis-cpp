#include "./server_info.hpp"

std::string ServerInfo::get_role(){
    return role;
}
int ServerInfo::get_connected_slaves(){
    return connected_slaves;
}
std::string ServerInfo::get_master_replid(){
    return master_replid;
}
int ServerInfo::get_master_repl_offset(){
    return master_repl_offset;
}
int ServerInfo::get_second_repl_offset()
{
    return second_repl_offset;
}
int ServerInfo::get_repl_backlog_active(){
    return repl_backlog_active;
}
int ServerInfo::get_repl_backlog_size(){
    return repl_backlog_size;
}
int ServerInfo::get_repl_backlog_first_byte_offset(){
    return repl_backlog_first_byte_offset;
}
int ServerInfo::get_repl_backlog_histlen(){
    return repl_backlog_histlen;
}
void ServerInfo::updateInfo(std::string role){
    this->role.assign(role);
}
void ServerInfo::updateInfo(std::string role, int connected_slaves){
    this->role.assign(role);
    this->connected_slaves = connected_slaves;
}