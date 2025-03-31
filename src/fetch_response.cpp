#include "include/fetch_response.hpp"
#include <crc32c/crc32c.h>
#include <cstdint>

FetchResponse &FetchResponse::writeHeader(int32_t correlation_id) {
  skipBytes(4) // Message size placeholder
      .writeInt32(correlation_id)
      .writeInt8(0); // TAG_BUFFER
  return *this;
}

FetchResponse &FetchResponse::writeResponseData(int32_t throttle_time_ms,
                                                int16_t error_code,
                                                int32_t session_id,
                                                int64_t topic_count) {
  std::cout << "Topic Count: " << topic_count;
  writeInt32(throttle_time_ms)
      .writeInt16(error_code)
      .writeInt32(session_id)
      .writeVarInt(static_cast<int64_t>(topic_count));
  return *this;
}

FetchResponse &FetchResponse::writeTopicHeader(uint128_t topic_id,
                                               int64_t partition_count) {
  writeUint128(topic_id).writeVarInt(static_cast<int64_t>(partition_count));
  return *this;
}

FetchResponse &FetchResponse::writePartitionHeader(
    int32_t partition_index, int16_t error_code, int64_t high_watermark,
    int64_t last_stable_offset, int64_t log_start_offset,
    int32_t preferred_read_replica) {
  writeInt32(partition_index)
      .writeInt16(error_code)
      .writeInt64(high_watermark)
      .writeInt64(last_stable_offset)
      .writeInt64(log_start_offset)
      .writeInt32(preferred_read_replica);
  return *this;
}

FetchResponse &FetchResponse::writeAbortedTransactions(
    const std::vector<AbortedTransaction> &aborted_txns) {
  writeVarInt(aborted_txns.size() + 1);
  for (const auto &txn : aborted_txns) {
    writeInt64(txn.producer_id).writeInt64(txn.first_offset);
  }
  return *this;
}

FetchResponse &FetchResponse::writeRecordBatches(
    const std::vector<RecordBatchReader::RecordBatch> &record_batches) {
  // Calculate total size upfront
    size_t total_size = 0;
    for (const auto &batch : record_batches) {
        total_size += batch.raw_data.size();
    }

    // Write the total size once we know it
    writeVarInt(static_cast<int64_t>(total_size));

    // Write all batches sequentially
    for (const auto &batch : record_batches) {
        writeBytes(batch.raw_data.data(), batch.raw_data.size());
    }

    writeInt8(0); // terminator
    return *this;
}

FetchResponse &FetchResponse::writePartitionData(
    int32_t partition_index, int16_t error_code, int64_t high_watermark,
    int64_t last_stable_offset, int64_t log_start_offset,
    const std::vector<AbortedTransaction> &aborted_txns,
    int32_t preferred_read_replica,
    const std::vector<RecordBatchReader::RecordBatch> &record_batches) {

  writePartitionHeader(partition_index, error_code, high_watermark,
                       last_stable_offset, log_start_offset,
                       preferred_read_replica)
      .writeAbortedTransactions(aborted_txns)
      .writeRecordBatches(record_batches)
      .writeInt8(0); // TAG_BUFFER
  return *this;
}

FetchResponse &FetchResponse::complete() {
  writeInt8(0); // Final TAG_BUFFER
  updateMessageSize();
  return *this;
}
