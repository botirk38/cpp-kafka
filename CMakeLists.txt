cmake_minimum_required(VERSION 3.28)
project(cpp-kafka LANGUAGES CXX)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})

enable_testing()

include(ProjectOptions)
include(CompilerWarnings)
include(Sanitizers)
include(Coverage)
include(Dependencies)

add_subdirectory(src/common)
add_subdirectory(src/protocol)  # protocol adds storage as subdirectory
add_subdirectory(src/core)

add_executable(kafka src/main.cpp)
target_link_libraries(kafka PRIVATE kafka_core)
kafka_enable_warnings(kafka)
kafka_enable_sanitizers(kafka)
kafka_enable_coverage(kafka)

# Tests
include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG v1.14.0
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)
include(GoogleTest)
add_subdirectory(${CMAKE_SOURCE_DIR}/src/core/tests ${CMAKE_BINARY_DIR}/core-tests)
add_subdirectory(${CMAKE_SOURCE_DIR}/src/protocol/tests ${CMAKE_BINARY_DIR}/protocol-tests)
add_subdirectory(${CMAKE_SOURCE_DIR}/src/storage/tests ${CMAKE_BINARY_DIR}/storage-tests)
