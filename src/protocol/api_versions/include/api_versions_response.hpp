#pragma once
#include "../../base/include/message_writer.hpp"

class ApiVersionsResponse : public MessageWriter<ApiVersionsResponse> {
public:
  explicit ApiVersionsResponse(char *buffer) : MessageWriter(buffer) {}

  ApiVersionsResponse &writeHeader(int32_t correlation_id, int16_t api_version);
  ApiVersionsResponse &writeApiVersionSupport();
  ApiVersionsResponse &writeDescribeTopicsSupport();
  ApiVersionsResponse &writeFetchSupport();
  ApiVersionsResponse &writeMetadata();
  ApiVersionsResponse &complete();
};
