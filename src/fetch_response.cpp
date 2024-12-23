#include "include/fetch_response.hpp"
#include "include/kafka_errors.hpp"

FetchResponse &FetchResponse::writeHeader(int32_t correlation_id,
                                          int16_t error_code,
                                          int32_t session_id,
                                          int8_t topic_count) {
  skipBytes(4) // Message size placeholder
      .writeInt32(correlation_id)
      .writeInt8(0)  // TAG_BUFFER
      .writeInt32(0) // throttle_time_ms
      .writeInt16(error_code)
      .writeInt32(session_id)
      .writeInt8(topic_count);
  return *this;
}

FetchResponse &FetchResponse::writeTopicResponse(
    uint128_t topic_id, int32_t partition_index, int64_t high_watermark,
    int64_t last_stable_offset, int64_t log_start_offset,
    const std::vector<AbortedTransaction> &aborted_txns,
    int32_t preferred_read_replica, const char *records_data,
    size_t records_len, bool topic_exists) {

  writeUint128(topic_id)
      .writeInt8(2) // Number of partitions
      .writeInt32(partition_index)
      .writeInt16(topic_exists ? 0 : ERROR_UNKNOWN_TOPIC)
      .writeInt64(high_watermark)
      .writeInt64(last_stable_offset)
      .writeInt64(log_start_offset)
      .writeInt8(0) // Empty aborted transactions array
      .writeInt32(aborted_txns.size() + 1)
      .writeInt32(1) // Records length
      .writeInt8(0); // TAG_BUFFER

  return *this;
}

FetchResponse &FetchResponse::complete() {
  writeInt8(0); // Final TAG_BUFFER
  updateMessageSize();
  return *this;
}
