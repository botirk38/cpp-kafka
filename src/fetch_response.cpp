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
  int start_offset = offset;

  writeInt64(batch.base_offset);
  int batch_length_offset = offset;
  writeInt32(0); // BatchLength placeholder
  writeInt32(batch.partition_leader_epoch);
  writeInt8(batch.magic_byte); // Magic value is 2

  int crc_start_offset = offset;
  writeInt32(0); // CRC placeholder
  int crc_end_offset = offset;

  writeInt16(batch.attributes)
      .writeInt32(batch.last_offset_delta)
      .writeInt64(batch.base_timestamp)
      .writeInt64(batch.max_timestamp)
      .writeInt64(batch.producer_id)
      .writeInt16(batch.producer_epoch)
      .writeInt32(batch.base_sequence)
      .writeInt32(static_cast<int32_t>(batch.records_count));

  for (int32_t i = 0; i < batch.records.size(); i++) {
    auto record = batch.records[i];
    record.offset_delta = i; // Set consecutive offset deltas
    writeRecord(record);
  }

  int batch_length = offset - 12 - start_offset;
  uint32_t network_length = htonl(batch_length);
  memcpy(buffer + batch_length_offset, &network_length, sizeof(network_length));

  uint32_t computed_crc =
      crc32c::Crc32c(reinterpret_cast<const uint8_t *>(buffer + crc_end_offset),
                     offset - crc_end_offset);
  uint32_t network_crc = htonl(computed_crc);
  memcpy(buffer + crc_start_offset, &network_crc, sizeof(network_crc));

  return *this;
}

FetchResponse &FetchResponse::writeRecord(const RecordReader::Record &record) {
  // Save the current offset to calculate length later
  int lengthOffset = offset;
  // Skip the length field for now
  writeVarInt(0);

  int recordStart = offset;

  // Write all record fields
  writeInt8(record.attributes)
      .writeVarInt(record.timestamp_delta)
      .writeVarInt(static_cast<int64_t>(record.offset_delta));

  // Handle key
  if (record.key.empty()) {
    writeVarInt(1);
  } else {
    writeVarInt(static_cast<uint64_t>(record.key.size() + 1));
    writeBytes(record.key.data(), record.key.size());
  }

  writeVarInt(static_cast<uint64_t>(record.value.size()))
      .writeBytes(record.value.data(), record.value.size());

  writeVarInt(static_cast<int64_t>(record.headers.size()))
      .writeRecordHeaders(record.headers);

  // Calculate actual record length
  int recordLength = offset - recordStart;

  // Store current offset
  int currentOffset = offset;

  // Go back and write the length
  offset = lengthOffset;
  writeVarInt(static_cast<int64_t>(recordLength));

  // Restore offset
  offset = currentOffset;

  return *this;
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
  writeVarInt(static_cast<int64_t>(record_batches.size() + 1));
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
