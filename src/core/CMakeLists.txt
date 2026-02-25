add_library(kafka_core
  kafka_server.cpp
  kafka_parser.cpp
  thread_pool.cpp
)
target_include_directories(kafka_core PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_link_libraries(kafka_core PUBLIC
  kafka_protocol
  kafka_storage
  kafka_common
)
if(ENABLE_CPP_MODULES)
  target_compile_definitions(kafka_core PRIVATE USE_CPP_MODULES=1)
endif()
if(spdlog_FOUND)
  target_link_libraries(kafka_core PRIVATE spdlog::spdlog)
endif()
target_link_libraries(kafka_core PRIVATE $<$<BOOL:${MINGW}>:ws2_32>)
kafka_enable_warnings(kafka_core)
kafka_enable_sanitizers(kafka_core)
kafka_enable_coverage(kafka_core)
