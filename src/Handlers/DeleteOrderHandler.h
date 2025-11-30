#pragma once
#include "../Utils/ServiceUtils.h"


class DeleteOrderHandler
{
private:
    std::shared_ptr<OrderService> m_orderService;

public:
    DeleteOrderHandler(std::shared_ptr<OrderService> orderService)
        : m_orderService(orderService) {}

    http::message_generator HandlerDeletetOrder(http::request<http::string_body>& req, const int64_t orderId);
};