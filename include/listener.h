#pragma once
#include <boost/asio.hpp>
#include <memory>
#include "server_pool.h"

class Listener : public std::enable_shared_from_this<Listener>
{
private:
    boost::asio::ip::tcp::acceptor acceptor_;
    boost::asio::ip::tcp::socket socket_;
    ServerPool &server_pool_;

public:
    Listener(boost::asio::io_context &io_context,
             boost::asio::ip::tcp::endpoint endpoint
                , ServerPool &pool);

    void run();

private:
    void do_accept();
};