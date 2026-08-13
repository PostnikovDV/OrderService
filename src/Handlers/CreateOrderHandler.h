#pragma once
#include "../Orders/OrderService.h"

class CreateOrderHandler
{
private:
	std::shared_ptr<OrderService> m_orderService;
	OrderService::OrderInfo OrderInfoFromJson(const nlohmann::json& jsonBody);
	std::string m_sessionId;
	bool m_isNewSession = false;
public:
	CreateOrderHandler(std::shared_ptr<OrderService> orderService)
		: m_orderService(std::move(orderService)) {}

	http::message_generator HandleCreateOrder(http::request<http::string_body>& req);
};