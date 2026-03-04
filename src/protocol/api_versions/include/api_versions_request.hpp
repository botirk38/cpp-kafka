#pragma once

#include "../../base/include/api_keys.hpp"
#include "../../base/include/kafka_request.hpp"

class ApiVersionRequest : public KafkaRequest {
public:
  static constexpr int16_t KEY = KafkaProtocol::API_VERSIONS;
};
