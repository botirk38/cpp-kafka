#include "include/kafka_server.hpp"
#include "include/api_version_response.hpp"
#include "include/describe_topics_partitions_response.hpp"
#include "include/fetch_response.hpp"
#include "include/kafka_errors.hpp"
#include "include/kafka_parser.hpp"
#include "include/kafka_request.hpp"
#include "include/log_metadata_reader.hpp"
#include "include/record_batch_reader.hpp"
#include <cstring>
#include <errno.h>
#include <fcntl.h>
#include <iostream>
#include <ostream>
#include <string>
#include <sys/socket.h>
#include <system_error>
#include <unistd.h>

std::ostream &operator<<(std::ostream &os, const uint128_t &value) {
  return os << std::hex << "0x" << static_cast<uint64_t>(value >> 64)
            << static_cast<uint64_t>(value);
}

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

  KafkaLogMetadataReader reader("/tmp/kraft-combined-logs");
  auto cluster_metadata = reader.loadClusterMetadata();

  DescribeTopicPartitionsResponse writer(response);
  writer.writeHeader(header.correlation_id,
                     describe_request.topic_names.size());

  for (const auto &topic_name : describe_request.topic_names) {
    // Find topic by name in cluster metadata
    auto topic_it = std::find_if(cluster_metadata.topics_by_id.begin(),
                                 cluster_metadata.topics_by_id.end(),
                                 [&topic_name](const auto &pair) {
                                   return pair.second.name == topic_name;
                                 });

    std::optional<KafkaMetadata::TopicMetadata> topic_metadata =
        topic_it != cluster_metadata.topics_by_id.end()
            ? std::make_optional(topic_it->second)
            : std::nullopt;

    writer.writeTopic(topic_name, topic_metadata);
  }

  writer.complete();
  offset = writer.getOffset();
}

void KafkaServer::handleFetch(const KafkaRequest &request, char *response,
                              int &offset) {
  const auto &header = request.header;
  const auto &fetch_request = dynamic_cast<const FetchRequest &>(request);
  int8_t topics_size = fetch_request.topics.size();

  std::cout << "Processing fetch request with " << (int)topics_size << " topics"
            << std::endl;

  FetchResponse writer(response);
  writer.writeHeader(header.correlation_id)
      .writeResponseData(0, 0, 0, topics_size + 1);

  std::cout << "Loading cluster metadata from /tmp/kraft-combined-logs"
            << std::endl;
  KafkaLogMetadataReader reader("/tmp/kraft-combined-logs");
  auto cluster_metadata = reader.loadClusterMetadata();

  std::cout << "Loaded cluster metadata" << std::endl;

  for (const auto &topic : fetch_request.topics) {
    int8_t partition_count = topic.partitions.size();
    std::cout << "Processing topic ID: " << topic.topic_id << " with "
              << (int)partition_count << " partitions" << std::endl;

    writer.writeTopicHeader(topic.topic_id, partition_count + 1);

    auto topic_metadata = cluster_metadata.topics_by_id.find(topic.topic_id);
    bool topic_exists = topic_metadata != cluster_metadata.topics_by_id.end();

    if (!topic_exists) {
      std::cout << "Topic ID " << topic.topic_id << " not found in metadata"
                << std::endl;
    }

    for (const auto &partition : topic.partitions) {
      std::cout << "Processing partition " << partition.partition << std::endl;

      if (!topic_exists) {
        std::cout << "Writing error response for unknown topic" << std::endl;
        writer.writePartitionData(
            partition.partition, ERROR_UNKNOWN_TOPIC, 0, 0, 0,
            std::vector<FetchResponse::AbortedTransaction>{}, 0,
            std::vector<RecordBatchReader::RecordBatch>{});
        continue;
      }

      const auto &partition_metadata =
          topic_metadata->second.partitions[partition.partition];
      const auto &record_batches = partition_metadata.record_batches;

      writer.writePartitionData(
          partition.partition, 0, 0, 0, 0,
          std::vector<FetchResponse::AbortedTransaction>{}, 1, record_batches);

      if (record_batches.empty()) {
        std::cout << "No record batches found for partition "
                  << partition.partition << std::endl;
      }
    }
  }

  writer.complete();
  offset = writer.getOffset();
  std::cout << "Fetch response completed with offset: " << offset << std::endl;
}
