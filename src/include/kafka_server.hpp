#pragma once
#include <cstdint>
#include <netinet/in.h>

class KafkaServer {
public:
  KafkaServer(uint16_t port = 9092);
  ~KafkaServer();

  void start();

private:
  void handleClient(int client_fd);
  void handleApiVersionsRequest(const char *buffer, char *response,
                                int &offset);

  int server_fd;
  uint16_t port;
  struct sockaddr_in server_addr;
  static constexpr int BUFFER_SIZE = 1024;
};
