#pragma once
#include <cstdint>

struct ApiVersionResponse {
  static constexpr int16_t API_VERSIONS_KEY = 18;
  static constexpr int16_t MIN_VERSION = 0;
  static constexpr int16_t MAX_VERSION = 4;
  static constexpr int16_t UNSUPPORTED_VERSION = 35;
  static constexpr int16_t DESCRIBE_TOPIC_PARTITIONS_KEY = 75;

  static void write(char *buffer, int &offset, int32_t correlation_id,
                    int16_t api_version);

private:
  static void writeMessageSizePlaceholder(char *buffer, int &offset);
  static void writeCorrelationId(char *buffer, int &offset,
                                 int32_t correlation_id);
  static void writeErrorCode(char *buffer, int &offset, int16_t api_version);
  static void writeApiVersionsEntry(char *buffer, int &offset);
  static void writeTagBuffersAndThrottleTime(char *buffer, int &offset);
  static void updateMessageSize(char *buffer, int final_offset);
  static void writeDescribeTopicPartitionsEntry(char *buffer, int &offset);
};
