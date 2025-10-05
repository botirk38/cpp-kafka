#include "core/include/kafka_server.hpp"
#include <iostream>

int main() {
  try {
    KafkaServer server;
    server.start();
  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  }
  return 0;
}

