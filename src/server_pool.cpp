#include "server_pool.h"
#include <stdexcept>

ServerPool::ServerPool() : index_(0)
{
    servers_.push_back({"127.0.0.1", "9000",true});
    servers_.push_back({"127.0.0.1", "9001",true});
    servers_.push_back({"127.0.0.1", "9002",true});
}

BackendServer ServerPool::get_next_server()
{
    std::lock_guard<std::mutex> lock(mutex_);

    for (size_t i = 0; i < servers_.size(); i++)
    {
        BackendServer &server = servers_[index_];

        index_ = (index_ + 1) % servers_.size();

        if (server.alive)
        {
            return server;
        }
    }

    throw std::runtime_error("No backend servers available");
}

void ServerPool::mark_server_dead(const std::string& port)
{
    std::lock_guard<std::mutex> lock(mutex_);

    for (auto &server : servers_)
    {
        if (server.port == port)
        {
            server.alive = false;
        }
    }
}