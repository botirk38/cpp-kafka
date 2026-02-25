#pragma once

#include "api_keys.hpp"
#include "kafka_request.hpp"

class ApiVersionRequest : public KafkaRequest {
public:
  static constexpr int16_t KEY = KafkaProtocol::API_VERSIONS;
};
