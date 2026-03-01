#pragma once
#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <memory>

class Session : public std::enable_shared_from_this<Session>
{
private:
    boost::asio::ip::tcp::socket client_socket_;
    boost::asio::ip::tcp::socket backend_socket_;
    boost::asio::ip::tcp::resolver resolver_;
    boost::beast::flat_buffer buffer_;

public:
    explicit Session(boost::asio::ip::tcp::socket socket);

    void run();

private:
    void read_request();
    void connect_backend();
    void forward_request();
    void read_response();
    void send_response();
};