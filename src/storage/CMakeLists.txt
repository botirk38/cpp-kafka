add_library(kafka_storage
  log_metadata_reader.cpp
  metadata_decoder.cpp
)
target_include_directories(kafka_storage PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_link_libraries(kafka_storage PUBLIC
  kafka_protocol_base
  kafka_common
)
kafka_enable_warnings(kafka_storage)
kafka_enable_sanitizers(kafka_storage)
kafka_enable_coverage(kafka_storage)
