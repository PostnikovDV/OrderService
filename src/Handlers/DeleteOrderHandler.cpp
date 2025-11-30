#include  "DeleteOrderHandler.h"


http::message_generator DeleteOrderHandler::HandlerDeletetOrder(http::request<http::string_body>& req, const int64_t orderId)
{
    try
    {
        bool orderDelete = m_orderService->DeleteOrder(orderId);

        if (orderDelete)
        {
            nlohmann::json responseJson
            {
                {"status", "success"},
                {"message", "Order deleted successfully"},
                {"orderId", orderId}
            };

            http::response<http::string_body> res{ http::status::ok, req.version() };
            res.keep_alive(req.keep_alive());
            res.set(http::field::content_type, "application/json");
            res.body() = responseJson.dump();
            res.prepare_payload();
            return res;
        }
        else
        {
            nlohmann::json responseJson
            {
                {"status", "error"},
                {"message", "Order not found"},
                {"orderId", orderId}
            };

            http::response<http::string_body> res{ http::status::not_found, req.version() };
            res.keep_alive(req.keep_alive());
            res.set(http::field::content_type, "application/json");
            res.body() = responseJson.dump();
            res.prepare_payload();
            return res;
        }
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