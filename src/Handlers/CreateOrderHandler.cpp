#include "CreateOrderHandler.h"

OrderService::OrderInfo CreateOrderHandler::OrderInfoFromJson(const nlohmann::json& jsonBody)
{

    std::vector<std::string> required_fields
    {
        "productId", "amount", "emailClient", "price", "phoneNumber"
    };

    for (const auto& field : required_fields)
    {
        if (!jsonBody.contains(field))
        {
            throw std::runtime_error("Missing required field: " + field);
        }
    }

    OrderService::OrderInfo order(
        jsonBody["productId"].get<int64_t>(),
        jsonBody["amount"].get<int32_t>(),
        jsonBody["emailClient"].get<std::string>(),
        jsonBody["price"].get<double>(),
        jsonBody["phoneNumber"].get<std::string>()
    );
    return order;
}

http::message_generator CreateOrderHandler::HandleCreateOrder(http::request<http::string_body>& req)
{
    try
    {
        const auto jsonBody = nlohmann::json::parse(req.body());
        OrderService::OrderInfo orderData = OrderInfoFromJson(jsonBody);

        int64_t orderId = m_orderService->CreateOrder(orderData);

        nlohmann::json responseJson = { {"orderId", orderId} };

        http::response<http::string_body> res{ http::status::ok, req.version() };
        res.keep_alive(req.keep_alive());
        res.set(http::field::content_type, "application/json");
        res.body() = responseJson.dump();
        res.prepare_payload();
        return res;

    }
    catch (std::exception& e)
    {
        http::response<http::string_body> res{ http::status::bad_request, req.version() };
        res.keep_alive(req.keep_alive());
        res.set(http::field::content_type, "application/json");
        res.body() = nlohmann::json{ { "error", e.what() } }.dump();
        res.prepare_payload();
        return res;
    }
    catch (...)
    {
        http::response<http::string_body> res{ http::status::bad_request, req.version() };
        res.keep_alive(req.keep_alive());
        res.set(http::field::content_type, "application/json");
        res.body() = nlohmann::json{ {"error", "error"} }.dump();
        res.prepare_payload();
        return res;
    }
}