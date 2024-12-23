#pragma once
#include "message_writer.hpp"
#include <vector>

using uint128_t = __uint128_t;

class FetchResponse : public MessageWriter<FetchResponse> {

public:
  enum Fetch { KEY = 1, MAX_VERSION = 16, MIN_VERSION = 0 };

  struct AbortedTransaction {
    int64_t producer_id;
    int64_t first_offset;
  };

  explicit FetchResponse(char *buf) : MessageWriter(buf) {}

  FetchResponse &writeHeader(int32_t correlation_id, int16_t error_code,
                             int32_t session_id, int8_t topic_count);

  FetchResponse &
  writeTopicResponse(uint128_t topic_id, int32_t partition_index,
                     int16_t error_code, int64_t high_watermark,
                     int64_t last_stable_offset, int64_t log_start_offset,
                     const std::vector<AbortedTransaction> &aborted_txns,
                     int32_t preferred_read_replica, const char *records_data,
                     size_t records_len, bool topic_exists);

  FetchResponse &complete();

private:
};
