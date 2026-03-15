#pragma once
#include <vector>
#include <string>
#include <mutex>

struct BackendServer
{
    std::string host;
    std::string port;
    bool alive;
    int active_connections;
};

class ServerPool
{
private:
    std::vector<BackendServer> servers_;
    std::size_t index_;
    std::mutex mutex_;

public:
    ServerPool();

    BackendServer get_next_server();
    void mark_server_alive(const std::string& port);
    void mark_server_dead(const std::string& port);
    std::vector<BackendServer> get_servers();
    void release_connection(const std::string& port);
};