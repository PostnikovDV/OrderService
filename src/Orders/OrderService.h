#pragma once

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
using tcp = boost::asio::ip::tcp;

class PaymentClient;
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
			std::string phoneNumber,
			std::string sessionId = ""
		) :
			m_productId(productId)
			, m_amount(amount)
			, m_emailClient(std::move(emailClient))
			, m_price(price)
			, m_phoneNumber(std::move(phoneNumber))
			, m_sessionId(std::move(sessionId))
		{

		}
		int64_t GetProductId() const { return m_productId; }
		int32_t GetAmount() const { return m_amount; }
		std::string GetEmailClient() const { return m_emailClient; }
		double GetPrice() const { return m_price; }
		std::string GetPhoneNumber() const { return m_phoneNumber; }
		std::string GetSessionId() const { return m_sessionId; }
	private:
		int64_t m_productId;
		int32_t m_amount;
		std::string m_emailClient;
		double m_price;
		std::string m_phoneNumber;
		std::string m_sessionId;
	};

	OrderService(const std::string& dbConnection, net::io_context& ioc);

	int64_t CreateOrder(const OrderInfo& orderRequest);
	OrderInfo GetOrder(int64_t order_id);
	bool DeleteOrder(int64_t order_id);
	std::string GenerateSessionId();

	void ProcessPayment(
		int64_t order_id,
		double amount,
		const std::string& session_id
	);

private:
	int64_t InsertOrderToDB(const OrderInfo& request);
	OrderInfo GetOrderFromDb(int64_t orderId);
private:
	std::shared_ptr<PaymentClient> payment_client_;
	net::io_context& ioc_;
	pqxx::connection m_dbConnection;
};