#include <boost/asio.hpp>
#include "listener.h"
#include <iostream>

int main()
{
    try
    {
        boost::asio::io_context io_context{1};

        auto address = boost::asio::ip::make_address("0.0.0.0");
        unsigned short port = 8080;

        std::make_shared<Listener>(
            io_context,
            boost::asio::ip::tcp::endpoint{address, port})
            ->run();

        io_context.run();
    }
    catch (std::exception &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}