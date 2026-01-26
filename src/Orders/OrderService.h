#pragma once

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
using tcp = boost::asio::ip::tcp;

class OrderService
{
public:
	class OrderInfo
	{
	public:
		OrderInfo(
			int64_t productId,
			int32_t amount,
			std::string emailClient,
			double price,
			std::string phoneNumber
		) : m_productId(productId), m_amount(amount), m_emailClient(emailClient), m_price(price), m_phoneNumber(phoneNumber)
		{

		}
		int64_t GetProductId() const { return m_productId; }
		int32_t GetAmount() const { return m_amount; }
		std::string GetEmailClient() const { return m_emailClient; }
		double GetPrice() const { return m_price; };
		std::string GetPhoneNumber() const { return m_phoneNumber; };
	private:
		int64_t m_productId;
		int32_t m_amount;
		std::string m_emailClient;
		double m_price;
		std::string m_phoneNumber;
	};

	OrderService(const std::string& dbConnection)
		: m_dbConnection(dbConnection) {}

	int64_t CreateOrder(const OrderInfo& orderRequest);
	OrderInfo GetOrder(int64_t order_id);
	bool DeleteOrder(int64_t order_id);
private:
	int64_t InsertOrderToDB(const OrderInfo& request);
	OrderInfo GetOrderFromDb(int64_t orderId);
	pqxx::connection m_dbConnection;

};