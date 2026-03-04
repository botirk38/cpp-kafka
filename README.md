# cpp-kafka

A lightweight, high-performance Kafka server implementation in modern C++26. This project implements core Kafka protocol operations with an efficient thread-pool architecture and CRTP-based binary protocol serialization.

## Status

| Metric | Value |
|--------|-------|
| C++ Standard | C++26 |
| CMake | 4.2.3+ |
| License | Apache 2.0 |

CI Status: Build, Tests, Lint, ASan, and Coverage all pass on Linux (GCC 15) and macOS (GCC 15 via Homebrew).

## Quick Start

### Prerequisites

- GCC 15 or later (C++26 support required)
- CMake 4.2.3 or later
- spdlog (optional, for logging)

### Build and Run

```bash
git clone https://github.com/botirk38/cpp-kafka.git
cd cpp-kafka

cmake -B build -S .
cmake --build ./build

# Run the server (listens on port 9092)
./build/kafka

# Run tests
ctest --test-dir build --output-on-failure
```

Or use the convenience build script:

```bash
./your_program.sh
```

## Features

- Kafka Protocol Support: API Versions, Describe Topic Partitions, Fetch operations
- High Performance: Thread-pool based concurrent client handling
- Modern C++: Full C++26 features and CRTP patterns
- Efficient Storage: Log-based storage with batch reading
- Clean Architecture: Modular design for easy extension

## Architecture

The codebase follows a clean layered architecture with clear separation of concerns.

### Layer Overview

```
Server Layer
  - Network I/O, client connection management
  - KafkaServer, ThreadPool, SocketFD

Protocol Layer
  - Kafka API implementations (API Versions, Describe Topics, Fetch)
  - Request parsing and response generation
  - Binary serialization (MessageWriter, ByteReader)

Storage Layer
  - Topic metadata, partition info
  - Log storage and batch reading
  - IStorageService interface for abstraction

Common
  - Shared utilities and error types
```

### Directory Structure

```
src/
├── server/              Server and networking
│   ├── include/        Public headers
│   ├── tests/          Unit tests
│   └── *.cpp           Implementation
├── protocol/           Kafka protocol layer
│   ├── base/           Common protocol types and serialization
│   ├── parser/         Request parsing
│   ├── api_versions/   API Versions implementation
│   ├── describe_topic_partitions/  Describe Topics implementation
│   ├── fetch/          Fetch implementation
│   └── tests/          Protocol tests
├── storage/            Data persistence
│   ├── include/        Public API
│   ├── io/            Binary I/O operations
│   ├── metadata/       Metadata management
│   ├── log/            Log storage
│   ├── internal/       Implementation details
│   └── tests/          Storage tests
```

## Supported Kafka APIs

| Operation | API Key | Description |
|-----------|---------|-------------|
| API Versions | 18 | Query supported protocol versions |
| Describe Topic Partitions | 75 | Get topic and partition metadata |
| Fetch | 1 | Retrieve messages from partitions |

## Key Components

- **KafkaServer**: TCP listener on port 9092, manages client lifecycle
- **KafkaParser**: Binary protocol message parser
- **ThreadPool**: Concurrent request handling with worker threads
- **MessageWriter / ByteReader**: CRTP-based binary serialization with network byte order conversion
- **IStorageService**: Abstract storage interface for topics, partitions, and messages

## Build Options

```bash
cmake -B build -S . \
  -DENABLE_COVERAGE=ON    # Code coverage instrumentation
  -DENABLE_ASAN=ON        # AddressSanitizer (memory safety)
  -DENABLE_UBSAN=ON       # UndefinedBehaviorSanitizer
  -DCMAKE_BUILD_TYPE=Release
```

## Development

### Running Tests

```bash
cmake -B build -S .
cmake --build ./build
ctest --test-dir build --output-on-failure
```

### Code Standards

- C++26 is required throughout the project
- All binary protocol data uses network byte order (big-endian)
- Each module has its own `include/` directory for public headers
- Use relative paths for cross-module includes
- Proper error handling with exceptions
- All code must pass:
  - clang-format-18 style checks
  - AddressSanitizer checks
  - Code coverage instrumentation

### Adding Features

1. Create headers in `<module>/include/`
2. Implement in `<module>/` directory
3. Update CMakeLists.txt if adding new files
4. Add comprehensive tests
5. Ensure no compiler warnings
6. Run full CI locally before pushing

## CI/CD

All commits trigger GitHub Actions CI:

- Build and test on Ubuntu 22.04 (GCC 15) and macOS (GCC 15 via Homebrew)
- Code formatting check with clang-format-18
- AddressSanitizer to catch memory errors
- Code coverage tracking with lcov

See [.github/workflows/ci.yml](.github/workflows/ci.yml) for details.

## Contributing

See [CONTRIBUTING.md](CONTRIBUTING.md) for development guidelines and how to contribute.

## License

This project is licensed under Apache License 2.0. See [LICENSE](LICENSE) for details.

## Resources

- [Kafka Protocol Documentation](https://kafka.apache.org/protocol.html)
- [C++26 Reference](https://en.cppreference.com/w/cpp)
- [CLAUDE.md](CLAUDE.md) - Developer notes and architecture details
