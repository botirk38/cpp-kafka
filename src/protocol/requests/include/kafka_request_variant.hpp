#pragma once

#include "api_version_request.hpp"
#include "describe_topic_partitions_request.hpp"
#include "fetch_request.hpp"
#include <variant>

using KafkaRequestVariant = std::variant<ApiVersionRequest, DescribeTopicsRequest, FetchRequest>;

inline int16_t getApiKey(const KafkaRequestVariant &v) {
  return std::visit([](const auto &r) { return r.header.api_key; }, v);
}
