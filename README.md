
# cpp-kafka
A C++ implementation of an Apache Kafka client.

## Project Overview
The `cpp-kafka` repository provides a set of libraries and tools for interacting with Kafka clusters, including support for producing and consuming messages, as well as administering topics and brokers.

### Key Features
* Support for Kafka protocol versions 0.8.0 to 2.6.0
* Producer and consumer APIs for sending and receiving messages
* Administration API for creating, deleting, and modifying topics and brokers
* Support for SSL/TLS encryption and authentication

## Getting Started
To get started with the `cpp-kafka` repository, you will need to have the following prerequisites installed:
* C++ compiler (e.g., GCC or Clang)
* CMake build system
* Apache Kafka broker (for testing and deployment)

To install the `cpp-kafka` repository, follow these steps:
1. Clone the repository using Git: `git clone https://github.com/botirk38/cpp-kafka.git`
2. Change into the repository directory: `cd cpp-kafka`
3. Build the repository using CMake: `cmake . && make`

## Usage
The `cpp-kafka` repository provides a set of APIs for interacting with Kafka clusters. Here are some basic examples of how to use the repository:
* Producing a message: `KafkaProducer producer; producer.produce("my_topic", "Hello, World!");`
* Consuming a message: `KafkaConsumer consumer; consumer.consume("my_topic", [](const std::string& message) { std::cout << message << std::endl; });`

For more information on using the `cpp-kafka` repository, please see the [API documentation](https://github.com/botirk38/cpp-kafka/blob/master/docs/api.md).

## Architecture Overview
The `cpp-kafka` repository consists of several main components:
* `kafka_server.cpp`: Handles incoming requests and responses from Kafka brokers
* `kafka_parser.cpp`: Parses incoming requests and responses from Kafka brokers
* `log_metadata_reader.cpp`: Reads log metadata from Kafka brokers
* `thread_pool.cpp`: Provides a thread pool for handling incoming requests and responses

These components work together to provide a robust and efficient C++ implementation of an Apache Kafka client.

## Error Handling
The `cpp-kafka` repository uses a combination of error codes and exceptions to handle errors and exceptions. For more information on error handling, please see the [error handling documentation](https://github.com/botirk38/cpp-kafka/blob/master/docs/error_handling.md).

## Security and Authentication
The `cpp-kafka` repository supports SSL/TLS encryption and authentication. For more information on security and authentication, please see the [security documentation](https://github.com/botirk38/cpp-kafka/blob/master/docs/security.md).

## Contributing
To contribute to the `cpp-kafka` repository, please see the [contributing guidelines](https://github.com/botirk38/cpp-kafka/blob/master/CONTRIBUTING.md).

## License
The `cpp-kafka` repository is licensed under the [Apache License, Version 2.0](https://www.apache.org/licenses/LICENSE-2.0).

## Future Development
The `cpp-kafka` repository is actively maintained and updated. For more information on future development plans and current work in progress, please see the [roadmap](https://github.com/botirk38/cpp-kafka/blob/master/ROADMAP.md).
