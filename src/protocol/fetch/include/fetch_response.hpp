#pragma once
#include "../../base/include/kafka_types.hpp"
#include "../../base/include/message_writer.hpp"
#include <cstdint>
#include <vector>

namespace KafkaProtocol::Fetch {
inline constexpr int16_t ERROR_UNKNOWN_TOPIC_OR_PARTITION = 3;
}

class FetchResponse : public MessageWriter<FetchResponse> {
public:
  struct AbortedTransaction {
    int64_t producer_id;
    int64_t first_offset;
  };

  explicit FetchResponse(char *buf) : MessageWriter(buf) {}

  FetchResponse &writeHeader(int32_t correlation_id);
  FetchResponse &writeResponseData(int32_t throttle_time_ms, int16_t error_code, int32_t session_id,
                                   int64_t topic_count);
  FetchResponse &writeTopicHeader(uint128_t topic_id, int64_t partition_count);

  FetchResponse &writePartitionData(int32_t partition_index, int16_t error_code,
                                    int64_t high_watermark, int64_t last_stable_offset,
                                    int64_t log_start_offset,
                                    const std::vector<AbortedTransaction> &aborted_txns,
                                    int32_t preferred_read_replica,
                                    const RecordBatches &record_batches);

  FetchResponse &writePartitionHeader(int32_t partition_index, int16_t error_code,
                                      int64_t high_watermark, int64_t last_stable_offset,
                                      int64_t log_start_offset, int32_t preferred_read_replica);

  FetchResponse &writeAbortedTransactions(const std::vector<AbortedTransaction> &aborted_txns);

  FetchResponse &writeRecordBatches(const RecordBatches &record_batches);

  FetchResponse &complete();
};
