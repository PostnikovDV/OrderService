#include "GetOrderHandler.h"

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
    catch (...)
    {
        http::response<http::string_body> res{ http::status::bad_request, req.version() };
        res.set(http::field::content_type, "application/json");
        res.body() = nlohmann::json{ {"error", "error"} }.dump();
        res.prepare_payload();
        return res;
    }
}