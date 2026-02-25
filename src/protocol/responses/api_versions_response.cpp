#include "include/api_version_response.hpp"
#include "include/describe_topics_partitions_response.hpp"
#include "include/fetch_response.hpp"

using DescribeTopicPartitions = DescribeTopicPartitionsResponse::DescribeTopicPartitions;

ApiVersionResponse &ApiVersionResponse::writeHeader(int32_t correlation_id, int16_t api_version) {
  skipBytes(4) // Message size placeholder
      .writeInt32(correlation_id)
      .writeInt16(api_version >= ApiVersions::MIN_VERSION && api_version <= ApiVersions::MAX_VERSION
                      ? 0
                      : ApiVersions::UNSUPPORTED_VERSION)
      .writeUInt8(4); // num_entries
  return *this;
}

ApiVersionResponse &ApiVersionResponse::writeApiVersionSupport() {
  writeInt16(ApiVersions::KEY)
      .writeInt16(ApiVersions::MIN_VERSION)
      .writeInt16(ApiVersions::MAX_VERSION)
      .writeUInt8(0); // tag_buffer
  return *this;
}

ApiVersionResponse &ApiVersionResponse::writeDescribeTopicsSupport() {
  writeInt16(DescribeTopicPartitions::KEY)
      .writeInt16(0)  // version
      .writeInt16(0)  // version
      .writeUInt8(0); // tag_buffer
  return *this;
}

ApiVersionResponse &ApiVersionResponse::writeFetchSupport() {

  writeInt16(FetchResponse::Fetch::KEY)
      .writeInt16(FetchResponse::Fetch::MIN_VERSION)
      .writeInt16(FetchResponse::Fetch::MAX_VERSION)
      .writeUInt8(0);

  return *this;
};

ApiVersionResponse &ApiVersionResponse::writeMetadata() {
  writeInt32(0)       // throttle_time
      .writeUInt8(0); // tag_buffer
  return *this;
}

ApiVersionResponse &ApiVersionResponse::complete() {
  updateMessageSize();
  return *this;
}
