#include "kafka_request.hpp"
#include <optional>
#include <string>
#include <vector>

class DescribeTopicsRequest : public KafkaRequest {
public:
  std::vector<std::string> topic_names;
  int32_t response_partition_limit;
  struct Cursor {
    std::string topic_name;
    int32_t partition_index;
  };
  std::optional<Cursor> cursor;
};
