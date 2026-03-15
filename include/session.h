#pragma once
#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <memory>
#include "server_pool.h"
#include "metrics.h"

class Session : public std::enable_shared_from_this<Session>
{
private:
    boost::asio::ip::tcp::socket client_socket_;
    boost::asio::ip::tcp::socket backend_socket_;
    boost::asio::ip::tcp::resolver resolver_;
    boost::beast::flat_buffer buffer_;
    ServerPool &server_pool_;
    Metrics &metrics_;
    BackendServer current_backend_;
    boost::beast::http::request<boost::beast::http::string_body> req_;
    boost::beast::http::response<boost::beast::http::string_body> res_;
    bool is_metrics_request_ = false;

public:
    Session(boost::asio::ip::tcp::socket socket, ServerPool &pool, Metrics &metrics);

    void run();

private:
    void read_request();
    void connect_backend();
    void forward_request();
    void read_response();
    void send_response();
};