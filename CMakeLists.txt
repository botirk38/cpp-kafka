cmake_minimum_required(VERSION 3.13)

project(codecrafters-kafka)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Configure CRC32C options before fetching
set(CRC32C_BUILD_TESTS
    OFF
    CACHE BOOL "" FORCE)
set(CRC32C_BUILD_BENCHMARKS
    OFF
    CACHE BOOL "" FORCE)
set(CRC32C_USE_GLOG
    OFF
    CACHE BOOL "" FORCE)
set(CRC32C_BUILD_SHARED
    OFF
    CACHE BOOL "" FORCE)

include(FetchContent)
FetchContent_Declare(
  crc32c
  GIT_REPOSITORY https://github.com/google/crc32c.git
  GIT_TAG 1.1.2)
FetchContent_MakeAvailable(crc32c)

file(GLOB_RECURSE SOURCE_FILES src/*.cpp src/*.hpp)

add_executable(kafka ${SOURCE_FILES})
target_link_libraries(kafka PRIVATE crc32c)
