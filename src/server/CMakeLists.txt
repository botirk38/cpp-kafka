add_library(kafka_server
  kafka_server.cpp
  thread_pool.cpp
)
target_include_directories(kafka_server PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_link_libraries(kafka_server PUBLIC
  kafka_protocol
  kafka_storage
)
if(spdlog_FOUND)
  target_link_libraries(kafka_server PRIVATE spdlog::spdlog)
endif()
target_link_libraries(kafka_server PRIVATE $<$<BOOL:${MINGW}>:ws2_32>)
kafka_enable_warnings(kafka_server)
kafka_enable_sanitizers(kafka_server)
kafka_enable_coverage(kafka_server)
