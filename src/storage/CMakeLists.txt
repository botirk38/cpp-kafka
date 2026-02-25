add_library(kafka_storage
  src/io/path_resolver.cpp
  src/metadata/metadata_decoder.cpp
  src/metadata/record_extractor.cpp
  src/metadata/metadata_store.cpp
  src/log/batch_scanner.cpp
  src/log/log_store.cpp
  src/internal/storage_service_impl.cpp
  src/storage_service_factory.cpp
)
target_include_directories(kafka_storage PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(kafka_storage PUBLIC kafka_common)
kafka_enable_warnings(kafka_storage)
kafka_enable_sanitizers(kafka_storage)
kafka_enable_coverage(kafka_storage)
