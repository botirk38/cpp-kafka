# Protocol base (headers only)
add_library(kafka_protocol_base INTERFACE)
target_include_directories(kafka_protocol_base INTERFACE
  ${CMAKE_CURRENT_SOURCE_DIR}/base/include
)
kafka_enable_warnings(kafka_protocol_base)

# Storage must be built before protocol responses (responses depend on storage types)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../storage ${CMAKE_BINARY_DIR}/storage)

# Parser module
add_subdirectory(parser)

# API modules
add_subdirectory(api_versions)
add_subdirectory(describe_topic_partitions)
add_subdirectory(fetch)

# Combined protocol target (base + all modules)
add_library(kafka_protocol INTERFACE)
target_link_libraries(kafka_protocol INTERFACE
  kafka_protocol_base
  kafka_protocol_parser
  kafka_protocol_api_versions
  kafka_protocol_describe_topic_partitions
  kafka_protocol_fetch
)
