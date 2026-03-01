#pragma once
#include <vector>
#include <string>
#include <mutex>

struct BackendServer
{
    std::string host;
    std::string port;
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
};