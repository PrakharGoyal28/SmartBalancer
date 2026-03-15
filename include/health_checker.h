#pragma once
#include "server_pool.h"
#include <thread>
#include <atomic>

class HealthChecker
{
private:
    ServerPool &server_pool_;
    std::atomic<bool> running_;

public:
    HealthChecker(ServerPool &pool);

    void start();
    void stop();

private:
    void run();
};
