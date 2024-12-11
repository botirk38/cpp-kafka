#pragma once
#include "message_writer.hpp"
#include <string>

class DescribeTopicsResponse : public MessageWriter<DescribeTopicsResponse> {
public:
  DescribeTopicsResponse(char *buf) : MessageWriter(buf) {}

  DescribeTopicsResponse &writeHeader(int32_t correlation_id);
  DescribeTopicsResponse &writeTopic(const std::string &topic_name);
  DescribeTopicsResponse &complete();

  enum DescribeTopics { KEY = 75, ERROR_UNKNOWN_TOPIC = 3, TAG_BUFFER = 0 };
};

