add_executable(metadata_decoder_tests metadata_decoder_test.cpp)
target_link_libraries(metadata_decoder_tests PRIVATE GTest::gtest_main kafka_storage)
target_include_directories(metadata_decoder_tests PRIVATE
  ${CMAKE_SOURCE_DIR}/src
  ${CMAKE_SOURCE_DIR}/src/storage/include
  ${CMAKE_SOURCE_DIR}/src/protocol/base/include
)
kafka_enable_warnings(metadata_decoder_tests)
kafka_enable_sanitizers(metadata_decoder_tests)
kafka_enable_coverage(metadata_decoder_tests)
gtest_discover_tests(metadata_decoder_tests)
