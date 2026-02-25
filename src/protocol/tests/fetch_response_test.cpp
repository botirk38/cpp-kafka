#include "../../storage/include/record_batch_reader.hpp"
#include "../responses/include/fetch_response.hpp"
#include <gtest/gtest.h>

TEST(FetchResponseTest, WritesValidResponse) {
  char buf[1024] = {};
  FetchResponse writer(buf);
  writer.writeHeader(42)
      .writeResponseData(0, 0, 0, 2) // 1 topic
      .writeTopicHeader(1, 2)        // 1 partition
      .writePartitionData(0, 0, 0, 0, 0, std::vector<FetchResponse::AbortedTransaction>{}, 0,
                          std::vector<RecordBatchReader::RecordBatch>{})
      .complete();
  EXPECT_GT(writer.getOffset(), 0);
}

TEST(FetchResponseTest, WritesPartitionWithError) {
  char buf[1024] = {};
  FetchResponse writer(buf);
  writer.writeHeader(1)
      .writeResponseData(0, 0, 0, 2)
      .writeTopicHeader(0, 2)
      .writePartitionData(0, -1, 0, 0, 0, // error_code -1 = UNKNOWN_TOPIC
                          std::vector<FetchResponse::AbortedTransaction>{}, 0,
                          std::vector<RecordBatchReader::RecordBatch>{})
      .complete();
  EXPECT_GT(writer.getOffset(), 0);
}
