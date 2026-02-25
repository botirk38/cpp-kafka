add_executable(parser_tests parser_test.cpp)
target_link_libraries(parser_tests PRIVATE GTest::gtest_main kafka_core)
target_include_directories(parser_tests PRIVATE
  ${CMAKE_SOURCE_DIR}/src
  ${CMAKE_SOURCE_DIR}/src/core/include
  ${CMAKE_SOURCE_DIR}/src/protocol/base/include
  ${CMAKE_SOURCE_DIR}/src/protocol/requests/include
  ${CMAKE_SOURCE_DIR}/src/protocol/responses/include
  ${CMAKE_SOURCE_DIR}/src/storage/include
  ${CMAKE_SOURCE_DIR}/src/common/include
)
kafka_enable_warnings(parser_tests)
kafka_enable_sanitizers(parser_tests)
kafka_enable_coverage(parser_tests)
gtest_discover_tests(parser_tests)

add_executable(thread_pool_tests thread_pool_test.cpp)
target_link_libraries(thread_pool_tests PRIVATE GTest::gtest_main kafka_core)
target_include_directories(thread_pool_tests PRIVATE
  ${CMAKE_SOURCE_DIR}/src
  ${CMAKE_SOURCE_DIR}/src/core/include
)
kafka_enable_warnings(thread_pool_tests)
kafka_enable_sanitizers(thread_pool_tests)
kafka_enable_coverage(thread_pool_tests)
gtest_discover_tests(thread_pool_tests)

add_executable(socket_fd_tests socket_fd_test.cpp)
target_link_libraries(socket_fd_tests PRIVATE GTest::gtest_main kafka_core)
target_include_directories(socket_fd_tests PRIVATE
  ${CMAKE_SOURCE_DIR}/src
  ${CMAKE_SOURCE_DIR}/src/core/include
)
kafka_enable_warnings(socket_fd_tests)
kafka_enable_sanitizers(socket_fd_tests)
kafka_enable_coverage(socket_fd_tests)
gtest_discover_tests(socket_fd_tests)
