# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build Commands

This project uses CMake for building:

```bash
# Build the project
cmake -B build -S .
cmake --build ./build

# Run the built binary
./build/kafka

# Use the convenience script (builds and runs)
./your_program.sh
```

Note: The project can optionally use vcpkg for dependency management if VCPKG_ROOT is set.

## Project Structure

The codebase follows a modular, layered architecture with clear separation between headers and source files:

```
src/
├── core/                    # Core server components
│   ├── include/*.hpp        # Public headers
│   ├── tests/               # Core module tests
│   └── *.cpp                # Implementation files
├── protocol/                # Protocol layer
│   ├── base/include/        # Base classes (MessageWriter, ByteReader, KafkaRequest)
│   ├── requests/include/    # Request type headers
│   ├── responses/           # Response implementations
│   │   ├── include/*.hpp    # Response headers
│   │   └── *.cpp            # Response implementations
│   └── tests/               # Protocol module tests
├── storage/                 # Storage layer
│   ├── include/*.hpp        # Storage headers
│   ├── tests/               # Storage module tests
│   └── *.cpp                # Storage implementations
├── common/include/          # Common utilities (errors, metadata)
└── main.cpp
```

### Running Tests

```bash
ctest --test-dir build --output-on-failure
```

### Core Components

- **KafkaServer** (`src/core/`): Main server class that listens on port 9092, manages client connections using a thread pool, and routes requests to appropriate handlers
- **KafkaParser** (`src/core/`): Handles parsing of incoming Kafka protocol messages
- **ThreadPool** (`src/core/`): Manages concurrent client connections
- **LogMetadataReader** (`src/storage/`): Reads log metadata and record batches from disk

### Request/Response System

The server implements a handler-based architecture where each Kafka API has dedicated request/response classes:

- **Request Types**: `KafkaRequest` (base), `FetchRequest`, `ApiVersionRequest`, `DescribeTopicsRequest`
- **KafkaRequestVariant**: `std::variant<ApiVersionRequest, DescribeTopicsRequest, FetchRequest>`; parser returns variant, handlers use `std::get<>`
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

- **Language**: C++23 (set in CMakeLists.txt)
- **Dependencies**: spdlog (for logging)
- **Port**: Server runs on port 9092 by default
- **Build System**: CMake with vcpkg toolchain

## Binary Protocol Handling

The project uses template-based CRTP (Curiously Recurring Template Pattern) for efficient binary I/O:

- **MessageWriter<Derived>**: Base template for writing binary protocol messages with network byte order conversion (htonl/htons). Provides fluent interface for chaining write operations
- **ByteReader<Derived>**: Base template for reading binary data from files with network-to-host byte order conversion (ntohl/ntohs). Handles varints and compact encodings

Response classes inherit from MessageWriter:
- `FetchResponse : public MessageWriter<FetchResponse>`
- `ApiVersionsResponse : public MessageWriter<ApiVersionsResponse>`
- `DescribeTopicPartitionsResponse : public MessageWriter<DescribeTopicPartitionsResponse>`

Reader classes inherit from ByteReader:
- `LogMetadataReader : public ByteReader<LogMetadataReader>`
- `RecordBatchReader : public ByteReader<RecordBatchReader>`

## Development Notes

- Each package (core, protocol/*, storage, common) has its own `include/` directory for headers
- Header files use relative paths to reference dependencies in other packages
- The project follows a layered architecture: Core → Protocol → Storage, with Common used by all
- Uses modern C++ features (C++23 standard)
- Implements proper error handling with exceptions
- Thread-safe design using a thread pool for client handling
- Binary data is always converted between network byte order (big-endian) and host byte order

### Adding New Components

When adding new code:
- Place headers in the appropriate `include/` directory within the package
- Place implementation files in the package root directory
- Use relative paths for cross-package includes (e.g., `../../common/include/kafka_errors.hpp`)
- CMake automatically discovers new `.cpp` and `.hpp` files in the standard locations