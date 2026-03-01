#include "session.h"
#include <iostream>

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
using tcp = net::ip::tcp;

http::request<http::string_body> req;
http::response<http::string_body> res;

Session::Session(tcp::socket socket)
    : client_socket_(std::move(socket)),
      backend_socket_(client_socket_.get_executor()),
      resolver_(client_socket_.get_executor())
{
}

void Session::run()
{
    read_request();
}

void Session::read_request()
{
    auto self = shared_from_this();
    http::async_read(client_socket_, buffer_, req,
                     [self](beast::error_code ec, std::size_t)
                     {
                         if (!ec)
                         {
                             self->connect_backend();
                         }
                     });
}

void Session::connect_backend()
{
    auto self = shared_from_this();
    resolver_.async_resolve("127.0.0.1", "9000",
                            [self](beast::error_code ec, tcp::resolver::results_type results)
                            {
                                if (!ec)
                                {
                                    net::async_connect(self->backend_socket_, results,
                                                       [self](beast::error_code ec, tcp::endpoint)
                                                       {
                                                           if (!ec)
                                                           {
                                                               self->forward_request();
                                                           }
                                                       });
                                }
                            });
}

void Session::forward_request()
{
    auto self = shared_from_this();
    http::async_write(backend_socket_, req,
                      [self](beast::error_code ec, std::size_t)
                      {
                          if (!ec)
                          {
                              self->read_response();
                          }
                      });
}

void Session::read_response()
{
    auto self = shared_from_this();
    http::async_read(backend_socket_, buffer_, res,
                     [self](beast::error_code ec, std::size_t)
                     {
                         if (!ec)
                         {
                             self->send_response();
                         }
                     });
}

void Session::send_response()
{
    auto self = shared_from_this();
    http::async_write(client_socket_, res,
                      [self](beast::error_code ec, std::size_t)
                      {
                          self->client_socket_.shutdown(tcp::socket::shutdown_send, ec);
                      });
}