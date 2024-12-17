#include "include/describe_topics_partitions_response.hpp"
#include <netinet/in.h>

DescribeTopicPartitionsResponse &
DescribeTopicPartitionsResponse::writeHeader(int32_t correlation_id) {
  skipBytes(4) // Message size placeholder
      .writeInt32(correlation_id)
      .writeInt8(0)   // Tag buffer
      .writeInt32(0); // throttle_time_ms
  return *this;
}

DescribeTopicPartitionsResponse &DescribeTopicPartitionsResponse::writeTopic(
    const std::string &topic_name,
    const std::optional<std::array<uint8_t, 16>> &topic_id) {
  writeInt8(2); // topics array length

  if (topic_id) {
    writeTopicMetadata(topic_name, *topic_id);
  } else {
    writeUnknownTopicError(topic_name);
  }

  return *this;
}

DescribeTopicPartitionsResponse &
DescribeTopicPartitionsResponse::writeTopicMetadata(
    const std::string &topic_name, const std::array<uint8_t, 16> &topic_id) {
  writeInt16(0)                           // error_code
      .writeInt8(topic_name.length() + 1) // Compact string length
      .writeCompactString(topic_name)
      .writeBytes(topic_id.data(), topic_id.size())
      .writeInt8(0)  // is_internal
      .writeInt8(2); // partitions array length (length + 1)

  // Write two partitions
  writePartitionMetadata(0);

  writeInt32(0xdf8)  // topic_authorized_operations
      .writeInt8(0); // Tag buffer
  return *this;
}

DescribeTopicPartitionsResponse &
DescribeTopicPartitionsResponse::writePartitionMetadata(int32_t partition_id) {

  writeInt16(0) // error_code
      .writeInt32(partition_id)
      .writeInt32(1) // leader_id
      .writeInt32(0) // leader_epoch
      .writeInt8(2)  // replica nodes array length (length + 1)
      .writeInt32(1) // replica node
      .writeInt8(2)  // isr nodes array length (length + 1)
      .writeInt32(1) // isr node
      .writeInt8(1)  // eligible leader replicas array length (empty)
      .writeInt8(1)  // last known elr array length (empty)
      .writeInt8(1)  // offline replicas array length (empty)
      .writeInt8(0); // Tag buffer

  return *this;
}

DescribeTopicPartitionsResponse &
DescribeTopicPartitionsResponse::writeUnknownTopicError(
    const std::string &topic_name) {
  writeInt16(ERROR_UNKNOWN_TOPIC)         // error code for unknown topic
      .writeInt8(topic_name.length() + 1) // Compact string length
      .writeCompactString(topic_name)
      .writeBytes(std::array<uint8_t, 16>{}.data(), 16) // Empty UUID
      .writeInt8(0)                                     // is_internal
      .writeInt8(1)  // partitions array length (empty array = 1)
      .writeInt32(0) // topic_authorized_operations
      .writeInt8(0); // Tag buffer

  return *this;
}

DescribeTopicPartitionsResponse &DescribeTopicPartitionsResponse::complete() {

  writeInt8(0xff)    // Next cursor (null)
      .writeInt8(0); // Tag buffer

  updateMessageSize();
  return *this;
}
