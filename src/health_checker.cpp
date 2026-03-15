#include "health_checker.h"
#include <boost/asio.hpp>
#include <iostream>
#include <chrono>
#include <thread>

using tcp = boost::asio::ip::tcp;

HealthChecker::HealthChecker(ServerPool &pool)
    : server_pool_(pool), running_(true) {}

void HealthChecker::start()
{
    std::thread([this]() { run(); }).detach();
}

void HealthChecker::stop()
{
    running_ = false;
}

void HealthChecker::run()
{
    while (running_)
    {
        auto servers = server_pool_.get_servers();

        for (auto &server : servers)
        {
            try
            {
                boost::asio::io_context io;
                tcp::resolver resolver(io);
                tcp::socket socket(io);

                auto endpoints = resolver.resolve(server.host, server.port);

                boost::asio::connect(socket, endpoints);

                server_pool_.mark_server_alive(server.port);

            }
            catch (...)
            {
                server_pool_.mark_server_dead(server.port);
            }
        }

        std::this_thread::sleep_for(std::chrono::seconds(5));
    }
}
