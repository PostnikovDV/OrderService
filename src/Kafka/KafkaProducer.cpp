#include "../stdafx.h"

#include "KafkaProducer.h"


KafkaProducer::KafkaProducer(const std::string& brokers)
{
	std::string errstr;
	conf_.reset(RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL));

	if (conf_->set("bootstrap.servers", brokers, errstr) != RdKafka::Conf::CONF_OK)
	{
		std::cerr << "Failed to set bootstrap.servers: " << errstr << std::endl;
	}

	if (conf_->set("acks", "all", errstr) != RdKafka::Conf::CONF_OK)
	{
		std::cerr << "Failed to set acks: " << errstr << std::endl;
	}

	if (conf_->set("retries", "3", errstr) != RdKafka::Conf::CONF_OK)
	{
		std::cerr << "Failed to set retries: " << errstr << std::endl;
	}

	if (conf_->set("compression.type", "snappy", errstr) != RdKafka::Conf::CONF_OK)
	{
		std::cerr << "Failed to set compression.type: " << errstr << std::endl;
	}

	producer.reset(RdKafka::Producer::create(conf_.get(), errstr));
	if (!producer)
	{
		std::cerr << "Failed to create producer: " << errstr << std::endl;
	}
}

KafkaProducer::~KafkaProducer()
{
	if (producer)
	{
		producer->flush(5000);
	}
}

bool KafkaProducer::SendMessageKafka(const std::string& topic, const nlohmann::json& message)
{
	return SendMessageKafka(topic, "", message);
}

bool KafkaProducer::SendMessageKafka(const std::string& topic, const std::string& key, const nlohmann::json& message)
{
	if (!producer)
	{
		std::cerr << "Kafka producer is null!" << std::endl;
		return false;
	}

	try
	{
		std::string payload = message.dump();

		RdKafka::ErrorCode err = producer->produce(
			topic,
			RdKafka::Topic::PARTITION_UA,
			RdKafka::Producer::RK_MSG_COPY,
			const_cast<char*>(payload.c_str()),
			payload.size(),
			key.empty() ? nullptr : key.c_str(),
			key.empty() ? 0 : key.size(),
			0,
			nullptr
		);

		if (err != RdKafka::ERR_NO_ERROR)
		{
			std::cerr << "Failed to send: " << RdKafka::err2str(err) << std::endl;
			return false;
		}

		producer->poll(0);
		std::cout << "Message sent to " << topic << std::endl;
		return true;

	}
	catch (const std::exception& e)
	{
		std::cerr << "Kafka exception: " << e.what() << std::endl;
		return false;
	}
	catch (...)
	{
		std::cerr << "Kafka unknown exception!" << std::endl;
		return false;
	}
}