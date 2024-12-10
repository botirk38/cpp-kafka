#pragma once
#include <cstdint>

struct ApiVersionResponse {
  static constexpr int16_t API_VERSIONS_KEY = 18;
  static constexpr int16_t MIN_VERSION = 0;
  static constexpr int16_t MAX_VERSION = 4;
  static constexpr int16_t UNSUPPORTED_VERSION = 35;

  static void write(char *buffer, int &offset, int32_t correlation_id,
                    int16_t api_version);
};
