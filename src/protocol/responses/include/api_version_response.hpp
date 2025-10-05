#pragma once
#include "../../base/include/message_writer.hpp"

class ApiVersionResponse : public MessageWriter<ApiVersionResponse> {
public:
  explicit ApiVersionResponse(char *buffer) : MessageWriter(buffer) {}

  ApiVersionResponse &writeHeader(int32_t correlation_id, int16_t api_version);
  ApiVersionResponse &writeApiVersionSupport();
  ApiVersionResponse &writeDescribeTopicsSupport();
  ApiVersionResponse &writeFetchSupport();
  ApiVersionResponse &writeMetadata();
  ApiVersionResponse &complete();

  enum ApiVersions {
    KEY = 18,
    MIN_VERSION = 0,
    MAX_VERSION = 4,
    UNSUPPORTED_VERSION = 35
  };
};
