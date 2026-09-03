#pragma once
#include <librdkafka/rdkafkacpp.h>
#include <nlohmann/json.hpp>
#include <memory>

class KafkaProducer
{
public:
	KafkaProducer(const std::string& brokers);
	~KafkaProducer();

	bool SendMessageKafka(const std::string& topic, const nlohmann::json& message);

	bool SendMessageKafka(const std::string& topic, const std::string& key, const nlohmann::json& message);
private:

	std::unique_ptr<RdKafka::Producer> producer;
	std::unique_ptr<RdKafka::Conf> conf_;
};