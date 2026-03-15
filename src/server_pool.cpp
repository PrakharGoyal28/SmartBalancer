#include "server_pool.h"
#include <stdexcept>

ServerPool::ServerPool() : index_(0)
{
    servers_.push_back({"127.0.0.1", "9000", true, 0});
    servers_.push_back({"127.0.0.1", "9001", true, 0});
    servers_.push_back({"127.0.0.1", "9002", true, 0});
}

BackendServer ServerPool::get_next_server()
{
    std::lock_guard<std::mutex> lock(mutex_);

    BackendServer* best = nullptr;

    for (auto &server : servers_)
    {
        if (!server.alive)
            continue;

        if (!best || server.active_connections < best->active_connections)
        {
            best = &server;
        }
    }

    if (!best)
    {
        throw std::runtime_error("No backend servers available");
    }

    best->active_connections++;

    return *best;
}

void ServerPool::mark_server_alive(const std::string& port)
{
    std::lock_guard<std::mutex> lock(mutex_);

    for (auto &server : servers_)
    {
        if (server.port == port)
        {
            server.alive = true;
        }
    }
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

std::vector<BackendServer> ServerPool::get_servers()
{
    std::lock_guard<std::mutex> lock(mutex_);
    return servers_;
}

void ServerPool::release_connection(const std::string& port)
{
    std::lock_guard<std::mutex> lock(mutex_);

    for (auto &server : servers_)
    {
        if (server.port == port && server.active_connections > 0)
        {
            server.active_connections--;
        }
    }
}