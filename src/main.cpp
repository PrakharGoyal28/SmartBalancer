#include <boost/asio.hpp>
#include "listener.h"
#include <iostream>
#include "server_pool.h"
#include "health_checker.h"
#include "metrics.h"
#include <thread>
int main()
{
    try
    {
        boost::asio::io_context io_context{1};
        ServerPool pool;
        Metrics metrics;

        HealthChecker checker(pool);
        checker.start();

        auto address = boost::asio::ip::make_address("0.0.0.0");
        unsigned short port = 8080;

        std::make_shared<Listener>(
            io_context,
            boost::asio::ip::tcp::endpoint{address, port},
            pool,
            metrics)
            ->run();

        // Detect CPU cores
        unsigned int threads = std::thread::hardware_concurrency();
        std::cout << "Running on " << threads << " cores" << std::endl;

        // Keep io_context alive
        auto work_guard = boost::asio::make_work_guard(io_context);

        // Create worker threads
        std::vector<std::thread> workers;

        for (unsigned int i = 0; i < threads; i++)
        {
            workers.emplace_back([&io_context]() {
                io_context.run();
            });
        }

        for (auto &t : workers)
        {
            t.join();
        }
    }
    catch (std::exception &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}