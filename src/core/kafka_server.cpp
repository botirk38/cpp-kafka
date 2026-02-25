#include "include/kafka_server.hpp"
#include "../protocol/base/include/kafka_request.hpp"
#include "../protocol/responses/include/api_version_response.hpp"
#include "../protocol/responses/include/describe_topics_partitions_response.hpp"
#include "../protocol/responses/include/fetch_response.hpp"
#include "../storage/include/log_metadata_reader.hpp"
#include "../storage/include/record_batch_reader.hpp"
#include "include/kafka_parser.hpp"
#if USE_CPP_MODULES
import kafka_errors;
#else
#include "../common/include/kafka_errors.hpp"
#endif
#include <cstring>
#include <iostream>
#include <ostream>
#include <string>
#include <sys/socket.h>

std::ostream &operator<<(std::ostream &os, const uint128_t &value) {
  return os << std::hex << "0x" << static_cast<uint64_t>(value >> 64)
            << static_cast<uint64_t>(value);
}

KafkaServer::KafkaServer(uint16_t port)
    : port(port), thread_pool(std::thread::hardware_concurrency()) {
  server_socket_ = SocketFd::create();
  server_socket_.setReuseAddr();

  server_addr = {};
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(port);

  registerHandlers();
}

void KafkaServer::registerHandlers() {
  namespace KP = KafkaProtocol;
  apiHandlers[KP::API_VERSIONS] = [this](const KafkaRequestVariant &v, char *response,
                                         int &offset) {
    handleApiVersions(std::get<ApiVersionRequest>(v), response, offset);
  };

  apiHandlers[KP::DESCRIBE_TOPIC_PARTITIONS] = [this](const KafkaRequestVariant &v, char *response,
                                                      int &offset) {
    handleDescribeTopicPartitions(std::get<DescribeTopicsRequest>(v), response, offset);
  };

  apiHandlers[KP::FETCH] = [this](const KafkaRequestVariant &v, char *response, int &offset) {
    handleFetch(std::get<FetchRequest>(v), response, offset);
  };
}

void KafkaServer::start() {
  server_socket_.bind(server_addr);
  server_socket_.listen(5);

  while (true) {
    struct sockaddr_in client_addr{};
    socklen_t client_addr_len = sizeof(client_addr);

    SocketFd client = server_socket_.accept(client_addr, client_addr_len);

    if (!client.valid()) {
      std::cerr << "Accept failed" << std::endl;
      continue;
    }

    int client_fd = client.release();
    thread_pool.enqueue([this, client_fd] { handleClient(client_fd); });
  }
}

void KafkaServer::handleClient(int client_fd) {
  SocketFd client(client_fd);
  std::vector<uint8_t> buffer(BUFFER_SIZE);
  char response[BUFFER_SIZE];

  client.setSendTimeout(5);

  while (true) {
    ssize_t bytes_received = recv(client.get(), buffer.data(), buffer.size(), 0);
    if (bytes_received <= 0) {
      break;
    }

    try {
      int offset = 0;
      auto request = Parser::parse(buffer.data(), bytes_received);
      auto handler = apiHandlers.find(getApiKey(request));

      if (handler != apiHandlers.end()) {
        handler->second(request, response, offset);

        if (send(client.get(), response, offset, 0) < 0) {
          break;
        }
      }
    } catch (const ParseError &e) {
      std::cerr << "Parse error: " << e.what() << std::endl;
      break;
    }
  }
}

void KafkaServer::handleApiVersions(const ApiVersionRequest &request, char *response, int &offset) {
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

void KafkaServer::handleDescribeTopicPartitions(const DescribeTopicsRequest &request,
                                                char *response, int &offset) {
  const auto &header = request.header;

  KafkaLogMetadataReader reader("/tmp/kraft-combined-logs");
  auto cluster_metadata = reader.loadClusterMetadata();

  DescribeTopicPartitionsResponse writer(response);
  writer.writeHeader(header.correlation_id, request.topic_names.size());

  for (const auto &topic_name : request.topic_names) {
    // Find topic by name in cluster metadata
    auto topic_it =
        std::find_if(cluster_metadata.topics_by_id.begin(), cluster_metadata.topics_by_id.end(),
                     [&topic_name](const auto &pair) { return pair.second.name == topic_name; });

    std::optional<KafkaMetadata::TopicMetadata> topic_metadata =
        topic_it != cluster_metadata.topics_by_id.end() ? std::make_optional(topic_it->second)
                                                        : std::nullopt;

    writer.writeTopic(topic_name, topic_metadata);
  }

  writer.complete();
  offset = writer.getOffset();
}

void KafkaServer::handleFetch(const FetchRequest &request, char *response, int &offset) {
  const auto &header = request.header;
  int8_t topics_size = static_cast<int8_t>(request.topics.size());

  FetchResponse writer(response);
  writer.writeHeader(header.correlation_id).writeResponseData(0, 0, 0, topics_size + 1);

  KafkaLogMetadataReader reader("/tmp/kraft-combined-logs");
  auto cluster_metadata = reader.loadClusterMetadata();

  for (const auto &topic : request.topics) {
    int8_t partition_count = topic.partitions.size();

    writer.writeTopicHeader(topic.topic_id, partition_count + 1);

    auto topic_metadata = cluster_metadata.topics_by_id.find(topic.topic_id);
    bool topic_exists = topic_metadata != cluster_metadata.topics_by_id.end();

    for (const auto &partition : topic.partitions) {
      if (!topic_exists) {
        writer.writePartitionData(partition.partition, ERROR_UNKNOWN_TOPIC, 0, 0, 0,
                                  std::vector<FetchResponse::AbortedTransaction>{}, 0,
                                  std::vector<RecordBatchReader::RecordBatch>{});
        continue;
      }

      const auto &partition_metadata = topic_metadata->second.partitions[partition.partition];
      const auto &record_batches = partition_metadata.record_batches;

      writer.writePartitionData(partition.partition, 0, 0, 0, 0,
                                std::vector<FetchResponse::AbortedTransaction>{}, 0,
                                record_batches);
    }
  }

  writer.complete();
  offset = writer.getOffset();
}
