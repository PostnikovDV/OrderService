#include "OrderService.h"

#include "../Clients/PaymentClient.h"

#include "../Utils/ServiceUtils.h"
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>


OrderService::OrderService(const std::string& dbConnection, net::io_context& ioc) :
	m_dbConnection(dbConnection)
	, ioc_(ioc)
	, payment_client_(std::make_shared<PaymentClient>(ioc))
{}

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
		"INSERT INTO orders (product_id, amount, email_client, price, phone_number, session_id) "
		"VALUES ($1, $2, $3, $4, $5, $6) RETURNING order_id",
		orderData.GetProductId(),
		orderData.GetAmount(),
		orderData.GetEmailClient(),
		orderData.GetPrice(),
		orderData.GetPhoneNumber(),
		orderData.GetSessionId()
	);
	work.commit();
	return result[0][0].as<int64_t>();
}

OrderService::OrderInfo OrderService::GetOrderFromDb(int64_t orderId)
{
	pqxx::work work(m_dbConnection);
	pqxx::result result = work.exec_params(
		"SELECT product_id, amount, email_client, price, phone_number, session_id"
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
		row["phone_number"].as<std::string>(),
		row["session_id"].as<std::string>()
	);
	return order;
}

std::string OrderService::GenerateSessionId()
{
	boost::uuids::random_generator gen;
	boost::uuids::uuid guid = gen();
	return boost::uuids::to_string(guid);
}

void OrderService::ProcessPayment(
	int64_t order_id,
	double amount,
	const std::string& session_id
)
{
	payment_client_->ProcessPayment(
		"payment_app",  // Хост (в Docker Compose)
		"8282",             // Порт PaymentService
		order_id,
		amount,
		session_id,
		[this, order_id](const PaymentResponse& response) {
			if (response.success) {
				std::cout << "Payment successful for order " << order_id
					<< ", payment_id: " << response.payment_id << std::endl;

				// Можно обновить статус в БД (если добавили колонку status)
			}
			else {
				std::cerr << "Payment failed for order " << order_id
					<< ": " << response.error_message << std::endl;

				// Можно обновить статус в БД
			}
		}
	);
}