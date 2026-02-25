# Protocol base (headers only)
add_library(kafka_protocol_base INTERFACE)
target_include_directories(kafka_protocol_base INTERFACE
  ${CMAKE_CURRENT_SOURCE_DIR}/base/include
)
target_link_libraries(kafka_protocol_base INTERFACE kafka_common)
kafka_enable_warnings(kafka_protocol_base)

# Storage must be built before protocol responses (responses depend on RecordBatchReader)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../storage ${CMAKE_BINARY_DIR}/storage)

# Protocol responses (has .cpp files)
add_library(kafka_protocol_responses
  responses/api_versions_response.cpp
  responses/describe_topics_response.cpp
  responses/fetch_response.cpp
)
target_include_directories(kafka_protocol_responses PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/base/include
  ${CMAKE_CURRENT_SOURCE_DIR}/requests/include
  ${CMAKE_CURRENT_SOURCE_DIR}/responses/include
)
target_link_libraries(kafka_protocol_responses PUBLIC
  kafka_protocol_base
  kafka_storage
  kafka_common
)
kafka_enable_warnings(kafka_protocol_responses)
kafka_enable_sanitizers(kafka_protocol_responses)
kafka_enable_coverage(kafka_protocol_responses)

# Combined protocol target (base + requests headers + responses)
add_library(kafka_protocol INTERFACE)
target_link_libraries(kafka_protocol INTERFACE
  kafka_protocol_base
  kafka_protocol_responses
)
target_include_directories(kafka_protocol INTERFACE
  ${CMAKE_CURRENT_SOURCE_DIR}/requests/include
)
