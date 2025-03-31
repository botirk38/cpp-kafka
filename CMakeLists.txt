cmake_minimum_required(VERSION 3.13)
project(codecrafters-kafka)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Add spdlog
find_package(spdlog REQUIRED)

file(GLOB_RECURSE SOURCE_FILES src/*.cpp src/*.hpp)
add_executable(kafka ${SOURCE_FILES})
target_link_libraries(kafka PRIVATE
    spdlog::spdlog
    $<$<BOOL:${MINGW}>:ws2_32>
)
