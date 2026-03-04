#include "include/api_versions_response.hpp"
#include "../../base/include/api_keys.hpp"

ApiVersionsResponse &ApiVersionsResponse::writeHeader(int32_t correlation_id, int16_t api_version) {
  skipBytes(4) // Message size placeholder
      .writeInt32(correlation_id)
      .writeInt16(api_version >= KafkaProtocol::ApiVersions::MIN_VERSION &&
                          api_version <= KafkaProtocol::ApiVersions::MAX_VERSION
                      ? 0
                      : KafkaProtocol::ApiVersions::UNSUPPORTED_VERSION)
      .writeUInt8(4); // num_entries
  return *this;
}

ApiVersionsResponse &ApiVersionsResponse::writeApiVersionSupport() {
  writeInt16(KafkaProtocol::API_VERSIONS)
      .writeInt16(KafkaProtocol::ApiVersions::MIN_VERSION)
      .writeInt16(KafkaProtocol::ApiVersions::MAX_VERSION)
      .writeUInt8(0); // tag_buffer
  return *this;
}

ApiVersionsResponse &ApiVersionsResponse::writeDescribeTopicsSupport() {
  writeInt16(KafkaProtocol::DESCRIBE_TOPIC_PARTITIONS)
      .writeInt16(KafkaProtocol::DescribeTopicPartitions::MIN_VERSION)
      .writeInt16(KafkaProtocol::DescribeTopicPartitions::MAX_VERSION)
      .writeUInt8(0); // tag_buffer
  return *this;
}

ApiVersionsResponse &ApiVersionsResponse::writeFetchSupport() {
  writeInt16(KafkaProtocol::FETCH)
      .writeInt16(KafkaProtocol::Fetch::MIN_VERSION)
      .writeInt16(KafkaProtocol::Fetch::MAX_VERSION)
      .writeUInt8(0);

  return *this;
};

ApiVersionsResponse &ApiVersionsResponse::writeMetadata() {
  writeInt32(0)       // throttle_time
      .writeUInt8(0); // tag_buffer
  return *this;
}

ApiVersionsResponse &ApiVersionsResponse::complete() {
  updateMessageSize();
  return *this;
}
