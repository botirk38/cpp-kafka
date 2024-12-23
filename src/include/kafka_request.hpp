#pragma once

#include <cstdint>
#include <string>

struct RequestHeader {
  int16_t api_key;
  int16_t api_version;
  int32_t correlation_id;
  std::string client_id;
};

class KafkaRequest {
public:
  virtual ~KafkaRequest() = default;
  RequestHeader header;
};
