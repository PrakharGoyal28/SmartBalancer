#include "session.h"
#include <iostream>
#include "server_pool.h"

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
using tcp = net::ip::tcp;

http::request<http::string_body> req;
http::response<http::string_body> res;

Session::Session(tcp::socket socket, ServerPool &pool)
    : client_socket_(std::move(socket)),
      backend_socket_(client_socket_.get_executor()),
      resolver_(client_socket_.get_executor()),
      server_pool_(pool)
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

    BackendServer backend = server_pool_.get_next_server();
    current_backend_ = backend;

    resolver_.async_resolve(backend.host, backend.port,
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
                                                           else
                                                           {
                                                               std::cerr << "Backend connection failed\n";
                                                               self->server_pool_.mark_server_dead(self->current_backend_.port);
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