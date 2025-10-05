cmake_minimum_required(VERSION 3.13)
project(codecrafters-kafka)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Add spdlog
find_package(spdlog QUIET)
if(NOT spdlog_FOUND)
    message(STATUS "spdlog not found via find_package, using system includes")
    set(SPDLOG_INCLUDE_DIRS "/usr/include")
endif()

# Collect source files from new structure
file(GLOB_RECURSE SOURCE_FILES
    src/main.cpp
    src/core/*.cpp
    src/protocol/**/*.cpp
    src/storage/*.cpp
)

# Collect header files for IDE support
file(GLOB_RECURSE HEADER_FILES
    src/core/include/*.hpp
    src/protocol/**/include/*.hpp
    src/storage/include/*.hpp
    src/common/include/*.hpp
)

add_executable(kafka ${SOURCE_FILES} ${HEADER_FILES})

# Add include directories for each package
target_include_directories(kafka PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src/protocol/base/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src/protocol/requests/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src/protocol/responses/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src/storage/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src/common/include
)

if(spdlog_FOUND)
    target_link_libraries(kafka PRIVATE
        spdlog::spdlog
        $<$<BOOL:${MINGW}>:ws2_32>
    )
else()
    target_link_libraries(kafka PRIVATE
        $<$<BOOL:${MINGW}>:ws2_32>
    )
endif()
