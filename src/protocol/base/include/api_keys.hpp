#pragma once

#include <cstdint>

namespace KafkaProtocol {

constexpr int16_t FETCH = 1;
constexpr int16_t API_VERSIONS = 18;
constexpr int16_t DESCRIBE_TOPIC_PARTITIONS = 75;

namespace ApiVersions {
inline constexpr int16_t MIN_VERSION = 0;
inline constexpr int16_t MAX_VERSION = 4;
inline constexpr int16_t UNSUPPORTED_VERSION = 35;
} // namespace ApiVersions

namespace Fetch {
inline constexpr int16_t MIN_VERSION = 0;
inline constexpr int16_t MAX_VERSION = 16;
} // namespace Fetch

namespace DescribeTopicPartitions {
inline constexpr int16_t MIN_VERSION = 0;
inline constexpr int16_t MAX_VERSION = 0;
} // namespace DescribeTopicPartitions

} // namespace KafkaProtocol
