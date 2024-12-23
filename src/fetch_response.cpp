#include "include/fetch_response.hpp"
#include "include/kafka_errors.hpp"
#include <array>

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
      .writeInt8(topic_count + 1);
  return *this;
}
FetchResponse &FetchResponse::writeTopicResponse(
    const std::array<uint8_t, 16> &topic_id, int32_t partition_index,
    int16_t error_code, int64_t high_watermark, int64_t last_stable_offset,
    int64_t log_start_offset,
    const std::vector<AbortedTransaction> &aborted_txns,
    int32_t preferred_read_replica, const char *records_data,
    size_t records_len, bool topic_exists) {

  if (!topic_exists) {
    writeUnknownTopicResponse(topic_id, partition_index);
  }

  // Write topic ID
  writeBytes(topic_id.data(), topic_id.size());

  // Write partition data
  writeInt32(partition_index)
      .writeInt16(error_code)
      .writeInt64(high_watermark)
      .writeInt64(last_stable_offset)
      .writeInt64(log_start_offset);

  // Write aborted transactions
  writeInt8(aborted_txns.size() + 1); // Compact array length
  for (const auto &txn : aborted_txns) {
    writeInt64(txn.producer_id)
        .writeInt64(txn.first_offset)
        .writeInt8(0); // TAG_BUFFER
  }

  writeInt32(preferred_read_replica);

  // Write records as compact bytes
  writeInt32(records_len);
  if (records_len > 0) {
    writeBytes(records_data, records_len);
  }

  writeInt8(0); // TAG_BUFFER
  return *this;
}

FetchResponse &FetchResponse::writeUnknownTopicResponse(
    const std::array<uint8_t, 16> &topic_id, int32_t partition_index) {
  writeBytes(topic_id.data(), topic_id.size())
      .writeInt32(partition_index)
      .writeInt16(ERROR_UNKNOWN_TOPIC_OR_PARTITION)
      .writeInt64(0)  // high_watermark
      .writeInt64(-1) // last_stable_offset
      .writeInt64(-1) // log_start_offset
      .writeInt8(1)   // empty aborted transactions array
      .writeInt32(-1) // preferred_read_replica
      .writeInt32(0)  // records length
      .writeInt8(0);  // TAG_BUFFER
  return *this;
}

FetchResponse &FetchResponse::complete() {
  writeInt8(0); // TAG_BUFFER
  updateMessageSize();
  return *this;
}
