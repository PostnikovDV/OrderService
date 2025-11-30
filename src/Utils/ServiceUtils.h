#pragma once
#include "../Orders/OrderService.h"


namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
using tcp = boost::asio::ip::tcp;

namespace ServiceUtils
{
    beast::string_view mime_type(beast::string_view path);
    std::string path_cat(beast::string_view base, beast::string_view path);

    OrderService::OrderInfo OrderInfoFromJson(const nlohmann::json& jsonBody);

    bool isValidEmail(const std::string& email);

    bool IsValidPhone(const std::string& phone);
    bool IsValidInternationalFormat(const std::string& phone);
    bool IsValidRussianFormat(const std::string& phone);
    bool IsValidSimpleFormat(const std::string& phone);

    template <class Body, class Allocator>
    http::message_generator HandleRequest(
        beast::string_view doc_root,
        http::request<Body, http::basic_fields<Allocator>>&& req
    )
    {
        auto const bad_request =
            [&req](beast::string_view why)
        {
            http::response<http::string_body> res{ http::status::bad_request, req.version() };
            res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
            res.set(http::field::content_type, "text/html");
            res.keep_alive(req.keep_alive());
            res.body() = std::string(why);
            res.prepare_payload();
            return res;
        };

        auto const not_found =
            [&req](beast::string_view target)
        {
            http::response<http::string_body> res{ http::status::not_found, req.version() };
            res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
            res.set(http::field::content_type, "text/html");
            res.keep_alive(req.keep_alive());
            res.body() = "The resource '" + std::string(target) + "' was not found.";
            res.prepare_payload();
            return res;
        };

        auto const server_error =
            [&req](beast::string_view what)
        {
            http::response<http::string_body> res{ http::status::internal_server_error, req.version() };
            res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
            res.set(http::field::content_type, "text/html");
            res.keep_alive(req.keep_alive());
            res.body() = "An error occurred: '" + std::string(what) + "'";
            res.prepare_payload();
            return res;
        };

        std::shared_ptr<OrderService> orderService = std::make_shared<OrderService>(dbManager->GetConnectionString());

        if (req.method() != http::verb::get &&
            req.method() != http::verb::head &&
            req.method() != http::verb::post &&
            req.method() != http::verb::delete_)
            return bad_request("Unknown HTTP-method");

        if (req.target().empty() ||
            req.target()[0] != '/' ||
            req.target().find("..") != beast::string_view::npos)
            return bad_request("Illegal request-target");

        if (req.method() == http::verb::get)
        {

            if (req.target().back() == '/')
            {
                std::string path = ServiceUtils::path_cat(doc_root, req.target());
                path.append("index.html");

                beast::error_code ec;
                http::file_body::value_type body;
                body.open(path.c_str(), beast::file_mode::scan, ec);

                if (ec)
                    return server_error(ec.message());

                if (ec == beast::errc::no_such_file_or_directory)
                    return not_found(req.target());

                auto const size = body.size();
                http::response<http::file_body> res{
                       std::piecewise_construct,
                       std::make_tuple(std::move(body)),
                       std::make_tuple(http::status::ok, req.version())
                };
                res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
                res.set(http::field::content_type, ServiceUtils::mime_type(path));
                res.content_length(size);
                res.keep_alive(req.keep_alive());
                return res;
            }
            else if (req.target().starts_with("/api/orders/"))
            {
                GetOrderHandler handler(orderService);
                std::regex order_pattern("/api/orders/(\\d+)");
                std::smatch matches;
                std::string targetStr = std::string(req.target());

                size_t lastSlash = targetStr.find_last_of('/');
                std::string target = std::string(req.target());
                if (std::regex_match(target, matches, order_pattern) && matches.size() > 1)
                {
                    int64_t orderId = std::stoll(matches[1].str());

                    return handler.HandlerGetOrder(req, orderId);
                }
                else
                {
                    return bad_request("Invalid order URL");
                }
            }
            else
            {
                return not_found(std::string(req.target()));
            }
        }
        else if (req.method() == http::verb::post)
        {
            if (req.target() == "/api/orders/create")
            {
                CreateOrderHandler handler(orderService);
                return handler.HandleCreateOrder(req);
            }
            else
            {
                return not_found(std::string(req.target()));
            }
        }
        else if (req.method() == http::verb::delete_)
        {
            if (req.target().starts_with("/api/orders/"))
            {
                DeleteOrderHandler handler(orderService);
                std::regex order_pattern("/api/orders/(\\d+)");
                std::smatch matches;
                std::string targetStr = std::string(req.target());

                size_t lastSlash = targetStr.find_last_of('/');
                std::string target = std::string(req.target());
                if (std::regex_match(target, matches, order_pattern) && matches.size() > 1)
                {
                    int64_t orderId = std::stoll(matches[1].str());

                    return handler.HandlerDeletetOrder(req, orderId);
                }
                else
                {
                    return bad_request("Invalid order URL");
                }
            }
            else
            {
                return not_found(std::string(req.target()));
            }
        }
        return not_found(std::string(req.target()));
    }
}