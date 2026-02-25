# ProjectOptions.cmake - Global project configuration
include_guard(GLOBAL)

# C++26 when compiler supports it; fallback to C++23 (C++26 not yet in stable compilers)
option(USE_CPP26 "Use C++26 standard (requires GCC 14+ / Clang 18+)" OFF)
if(USE_CPP26)
  set(CMAKE_CXX_STANDARD 26)
else()
  set(CMAKE_CXX_STANDARD 23)
endif()
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(ENABLE_COVERAGE "Enable coverage instrumentation" OFF)
option(ENABLE_ASAN "Enable AddressSanitizer" OFF)
option(ENABLE_UBSAN "Enable UndefinedBehaviorSanitizer" OFF)
option(ENABLE_TSAN "Enable ThreadSanitizer" OFF)
