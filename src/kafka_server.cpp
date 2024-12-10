#include "include/kafka_server.hpp"
#include "include/kafka_response.hpp"
#include <cstring>
#include <iostream>
#include <system_error>
#include <unistd.h>

KafkaServer::KafkaServer(uint16_t port) : port(port) {
  server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd < 0) {
    throw std::system_error(errno, std::generic_category(),
                            "Failed to create socket");
  }

  int reuse = 1;
  if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) <
      0) {
    close(server_fd);
    throw std::system_error(errno, std::generic_category(),
                            "Failed to set socket options");
  }

  server_addr = {};
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(port);
}

KafkaServer::~KafkaServer() {
  if (server_fd >= 0) {
    close(server_fd);
  }
}

void KafkaServer::start() {
  if (bind(server_fd, reinterpret_cast<struct sockaddr *>(&server_addr),
           sizeof(server_addr)) != 0) {
    throw std::system_error(errno, std::generic_category(), "Failed to bind");
  }

  if (listen(server_fd, 5) != 0) {
    throw std::system_error(errno, std::generic_category(), "Failed to listen");
  }

  std::cout << "Server listening on port " << port << std::endl;

  while (true) {
    struct sockaddr_in client_addr{};
    socklen_t client_addr_len = sizeof(client_addr);

    int client_fd =
        accept(server_fd, reinterpret_cast<struct sockaddr *>(&client_addr),
               &client_addr_len);
    if (client_fd < 0) {
      std::cerr << "Accept failed" << std::endl;
      continue;
    }

    handleClient(client_fd);
  }
}

void KafkaServer::handleClient(int client_fd) {
  char buffer[BUFFER_SIZE];
  char response[BUFFER_SIZE];
  int offset = 0;

  recv(client_fd, buffer, sizeof(buffer), 0);

  int32_t correlation_id;
  memcpy(&correlation_id, buffer + 8, sizeof(correlation_id));

  int16_t api_version;
  memcpy(&api_version, buffer + 6, sizeof(api_version));
  api_version = ntohs(api_version);

  ApiVersionResponse::write(response, offset, correlation_id, api_version);

  send(client_fd, response, offset, 0);
  close(client_fd);
}
