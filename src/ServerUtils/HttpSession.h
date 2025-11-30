#pragma once
#include "../Utils/ServiceUtils.h"


class HttpSession : public std::enable_shared_from_this<HttpSession>
{
    beast::tcp_stream stream_;
    beast::flat_buffer buffer_;
    http::request<http::string_body> req_;
    std::shared_ptr<std::string const> doc_root_;
public:
    HttpSession(tcp::socket&& socket, std::shared_ptr<std::string const> const& doc_root)
        : stream_(std::move(socket)), doc_root_(doc_root)
    {}
    void run()
    {
        net::dispatch(stream_.get_executor(),
            beast::bind_front_handler(&HttpSession::do_read,
                shared_from_this()));
    }

private:
    void do_read()
    {
        req_ = {};

        stream_.expires_after(std::chrono::seconds(30));

        http::async_read(stream_, buffer_, req_,
            beast::bind_front_handler(&HttpSession::on_read,
                shared_from_this())
        );
    }

    void on_read(beast::error_code ec, std::size_t bytes_transferred) {
        boost::ignore_unused(bytes_transferred);

        if (ec == http::error::end_of_stream)
            return do_eof();
        if (ec)
            return fail(ec, "read");

        send_response(ServiceUtils::HandleRequest(*doc_root_, std::move(req_)));
    }

    void send_response(http::message_generator&& msg)
    {
        bool keep_alive = msg.keep_alive();

        beast::async_write(
            stream_,
            std::move(msg),
            beast::bind_front_handler(
                &HttpSession::on_write, shared_from_this(), keep_alive));
    }

    void on_write(
        bool keep_alive,
        beast::error_code ec,
        std::size_t bytes_transferred)
    {
        boost::ignore_unused(bytes_transferred);

        if (ec)
            return fail(ec, "write");

        if (!keep_alive)
        {
            return do_eof();
        }

        do_read();
    }

    void do_eof()
    {
        beast::error_code ec;
        stream_.socket().shutdown(tcp::socket::shutdown_send, ec);
    }

    void fail(beast::error_code ec, char const* what)
    {
        std::cerr << what << ": " << ec.message() << std::endl;
    }
};