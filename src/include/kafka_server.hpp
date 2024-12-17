#pragma once

#include "kafka_parser.hpp"
#include "thread_pool.hpp"
#include <cstdint>
#include <functional>
#include <map>
#include <netinet/in.h>

class KafkaServer {
public:
  explicit KafkaServer(uint16_t port = 9092);
  ~KafkaServer();

  void start();

private:
  static constexpr size_t BUFFER_SIZE = 4096;

  using RequestHandler =
      std::function<void(const KafkaRequest &, char *, int &)>;

  void handleClient(int client_fd);
  void registerHandlers();

  void handleApiVersions(const KafkaRequest &request, char *response,
                         int &offset);
  void handleDescribeTopicPartitions(const KafkaRequest &request,
                                     char *response, int &offset);

  uint16_t port = 9092;
  int server_fd{-1};
  struct sockaddr_in server_addr;
  ThreadPool thread_pool;
  std::map<int16_t, RequestHandler> apiHandlers;
};
