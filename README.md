<p align="center">
    <img src="https://banner2.cleanpng.com/20180714/zb/aav3mwgfp.webp" align="center" width="30%">
</p>
<p align="center"><h1 align="center">CPP-KAFKA</h1></p>
<p align="center">
	<em>A high-performance C++ Kafka client for efficient message streaming.</em>
</p>
<p align="center">
	<img src="https://img.shields.io/github/license/botirk38/cpp-kafka?style=flat-square&logo=opensourceinitiative&logoColor=white&color=00ff1b" alt="license">
	<img src="https://img.shields.io/github/last-commit/botirk38/cpp-kafka?style=flat-square&logo=git&logoColor=white&color=00ff1b" alt="last-commit">
	<img src="https://img.shields.io/github/languages/top/botirk38/cpp-kafka?style=flat-square&color=00ff1b" alt="repo-top-language">
	<img src="https://img.shields.io/github/languages/count/botirk38/cpp-kafka?style=flat-square&color=00ff1b" alt="repo-language-count">
</p>
<p align="center">Built with the tools and technologies:</p>
<p align="center">
	<img src="https://img.shields.io/badge/GNU%20Bash-4EAA25.svg?style=flat-square&logo=GNU-Bash&logoColor=white" alt="GNU%20Bash">
	<img src="https://img.shields.io/badge/CMake-064F8C.svg?style=flat-square&logo=CMake&logoColor=white" alt="CMake">
</p>
<p align="center">
	<img src="https://upload.wikimedia.org/wikipedia/commons/6/64/Apache_Kafka.svg" width="100" alt="Kafka Logo">
</p>

##  Table of Contents

- [Overview](#overview)
- [Features](#features)
- [Project Structure](#project-structure)
- [Getting Started](#getting-started)
  - [Prerequisites](#prerequisites)
  - [Installation](#installation)
  - [Usage](#usage)
  - [Testing](#testing)
- [Project Roadmap](#project-roadmap)
- [Contributing](#contributing)
- [License](#license)
- [Acknowledgments](#acknowledgments)

---

## Overview

CPP-Kafka is a high-performance C++ library for Apache Kafka integration, providing efficient message handling and robust communication capabilities.

---

## Features

- Lightweight and fast Kafka client
- Supports message publishing and subscription
- Thread-safe architecture
- Built using CMake for easy compilation and integration

---

## Project Structure

```sh
cpp-kafka/
├── CMakeLists.txt
├── README.md
├── src/
│   ├── kafka_client.cpp
│   ├── kafka_producer.cpp
│   ├── kafka_consumer.cpp
│   └── include/
│       ├── kafka_client.hpp
│       ├── kafka_producer.hpp
│       ├── kafka_consumer.hpp
├── tests/
└── examples/
```

---

## Getting Started

### Prerequisites

Ensure your system meets the following requirements:

- **C++ Compiler:** GCC/Clang/MSVC
- **CMake** (>=3.10)
- **Apache Kafka**

### Installation

```sh
git clone https://github.com/botirk38/cpp-kafka.git
cd cpp-kafka
cmake . && make
```

### Usage

```sh
./cpp-kafka
```

### Testing

```sh
ctest
```

---

## Project Roadmap

- [X] Initial Kafka Client Implementation
- [ ] Add Advanced Consumer Features
- [ ] Improve Performance Benchmarks

---

## Contributing

1. Fork the repository
2. Create a feature branch
3. Commit your changes
4. Push and create a PR

---

## License

This project is licensed under the MIT License. See [LICENSE](LICENSE) for details.

---

## Acknowledgments

Special thanks to the Apache Kafka community for their work on distributed streaming technologies.
