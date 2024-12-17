# Kafka Protocol Implementation in C++

Welcome to the Kafka Protocol Implementation project. This high-performance implementation focuses on core Kafka protocol features, emphasizing clean code and strict protocol compliance.

## Features

- **API Versions Protocol Support**: Seamlessly handle different API versions.
- **Describe Topics API**: Access full metadata for topics.
- **Topic Partition Management**: Efficiently manage topic partitions.
- **Network Protocol Encoding/Decoding**: Robust handling of network protocols.
- **Error Handling & Response Codes**: Comprehensive error management.

## Getting Started

Follow these steps to set up and run the Kafka broker on your machine.

### Prerequisites

Ensure you have the following installed:

- CMake
- Build-essential tools

### Installation

1. **Install Dependencies**

   Open your terminal and run:

   ```bash
   sudo apt-get install cmake build-essential
   ```

2. **Build the Project**

   Navigate to the project directory and execute:

   ```bash
   mkdir build && cd build
   cmake ..
   make
   ```

### Running the Kafka Broker

Execute the following command to start the broker:

```bash
./your_program.sh
```

## Architecture Overview

### Message Protocol Layer

- **MessageWriter**: A template-based encoder for the wire protocol.
- **ApiVersionResponse**: Manages API version negotiations.
- **DescribeTopicPartitionsResponse**: Handles topic and partition metadata.

### Network Layer

- Manages TCP socket connections.
- Processes event loops efficiently.
- Oversees client connection management.

### Metadata Management

- Tracks topic IDs.
- Manages partition states.
- Coordinates replica sets.

## ByteReader Class

The `ByteReader` class is a utility for reading data from a file stream in a structured manner. It supports reading raw bytes, network order integers, and compact strings, making it essential for handling Kafka protocol data.

### Key Methods

- **readRaw**: Reads raw data into a specified type.
- **readBytes**: Reads a specified number of bytes into a buffer.
- **readNetworkOrder**: Reads integers in network byte order and converts them to host byte order.
- **readCompactString**: Reads a compact string from the stream.

### Usage Example

To use the `ByteReader`, derive a class from it and implement the specific reading logic needed for your application.

```cpp
#include "ByteReader.h"

class MyByteReader : public ByteReader<MyByteReader> {
public:
    MyByteReader(std::ifstream &file) : ByteReader(file) {}

    void readData() {
        // Implement specific reading logic here
    }
};
```

## Development Guide

### Building

Utilize CMake for build management:

```bash
mkdir build && cd build
cmake ..
make
```

### Testing

To submit your implementation, use:

```bash
git commit -am "your changes"
git push origin master
```

## Key Files

- **`src/main.cpp`**: Entry point and broker initialization.
- **`src/kafka_server.cpp`**: Core server implementation.
- **`src/describe_topics_response.cpp`**: Handles topic metadata protocol.
- **`src/api_versions_response.cpp`**: Supports API versioning.

## Protocol Implementation Details

This implementation adheres to the Kafka wire protocol specification, supporting:

- Network byte order encoding.
- Compact string formats.
- Variable length arrays.
- Tagged fields.
- Comprehensive error codes.

## Contributing

We welcome contributions! Please follow these steps:

1. Fork the repository.
2. Create a feature branch.
3. Submit a pull request for review.

## Resources

- [Kafka Protocol Guide](https://kafka.apache.org/protocol)

## License

This project is open-source and available for educational and development purposes.
