#pragma once

#pragma once
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/strand.hpp>
#include <nlohmann/json.hpp>
#include <functional>
#include <memory>
#include <string>

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
using tcp = boost::asio::ip::tcp;

// Структура для результата запроса
struct PaymentResponse
{
	bool success;
	std::string payment_id;
	std::string error_message;
	int http_status;
};

class PaymentClient : public std::enable_shared_from_this<PaymentClient>
{
public:
	using Callback = std::function<void(const PaymentResponse&)>;

	PaymentClient(net::io_context& ioc)
		: resolver_(net::make_strand(ioc))
		, stream_(net::make_strand(ioc))
	{}

	// Асинхронный запрос на создание платежа
	void ProcessPayment(
		const std::string& host,
		const std::string& port,
		int64_t order_id,
		double amount,
		const std::string& session_id,
		Callback callback
	)
	{
		callback_ = std::move(callback);

		// Формируем JSON запрос
		nlohmann::json payment_request = {
			{"orderId", order_id},
			{"price", amount},
			{"sessionId", session_id}
		};

		// Формируем HTTP запрос
		req_.version(11); // HTTP 1.1
		req_.method(http::verb::post);
		req_.target("/api/payments/create");
		req_.set(http::field::host, host);
		req_.set(http::field::content_type, "application/json");
		req_.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
		req_.body() = payment_request.dump();
		req_.prepare_payload();

		// Запускаем асинхронный резолвинг 
		resolver_.async_resolve(
			host,
			port,
			beast::bind_front_handler(
				&PaymentClient::on_resolve,
				shared_from_this()
			)
		);
	}

private:
	void on_resolve(beast::error_code ec, tcp::resolver::results_type results)
	{
		if (ec)
		{
			PaymentResponse response;
			response.success = false;
			response.error_message = "Resolve failed: " + ec.message();
			callback_(response);
			return;
		}

		stream_.expires_after(std::chrono::seconds(30));
		stream_.async_connect(
			results,
			beast::bind_front_handler(
				&PaymentClient::on_connect,
				shared_from_this()
			)
		);
	}

	void on_connect(beast::error_code ec, tcp::resolver::results_type::endpoint_type)
	{
		if (ec)
		{
			PaymentResponse response;
			response.success = false;
			response.error_message = "Connect failed: " + ec.message();
			callback_(response);
			return;
		}

		stream_.expires_after(std::chrono::seconds(30));
		http::async_write(
			stream_,
			req_,
			beast::bind_front_handler(
				&PaymentClient::on_write,
				shared_from_this()
			)
		);
	}

	void on_write(beast::error_code ec, std::size_t bytes_transferred)
	{
		boost::ignore_unused(bytes_transferred);

		if (ec)
		{
			PaymentResponse response;
			response.success = false;
			response.error_message = "Write failed: " + ec.message();
			callback_(response);
			return;
		}

		// Читаем ответ
		http::async_read(
			stream_,
			buffer_,
			res_,
			beast::bind_front_handler(
				&PaymentClient::on_read,
				shared_from_this()
			)
		);
	}

	void on_read(beast::error_code ec, std::size_t bytes_transferred)
	{
		boost::ignore_unused(bytes_transferred);

		PaymentResponse response;

		if (ec)
		{
			response.success = false;
			response.error_message = "Read failed: " + ec.message();
			callback_(response);
			return;
		}

		// Парсим ответ
		response.http_status = res_.result_int();

		try
		{
			auto json = nlohmann::json::parse(res_.body());

			if (response.http_status == 200 || response.http_status == 201)
			{
				if (json.contains("payment_id"))
				{
					response.success = true;
					response.payment_id = json["payment_id"].get<std::string>();
				}
				else
				{
					response.success = false;
					response.error_message = "Missing payment_id in response";
				}
			}
			else
			{
				response.success = false;
				response.error_message = json.value("error", "Unknown error");
			}
		}
		catch (const std::exception& e) {
			response.success = false;
			response.error_message = "Parse error: " + std::string(e.what());
		}

		// Закрываем соединение
		beast::error_code shutdown_ec;
		stream_.socket().shutdown(tcp::socket::shutdown_both, shutdown_ec);

		callback_(response);
	}

private:
	tcp::resolver resolver_;
	beast::tcp_stream stream_;
	beast::flat_buffer buffer_;
	http::request<http::string_body> req_;
	http::response<http::string_body> res_;
	Callback callback_;
};