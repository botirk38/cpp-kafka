add_executable(thread_pool_tests thread_pool_test.cpp)
target_link_libraries(thread_pool_tests PRIVATE GTest::gtest_main kafka_server)
target_include_directories(thread_pool_tests PRIVATE
  ${CMAKE_SOURCE_DIR}/src
  ${CMAKE_SOURCE_DIR}/src/server/include
)
kafka_enable_warnings(thread_pool_tests)
kafka_enable_sanitizers(thread_pool_tests)
kafka_enable_coverage(thread_pool_tests)
gtest_discover_tests(thread_pool_tests)

add_executable(socket_fd_tests socket_fd_test.cpp)
target_link_libraries(socket_fd_tests PRIVATE GTest::gtest_main kafka_server)
target_include_directories(socket_fd_tests PRIVATE
  ${CMAKE_SOURCE_DIR}/src
  ${CMAKE_SOURCE_DIR}/src/server/include
)
kafka_enable_warnings(socket_fd_tests)
kafka_enable_sanitizers(socket_fd_tests)
kafka_enable_coverage(socket_fd_tests)
gtest_discover_tests(socket_fd_tests)
