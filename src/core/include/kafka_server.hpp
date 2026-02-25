#pragma once

#include "../../protocol/base/include/api_keys.hpp"
#include "../../protocol/requests/include/kafka_request_variant.hpp"
#include "socket_fd.hpp"
#include "thread_pool.hpp"
#include <cstdint>
#include <functional>
#include <map>
#include <netinet/in.h>

class KafkaServer {
public:
  explicit KafkaServer(uint16_t port = 9092);
  ~KafkaServer() = default;

  void start();

private:
  static constexpr size_t BUFFER_SIZE = 4096;

  using RequestHandler = std::function<void(const KafkaRequestVariant &, char *, int &)>;

  void handleClient(int client_fd);
  void registerHandlers();

  void handleApiVersions(const ApiVersionRequest &request, char *response, int &offset);
  void handleDescribeTopicPartitions(const DescribeTopicsRequest &request, char *response,
                                     int &offset);
  void handleFetch(const FetchRequest &request, char *response, int &offset);

  uint16_t port = 9092;
  SocketFd server_socket_;
  struct sockaddr_in server_addr;
  ThreadPool thread_pool;
  std::map<int16_t, RequestHandler> apiHandlers;
};
