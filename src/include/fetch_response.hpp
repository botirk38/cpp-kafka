#pragma once
#include "message_writer.hpp"
#include "record_batch_reader.hpp"
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

  FetchResponse &writeHeader(int32_t correlation_id);
  FetchResponse &writeResponseData(int32_t throttle_time_ms, int16_t error_code,
                                   int32_t session_id, int8_t topic_count);
  FetchResponse &writeTopicHeader(uint128_t topic_id, int8_t partition_count);

  // Partition data writing methods
  FetchResponse &writePartitionData(
      int32_t partition_index, int16_t error_code, int64_t high_watermark,
      int64_t last_stable_offset, int64_t log_start_offset,
      const std::vector<AbortedTransaction> &aborted_txns,
      int32_t preferred_read_replica,
      const std::vector<RecordBatchReader::RecordBatch> &record_batches);

  FetchResponse &writePartitionHeader(int32_t partition_index,
                                      int16_t error_code,
                                      int64_t high_watermark,
                                      int64_t last_stable_offset,
                                      int64_t log_start_offset,
                                      int32_t preferred_read_replica);

  FetchResponse &
  writeAbortedTransactions(const std::vector<AbortedTransaction> &aborted_txns);

  FetchResponse &writeRecordBatches(
      const std::vector<RecordBatchReader::RecordBatch> &record_batches);

  FetchResponse &
  writeRecordBatchHeader(const RecordBatchReader::RecordBatch &batch);
  FetchResponse &writeRecords(const RecordBatchReader::RecordBatch &batch);
  FetchResponse &writeRecord(const RecordReader::Record &record);
  FetchResponse &
  writeRecordHeaders(const std::vector<RecordReader::Header> &headers);

  FetchResponse &complete();
};

