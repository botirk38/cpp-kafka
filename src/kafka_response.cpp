#include "include/kafka_response.hpp"
#include <arpa/inet.h>
#include <cstring>

void ApiVersionResponse::write(char *buffer, int &offset,
                               int32_t correlation_id, int16_t api_version) {
  // Reserve space for message size
  offset += 4;

  // Correlation ID
  memcpy(buffer + offset, &correlation_id, 4);
  offset += 4;

  // Error code
  int16_t error_code =
      (api_version >= MIN_VERSION && api_version <= MAX_VERSION)
          ? 0
          : UNSUPPORTED_VERSION;
  error_code = htons(error_code);
  memcpy(buffer + offset, &error_code, 2);
  offset += 2;

  // Number of entries
  uint8_t num_entries = 2;
  memcpy(buffer + offset, &num_entries, 1);
  offset += 1;

  // API Versions entry
  int16_t api_key = htons(API_VERSIONS_KEY);
  memcpy(buffer + offset, &api_key, 2);
  offset += 2;

  // Version range
  int16_t min_ver = htons(MIN_VERSION);
  int16_t max_ver = htons(MAX_VERSION);
  memcpy(buffer + offset, &min_ver, 2);
  offset += 2;
  memcpy(buffer + offset, &max_ver, 2);
  offset += 2;

  // Tag buffers and throttle time
  uint8_t tag_buffer = 0;
  int32_t throttle_time = 0;

  memcpy(buffer + offset, &tag_buffer, 1);
  offset += 1;

  throttle_time = htonl(throttle_time);
  memcpy(buffer + offset, &throttle_time, 4);
  offset += 4;

  memcpy(buffer + offset, &tag_buffer, 1);
  offset += 1;

  // Set message size
  int32_t message_size = htonl(offset - 4);
  memcpy(buffer, &message_size, 4);
}
