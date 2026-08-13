#include "GetOrderHandler.h"
#include "../Orders/OrderService.h"

http::message_generator GetOrderHandler::HandlerGetOrder(http::request<http::string_body>& req, const int64_t orderId)
{
    try
    {
        OrderService::OrderInfo order = m_orderService->GetOrder(orderId);

        nlohmann::json responseJson =
        {
            { "product_id", order.GetProductId() },
            { "amount", order.GetAmount() },
            { "email_client", order.GetEmailClient() },
            { "price", order.GetPrice() },
            { "phone_number", order.GetPhoneNumber() }
        };

        http::response<http::string_body> res{ http::status::ok, req.version() };
        res.keep_alive(req.keep_alive());
        res.set(http::field::content_type, "application/json");
        res.body() = responseJson.dump();
        res.prepare_payload();
        return res;

    }
    catch (const std::exception& e)
    {
        // Определяем тип ошибки по сообщению
        std::string errorMsg = e.what();
        http::status status = http::status::internal_server_error;
        std::string responseBody;

        if (errorMsg.find("not found") != std::string::npos ||
            errorMsg.find("does not exist") != std::string::npos)
        {
            status = http::status::not_found;
            responseBody = nlohmann::json{
                {"error", "Order not found"},
                {"order_id", orderId},
                {"status", "error"}
            }.dump();
        }
        else if (errorMsg.find("invalid") != std::string::npos)
        {
            status = http::status::bad_request;
            responseBody = nlohmann::json{
                {"error", errorMsg},
                {"status", "error"}
            }.dump();
        }
        else
        {
            status = http::status::internal_server_error;
            responseBody = nlohmann::json{
                {"error", "Internal server error"},
                {"status", "error"}
            }.dump();
        }

        http::response<http::string_body> res{ status, req.version() };
        res.keep_alive(req.keep_alive());
        res.set(http::field::content_type, "application/json");
        res.body() = responseBody;
        res.prepare_payload();
        return res;
    }
    catch (...)
    {
        // Неизвестная ошибка
        http::response<http::string_body> res{
            http::status::internal_server_error, req.version()
        };
        res.set(http::field::content_type, "application/json");
        res.body() = nlohmann::json{
            {"error", "Unknown error occurred"},
            {"status", "error"}
        }.dump();
        res.prepare_payload();
        return res;
    }
}