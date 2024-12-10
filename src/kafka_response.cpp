#include "include/kafka_response.hpp"
#include <arpa/inet.h>
#include <cstring>

void ApiVersionResponse::write(char *buffer, int &offset,
                               int32_t correlation_id, int16_t api_version) {
  writeMessageSizePlaceholder(buffer, offset);
  writeCorrelationId(buffer, offset, correlation_id);
  writeErrorCode(buffer, offset, api_version);
  writeApiVersionsEntry(buffer, offset);
  writeDescribeTopicPartitionsEntry(buffer, offset);
  writeTagBuffersAndThrottleTime(buffer, offset);
  updateMessageSize(buffer, offset);
}

void ApiVersionResponse::writeMessageSizePlaceholder(char *buffer,
                                                     int &offset) {
  // Reserve space for message size that will be filled later
  offset += 4;
}

void ApiVersionResponse::writeCorrelationId(char *buffer, int &offset,
                                            int32_t correlation_id) {
  memcpy(buffer + offset, &correlation_id, 4);
  offset += 4;
}

void ApiVersionResponse::writeErrorCode(char *buffer, int &offset,
                                        int16_t api_version) {
  int16_t error_code =
      (api_version >= MIN_VERSION && api_version <= MAX_VERSION)
          ? 0
          : UNSUPPORTED_VERSION;
  error_code = htons(error_code);
  memcpy(buffer + offset, &error_code, 2);
  offset += 2;

  // Write number of entries
  uint8_t num_entries = 3;
  memcpy(buffer + offset, &num_entries, 1);
  offset += 1;
}

void ApiVersionResponse::writeApiVersionsEntry(char *buffer, int &offset) {
  // Write API key
  int16_t api_key = htons(API_VERSIONS_KEY);
  uint8_t tag_buffer = 0;
  memcpy(buffer + offset, &api_key, 2);
  offset += 2;

  // Write version range
  int16_t min_ver = htons(MIN_VERSION);
  int16_t max_ver = htons(MAX_VERSION);
  memcpy(buffer + offset, &min_ver, 2);
  offset += 2;
  memcpy(buffer + offset, &max_ver, 2);
  offset += 2;

  memcpy(buffer + offset, &tag_buffer, 1);
  offset += 1;
}

void ApiVersionResponse::writeTagBuffersAndThrottleTime(char *buffer,
                                                        int &offset) {
  uint8_t tag_buffer = 0;
  int32_t throttle_time = 0;

  throttle_time = htonl(throttle_time);
  memcpy(buffer + offset, &throttle_time, 4);
  offset += 4;

  memcpy(buffer + offset, &tag_buffer, 1);
  offset += 1;
}

void ApiVersionResponse::updateMessageSize(char *buffer, int final_offset) {
  // Calculate and write the total message size at the beginning of the buffer
  int32_t message_size = htonl(final_offset - 4);
  memcpy(buffer, &message_size, 4);
}

void ApiVersionResponse::writeDescribeTopicPartitionsEntry(char *buffer,
                                                           int &offset) {
  // Write API key
  int16_t api_key = htons(DESCRIBE_TOPIC_PARTITIONS_KEY);
  uint8_t tag_buffer = 0;
  memcpy(buffer + offset, &api_key, 2);
  offset += 2;

  // Write version range (both min and max are 0 for DescribeTopicPartitions)
  int16_t version = htons(0);
  memcpy(buffer + offset, &version, 2); // MinVersion = 0
  offset += 2;
  memcpy(buffer + offset, &version, 2); // MaxVersion = 0
  offset += 2;
  memcpy(buffer + offset, &tag_buffer, 1);
  offset += 1;
}
