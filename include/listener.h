#pragma once
#include <boost/asio.hpp>
#include <memory>

class Listener : public std::enable_shared_from_this<Listener>
{
private:
    boost::asio::ip::tcp::acceptor acceptor_;
    boost::asio::ip::tcp::socket socket_;

public:
    Listener(boost::asio::io_context &io_context,
             boost::asio::ip::tcp::endpoint endpoint);

    void run();

private:
    void do_accept();
};