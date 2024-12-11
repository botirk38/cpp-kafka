#include "include/describe_topics_response.hpp"
#include <netinet/in.h>

DescribeTopicsResponse &
DescribeTopicsResponse::writeHeader(int32_t correlation_id) {
  skipBytes(4) // Message size placeholder
      .writeInt32(correlation_id)
      .writeInt8(TAG_BUFFER)
      .writeInt32(0); // throttle time ms
  return *this;
}

DescribeTopicsResponse &
DescribeTopicsResponse::writeTopic(const std::string &topic_name) {
  // array_length + 1 as per Rust implementation
  writeInt8(1 + 1) // topics array length + 1
      .writeInt16(ERROR_UNKNOWN_TOPIC);

  // Write topic name with length prefix
  writeInt8(static_cast<int8_t>(topic_name.length() + 1));
  memcpy(buffer + offset, topic_name.c_str(), topic_name.length());
  offset += topic_name.length();

  // Write UUID (16 bytes of zeros)
  for (int i = 0; i < 16; i++) {
    writeUInt8(0);
  }

  writeInt8(0)                // internal topic flag
      .writeInt8(1)           // partitions array
      .writeInt32(0x00000df8) // topic authorized operations
      .writeInt8(TAG_BUFFER); // tag buffer

  return *this;
}

DescribeTopicsResponse &DescribeTopicsResponse::complete() {
  writeUInt8(0xff)            // next cursor (null)
      .writeInt8(TAG_BUFFER); // final tag buffer

  updateMessageSize();
  return *this;
}
