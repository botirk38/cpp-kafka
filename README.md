
# cpp-kafka

[![C++23](https://img.shields.io/badge/C%2B%2B-23-blue.svg)](https://en.cppreference.com/w/cpp/23)
[![CMake](https://img.shields.io/badge/CMake-3.13+-064F8C.svg)](https://cmake.org/)
[![License](https://img.shields.io/badge/License-Apache%202.0-blue.svg)](LICENSE)

A lightweight, high-performance Kafka server implementation in modern C++23.

## Overview

cpp-kafka is a Kafka-compatible server that implements core Kafka protocol operations. Built with modern C++ practices, it provides efficient message handling through a thread-pool architecture and CRTP-based binary protocol serialization.

## Features

- ✅ **Kafka Protocol Support**: API Versions, Describe Topic Partitions, and Fetch operations
- ⚡ **High Performance**: Thread-pool based concurrent client handling
- 🏗️ **Modern C++**: Leverages C++23 features and CRTP patterns
- 📦 **Log-Based Storage**: Efficient record batch reading and storage
- 🔌 **Extensible**: Modular architecture for easy feature additions

## Quick Start

### Prerequisites

- **C++ Compiler**: GCC 14+ or Clang 16+ (with C++23 support)
- **CMake**: Version 3.13 or higher
- **spdlog**: Optional, for logging support

### Installation

```bash
# Clone the repository
git clone https://github.com/botirk38/cpp-kafka.git
cd cpp-kafka

# Build the project
cmake -B build -S .
cmake --build ./build

# Run the server
./build/kafka
```

The server will start listening on port **9092** by default.

### Quick Build Script

```bash
./your_program.sh
```

## Architecture

The project follows a clean, layered architecture:

```
┌─────────────────────────────────────┐
│         Core Layer                  │  Server, Parser, ThreadPool
├─────────────────────────────────────┤
│       Protocol Layer                │  Requests, Responses, Serialization
├─────────────────────────────────────┤
│       Storage Layer                 │  Logs, Metadata, RecordBatches
├─────────────────────────────────────┤
│       Common Layer                  │  Utilities, Errors, Metadata Types
└─────────────────────────────────────┘
```

### Directory Structure

```
src/
├── core/                    # Server and connection handling
│   ├── include/             # Public headers
│   └── *.cpp                # Implementation
├── protocol/                # Kafka protocol implementation
│   ├── base/include/        # Base classes (MessageWriter, ByteReader)
│   ├── requests/include/    # Request types
│   └── responses/           # Response types and implementations
├── storage/                 # Data persistence
│   ├── include/             # Storage headers
│   └── *.cpp                # Storage implementation
└── common/include/          # Shared utilities
```

### Key Components

| Component | Description |
|-----------|-------------|
| **KafkaServer** | TCP server managing client connections on port 9092 |
| **KafkaParser** | Binary protocol parser for Kafka messages |
| **ThreadPool** | Concurrent request handler |
| **MessageWriter/ByteReader** | CRTP-based binary serialization |
| **LogMetadataReader** | Cluster and topic metadata management |

## Supported Operations

| API | Key | Description |
|-----|-----|-------------|
| **ApiVersions** | 18 | Negotiate protocol versions |
| **DescribeTopicPartitions** | 75 | Retrieve topic metadata |
| **Fetch** | 1 | Fetch messages from topics |

## Development

### Building from Source

```bash
# Clean build
rm -rf build
cmake -B build -S .
cmake --build ./build

# Run tests (if available)
./build/kafka
```

### Project Guidelines

- Each module has its own `include/` directory
- Use relative paths for cross-module includes
- Follow C++23 standards and best practices
- All binary protocol data uses network byte order (big-endian)

### Adding New Features

1. Place headers in `<module>/include/`
2. Place implementations in `<module>/`
3. Update CMake if adding new directories
4. Ensure proper error handling with exceptions

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request.

## License

This project is licensed under the Apache License 2.0 - see the [LICENSE](LICENSE) file for details.

## Acknowledgments

Built as part of the CodeCrafters Kafka challenge.
