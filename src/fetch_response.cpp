#include "include/fetch_response.hpp"

FetchResponse &FetchResponse::writeHeader(int32_t correlation_id) {
  skipBytes(4) // Message size placeholder
      .writeInt32(correlation_id)
      .writeInt8(0); // TAG_BUFFER
  return *this;
}

FetchResponse &FetchResponse::writeResponseData(int32_t throttle_time_ms,
                                                int16_t error_code,
                                                int32_t session_id,
                                                int8_t topic_count) {
  std::cout << "Topic Count: " << topic_count;
  writeInt32(throttle_time_ms)
      .writeInt16(error_code)
      .writeInt32(session_id)
      .writeVarInt(topic_count);
  return *this;
}

FetchResponse &FetchResponse::writeTopicHeader(uint128_t topic_id,
                                               int8_t partition_count) {
  writeUint128(topic_id).writeVarInt(partition_count);
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

FetchResponse &FetchResponse::writeRecordBatchHeader(
    const RecordBatchReader::RecordBatch &batch) {
  writeInt64(batch.base_offset)
      .writeInt32(batch.batch_length + 1)
      .writeInt32(batch.partition_leader_epoch)
      .writeInt8(batch.magic_byte)
      .writeUInt32(batch.crc)
      .writeInt16(batch.attributes)
      .writeInt32(batch.last_offset_delta)
      .writeInt64(batch.base_timestamp)
      .writeInt64(batch.max_timestamp)
      .writeInt64(batch.producer_id)
      .writeInt16(batch.producer_epoch)
      .writeInt32(batch.base_sequence)
      .writeInt32(batch.records_count);
  return *this;
}

FetchResponse &FetchResponse::writeRecord(const RecordReader::Record &record) {
  writeVarInt(record.length)
      .writeInt8(record.attributes)
      .writeVarInt(record.timestamp_delta)
      .writeVarInt(record.offset_delta)
      .writeVarInt(record.key.size())
      .writeBytes(record.key.data(), record.key.size())
      .writeVarInt(record.value.size())
      .writeBytes(record.value.data(), record.value.size())
      .writeVarInt(record.headers.size());
  return writeRecordHeaders(record.headers);
}

FetchResponse &FetchResponse::writeRecordHeaders(
    const std::vector<RecordReader::Header> &headers) {
  for (const auto &header : headers) {
    writeVarInt(header.key.length())
        .writeBytes(header.key.data(), header.key.length())
        .writeVarInt(header.value.size())
        .writeBytes(header.value.data(), header.value.size());
  }
  return *this;
}

FetchResponse &
FetchResponse::writeRecords(const RecordBatchReader::RecordBatch &batch) {
  for (const auto &record : batch.records) {
    writeRecord(record);
  }
  return *this;
}

FetchResponse &FetchResponse::writeRecordBatches(
    const std::vector<RecordBatchReader::RecordBatch> &record_batches) {
  writeVarInt(record_batches.size() + 1);
  for (const auto &batch : record_batches) {
    writeRecordBatchHeader(batch).writeRecords(batch);
  }
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

