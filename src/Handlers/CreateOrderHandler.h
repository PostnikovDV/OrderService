#pragma once
#include "../Utils/ServiceUtils.h"

class CreateOrderHandler
{
private:
    std::shared_ptr<OrderService> m_orderService;

public:
    CreateOrderHandler(std::shared_ptr<OrderService> orderService)
        : m_orderService(orderService) {}

    http::message_generator HandleCreateOrder(http::request<http::string_body>& req);
};