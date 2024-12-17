#include "include/kafka_server.hpp"
#include "include/api_version_response.hpp"
#include "include/describe_topics_partitions_response.hpp"
#include "include/log_metadata_reader.hpp"
#include <cstring>
#include <iostream>
#include <optional>
#include <ostream>
#include <system_error>
#include <unistd.h>

KafkaServer::KafkaServer(uint16_t port)
    : port(port), thread_pool(std::thread::hardware_concurrency()) {

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

  registerHandlers();
}

void KafkaServer::registerHandlers() {
  apiHandlers[ApiVersionResponse::ApiVersions::KEY] =
      [this](const KafkaRequest &request, char *response, int &offset) {
        handleApiVersions(request, response, offset);
      };

  apiHandlers[DescribeTopicPartitionsResponse::DescribeTopicPartitions::KEY] =
      [this](const KafkaRequest &request, char *response, int &offset) {
        handleDescribeTopicPartitions(request, response, offset);
      };
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

    thread_pool.enqueue([this, client_fd] { handleClient(client_fd); });
  }
}

void KafkaServer::handleClient(int client_fd) {
  std::vector<uint8_t> buffer(BUFFER_SIZE);
  char response[BUFFER_SIZE];

  while (true) {
    ssize_t bytes_received = recv(client_fd, buffer.data(), buffer.size(), 0);
    if (bytes_received <= 0)
      break;

    try {
      int offset = 0;
      auto request = Parser::parse(buffer.data(), bytes_received);

      auto handler = apiHandlers.find(request->header.api_key);
      if (handler != apiHandlers.end()) {
        handler->second(*request, response, offset);

        ssize_t bytes_wrote = send(client_fd, response, offset, 0);
        if (bytes_wrote < 0)
          break;
      }
    } catch (const ParseError &e) {
      std::cerr << "Parse error: " << e.what() << std::endl;
      break;
    }
  }

  close(client_fd);
}

void KafkaServer::handleApiVersions(const KafkaRequest &request, char *response,
                                    int &offset) {
  const auto &header = request.header;

  ApiVersionResponse writer(response);
  writer.writeHeader(header.correlation_id, header.api_version)
      .writeApiVersionSupport()
      .writeDescribeTopicsSupport()
      .writeMetadata()
      .complete();

  offset = writer.getOffset();
}

void KafkaServer::handleDescribeTopicPartitions(const KafkaRequest &request,
                                                char *response, int &offset) {
  const auto &header = request.header;
  const auto &describe_request =
      dynamic_cast<const DescribeTopicsRequest &>(request);

  std::cout << "Topic Name: " << describe_request.topic_names[0] << std::endl;

  KafkaLogMetadataReader reader(
      "/tmp/kraft-combined-logs/__cluster_metadata-0/00000000000000000000.log");
  auto metadata = reader.findTopic(describe_request.topic_names[0]);

  std::string is_meta_data_null = !metadata ? "True" : "False";
  std::cout << "Metadata is null: " << is_meta_data_null << std::endl;

  DescribeTopicPartitionsResponse writer(response);
  writer.writeHeader(header.correlation_id)
      .writeTopic(describe_request.topic_names[0],
                  metadata ? std::make_optional(metadata->topic_id)
                           : std::nullopt)
      .complete();

  offset = writer.getOffset();
}
