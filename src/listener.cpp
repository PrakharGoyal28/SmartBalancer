#include "listener.h"
#include "session.h"

Listener::Listener(boost::asio::io_context &io_context,
                   boost::asio::ip::tcp::endpoint endpoint)
    : acceptor_(io_context), socket_(io_context)
{
    acceptor_.open(endpoint.protocol());
    acceptor_.set_option(boost::asio::socket_base::reuse_address(true));
    acceptor_.bind(endpoint);
    acceptor_.listen();
}

void Listener::run()
{
    do_accept();
}

void Listener::do_accept()
{
    acceptor_.async_accept(socket_,
                           [self = shared_from_this()](boost::system::error_code ec)
                           {
                               if (!ec)
                               {
                                   std::make_shared<Session>(std::move(self->socket_))->run();
                               }
                               self->do_accept();
                           });
}