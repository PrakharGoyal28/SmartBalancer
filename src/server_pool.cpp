#include "server_pool.h"

ServerPool::ServerPool() : index_(0)
{
    servers_.push_back({"127.0.0.1", "9000"});
    servers_.push_back({"127.0.0.1", "9001"});
    servers_.push_back({"127.0.0.1", "9002"});
}

BackendServer ServerPool::get_next_server()
{
    std::lock_guard<std::mutex> lock(mutex_);
    BackendServer server = servers_[index_];
    index_ = (index_ + 1) % servers_.size();
    return server;
}