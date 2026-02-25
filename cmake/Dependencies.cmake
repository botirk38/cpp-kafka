# Dependencies.cmake - Find and configure external dependencies
include_guard(GLOBAL)

find_package(spdlog QUIET)
if(NOT spdlog_FOUND)
  message(STATUS "spdlog not found via find_package, using system includes")
  set(SPDLOG_INCLUDE_DIRS "/usr/include")
endif()
