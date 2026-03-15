#include "session.h"
#include <iostream>
#include "server_pool.h"
#include "metrics.h"

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
using tcp = net::ip::tcp;

Session::Session(tcp::socket socket, ServerPool &pool, Metrics &metrics)
    : client_socket_(std::move(socket)),
      backend_socket_(client_socket_.get_executor()),
      resolver_(client_socket_.get_executor()),
      server_pool_(pool),
      metrics_(metrics)
{
}

void Session::run()
{
    read_request();
}

void Session::read_request()
{
    auto self = shared_from_this();
    http::async_read(client_socket_, buffer_, req_,
                     [self](beast::error_code ec, std::size_t)
                     {
                         if (!ec)
                         {
                             // Check for metrics endpoint
                             if (self->req_.target() == "/metrics")
                             {
                                 self->is_metrics_request_ = true;
                                 self->res_.result(http::status::ok);
                                 self->res_.set(http::field::content_type, "text/plain");
                                 self->res_.body() = self->metrics_.get_metrics();
                                 self->res_.prepare_payload();
                                 self->send_response();
                             }
                             else
                             {
                                 self->metrics_.increment_requests();
                                 self->metrics_.connection_open();
                                 self->connect_backend();
                             }
                         }
                     });
}

void Session::connect_backend()
{
    auto self = shared_from_this();

    BackendServer backend = server_pool_.get_next_server();
    current_backend_ = backend;
    metrics_.record_server_request(backend.port);

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
                                                               self->metrics_.increment_failed();
                                                           }
                                                       });
                                }
                            });
}

void Session::forward_request()
{
    auto self = shared_from_this();
    http::async_write(backend_socket_, req_,
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
    http::async_read(backend_socket_, buffer_, res_,
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
    http::async_write(client_socket_, res_,
                      [self](beast::error_code ec, std::size_t)
                      {
                          if (!self->is_metrics_request_)
                          {
                              self->server_pool_.release_connection(self->current_backend_.port);
                              self->metrics_.connection_close();
                          }
                          self->client_socket_.shutdown(tcp::socket::shutdown_send, ec);
                      });
}