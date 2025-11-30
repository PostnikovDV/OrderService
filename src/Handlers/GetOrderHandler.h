#pragma once
#include "../Utils/ServiceUtils.h"


class GetOrderHandler
{
private:
    std::shared_ptr<OrderService> m_orderService;

public:
    GetOrderHandler(std::shared_ptr<OrderService> orderService)
        : m_orderService(orderService) {}

    http::message_generator HandlerGetOrder(http::request<http::string_body>& req, const int64_t orderId);
};