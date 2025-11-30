#pragma once
#include "HttpSession.h"

class HttpListener : public std::enable_shared_from_this<HttpListener>
{
    net::io_context& ioc_;
    tcp::acceptor acceptor_;
    std::shared_ptr<std::string const> doc_root_;
public:
    HttpListener(net::io_context& ioc, tcp::endpoint endpoint,
        std::shared_ptr<std::string const> const& doc_root)
        : ioc_(ioc), acceptor_(net::make_strand(ioc)), doc_root_(doc_root)
    {
        beast::error_code ec;

        acceptor_.open(endpoint.protocol(), ec);
        if (ec) throw beast::system_error{ ec, "open" };

        acceptor_.set_option(net::socket_base::reuse_address(true), ec);
        if (ec) throw beast::system_error{ ec, "set_option" };

        acceptor_.bind(endpoint, ec);
        if (ec)
            throw beast::system_error{ ec, "bind" };

        acceptor_.listen(net::socket_base::max_listen_connections, ec);
        if (ec)
            throw beast::system_error{ ec, "listen" };
    }
    void run()
    {
        do_accept();
    }

private:
    void do_accept()
    {
        acceptor_.async_accept(
            net::make_strand(ioc_),
            beast::bind_front_handler(&HttpListener::on_accept,
                shared_from_this()));
    }

    void on_accept(beast::error_code ec, tcp::socket socket)
    {
        if (ec)
        {
            fail(ec, "accept");
        }
        else
        {
            std::make_shared<HttpSession>(std::move(socket), doc_root_)->run();
        }

        do_accept();
    }

    void fail(beast::error_code ec, char const* what)
    {
        std::cerr << what << ": " << ec.message() << std::endl;
    }
};