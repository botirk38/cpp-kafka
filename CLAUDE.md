# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build Commands

This project uses CMake with vcpkg for dependency management:

```bash
# Build the project
cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake
cmake --build ./build

# Run the built binary
./build/kafka

# Use the convenience script (builds and runs)
./your_program.sh
```

## Architecture Overview

This is a C++ implementation of a Kafka server that handles the Kafka protocol. The project is structured around:

### Core Components

- **KafkaServer** (`kafka_server.cpp/hpp`): Main server class that listens on port 9092, manages client connections using a thread pool, and routes requests to appropriate handlers
- **KafkaParser** (`kafka_parser.cpp/hpp`): Handles parsing of incoming Kafka protocol messages
- **ThreadPool** (`thread_pool.cpp/hpp`): Manages concurrent client connections
- **LogMetadataReader** (`log_metadata_reader.cpp/hpp`): Reads log metadata and record batches from disk

### Request/Response System

The server implements a handler-based architecture where each Kafka API has dedicated request/response classes:

- **Request Types**: `KafkaRequest` (base), `FetchRequest`, `ApiVersionRequest`, `DescribeTopicPartitionsRequest`
- **Response Types**: `FetchResponse`, `ApiVersionsResponse`, `DescribeTopicsResponse`
- **Handler Registration**: API handlers are registered by API key in `KafkaServer::registerHandlers()`

### Supported Kafka APIs

- **API Versions** (key 18): Returns supported API versions
- **Describe Topic Partitions** (key 75): Provides topic metadata
- **Fetch** (key 1): Retrieves messages from topics

### Key Data Structures

- **RequestHeader**: Standard Kafka request header (api_key, api_version, correlation_id, client_id)
- **RecordBatch**: Container for log records with headers and compressed data
- **TopicMetadata**: Contains topic information including partitions and replicas

## Project Configuration

- **Language**: C++23 (set in CMakeLists.txt and codecrafters.yml)
- **Dependencies**: spdlog (for logging)
- **Port**: Server runs on port 9092 by default
- **Build System**: CMake with vcpkg toolchain

## Development Notes

- All header files are in `src/include/`
- The project follows a modular design with separate classes for each protocol component
- Uses modern C++ features (C++23 standard)
- Implements proper error handling with exceptions
- Thread-safe design using a thread pool for client handling