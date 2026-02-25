add_executable(api_versions_response_tests api_versions_response_test.cpp)
target_link_libraries(api_versions_response_tests PRIVATE GTest::gtest_main kafka_protocol_responses)
target_include_directories(api_versions_response_tests PRIVATE
  ${CMAKE_SOURCE_DIR}/src
  ${CMAKE_SOURCE_DIR}/src/protocol/base/include
  ${CMAKE_SOURCE_DIR}/src/protocol/requests/include
  ${CMAKE_SOURCE_DIR}/src/protocol/responses/include
  ${CMAKE_SOURCE_DIR}/src/storage/include
)
kafka_enable_warnings(api_versions_response_tests)
kafka_enable_sanitizers(api_versions_response_tests)
kafka_enable_coverage(api_versions_response_tests)
gtest_discover_tests(api_versions_response_tests)

add_executable(fetch_response_tests fetch_response_test.cpp)
target_link_libraries(fetch_response_tests PRIVATE GTest::gtest_main kafka_protocol_responses)
target_include_directories(fetch_response_tests PRIVATE
  ${CMAKE_SOURCE_DIR}/src
  ${CMAKE_SOURCE_DIR}/src/protocol/base/include
  ${CMAKE_SOURCE_DIR}/src/protocol/responses/include
  ${CMAKE_SOURCE_DIR}/src/storage/include
)
kafka_enable_warnings(fetch_response_tests)
kafka_enable_sanitizers(fetch_response_tests)
kafka_enable_coverage(fetch_response_tests)
gtest_discover_tests(fetch_response_tests)
