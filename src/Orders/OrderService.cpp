#include "OrderService.h"
#include "../Utils/ServiceUtils.h"

int64_t OrderService::CreateOrder(const OrderInfo& orderRequest)
{
	if(orderRequest.GetAmount() <= 0)
	{
		throw std::invalid_argument("Invalid Amount");
	}
	if (orderRequest.GetPrice() < 0)
	{
		throw std::invalid_argument("Invalid Price");
	}
	if (!ServiceUtils::isValidEmail(orderRequest.GetEmailClient()))
	{
		throw std::invalid_argument("Invalid Email");
	}
	if (!ServiceUtils::IsValidPhone(orderRequest.GetPhoneNumber()))
	{
		throw std::invalid_argument("Invalid Phone Number");
	}
	return InsertOrderToDB(orderRequest);
}

OrderService::OrderInfo OrderService::GetOrder(int64_t order_id)
{
	OrderInfo order = GetOrderFromDb(order_id);
	return order;
}

bool OrderService::DeleteOrder(int64_t order_id)
{
	pqxx::work work(m_dbConnection);
	pqxx::result result = work.exec_params(
	"DELETE FROM orders WHERE order_id = $1", order_id);
	work.commit();
	int deletedCount = result.affected_rows();
	work.commit();
	if (deletedCount > 0)
		return true;

	return false;
}

int64_t OrderService::InsertOrderToDB(const OrderInfo& orderData)
{
	pqxx::work work(m_dbConnection);
	pqxx::result result = work.exec_params(
		"INSERT INTO orders (product_id, amount, email_client, price, phone_number) "
		"VALUES ($1, $2, $3, $4, $5) RETURNING order_id",
		orderData.GetProductId(),
		orderData.GetAmount(),
		orderData.GetEmailClient(),
		orderData.GetPrice(),
		orderData.GetPhoneNumber()
	);
	work.commit();
	return result[0][0].as<int64_t>();
}

OrderService::OrderInfo OrderService::GetOrderFromDb(int64_t orderId)
{
	pqxx::work work(m_dbConnection);
	pqxx::result result = work.exec_params(
		"SELECT product_id, amount, email_client, price, phone_number "
		"FROM orders WHERE order_id = $1",
		orderId
	);

	if (result.empty())
	{
		throw std::runtime_error("Order not found");
	}

	auto row = result[0];
	OrderInfo order(
		row["product_id"].as<int64_t>(),
		row["amount"].as<int32_t>(),
		row["email_client"].as<std::string>(),
		row["price"].as<double>(),
		row["phone_number"].as<std::string>()
	);
	return order;
}