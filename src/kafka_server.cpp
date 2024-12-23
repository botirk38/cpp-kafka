#include "include/kafka_server.hpp"
#include "include/api_version_response.hpp"
#include "include/describe_topics_partitions_response.hpp"
#include "include/fetch_response.hpp"
#include "include/kafka_errors.hpp"
#include "include/kafka_parser.hpp"
#include "include/kafka_request.hpp"
#include "include/log_metadata_reader.hpp"
#include <cstring>
#include <errno.h>
#include <fcntl.h>
#include <iostream>
#include <ostream>
#include <string>
#include <sys/socket.h>
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

  apiHandlers[FetchResponse::Fetch::KEY] = [this](const KafkaRequest &request,
                                                  char *response, int &offset) {
    handleFetch(request, response, offset);
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

  // Set socket to non-blocking mode
  int flags = fcntl(client_fd, F_GETFL, 0);
  fcntl(client_fd, F_SETFL, flags | O_NONBLOCK);

  while (true) {
    ssize_t bytes_received =
        recv(client_fd, buffer.data(), buffer.size(), MSG_DONTWAIT);
    if (bytes_received < 0) {
      if (errno == EAGAIN || errno == EWOULDBLOCK) {
        // No data available, continue polling
        continue;
      }
      break;
    }
    if (bytes_received == 0) {
      break;
    }

    try {
      int offset = 0;
      auto request = Parser::parse(buffer.data(), bytes_received);
      auto handler = apiHandlers.find(request->header.api_key);

      if (handler != apiHandlers.end()) {
        handler->second(*request, response, offset);

        // Send response with timeout
        struct timeval tv;
        tv.tv_sec = 5;
        tv.tv_usec = 0;
        setsockopt(client_fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));

        ssize_t bytes_wrote = send(client_fd, response, offset, 0);
        if (bytes_wrote < 0) {
          break;
        }
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
      .writeFetchSupport()
      .writeMetadata()
      .complete();

  offset = writer.getOffset();
}

void KafkaServer::handleDescribeTopicPartitions(const KafkaRequest &request,
                                                char *response, int &offset) {
  const auto &header = request.header;
  const auto &describe_request =
      dynamic_cast<const DescribeTopicsRequest &>(request);

  KafkaLogMetadataReader reader(
      "/tmp/kraft-combined-logs/__cluster_metadata-0/00000000000000000000.log");

  DescribeTopicPartitionsResponse writer(response);
  writer.writeHeader(header.correlation_id,
                     describe_request.topic_names.size());

  // Process each requested topic
  for (const auto &topic_name : describe_request.topic_names) {
    auto metadata = reader.findTopic(topic_name);
    writer.writeTopic(topic_name, metadata);
  }

  writer.complete();
  offset = writer.getOffset();
}

void KafkaServer::handleFetch(const KafkaRequest &request, char *response,
                              int &offset) {
  const auto &header = request.header;
  const auto &fetch_request = dynamic_cast<const FetchRequest &>(request);

  FetchResponse writer(response);
  writer.writeHeader(header.correlation_id, 0, 0, fetch_request.topics.size());

  // For now, treat all topics as unknown
  for (const auto &topic : fetch_request.topics) {
    for (const auto &partition : topic.partitions) {
      writer.writeTopicResponse(
          topic.topic_id, partition.partition, ERROR_UNKNOWN_TOPIC_OR_PARTITION,
          0,  // high_watermark
          -1, // last_stable_offset
          -1, // log_start_offset
          std::vector<FetchResponse::AbortedTransaction>{}, // empty aborted
                                                            // transactions
          -1,      // preferred_read_replica
          nullptr, // records data
          0,       // records length
          false);
    }
  }

  writer.complete();
  offset = writer.getOffset();
}
