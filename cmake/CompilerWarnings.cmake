# CompilerWarnings.cmake - Strict warning flags
include_guard(GLOBAL)

function(kafka_enable_warnings target)
  get_target_property(_type ${target} TYPE)
  if(_type STREQUAL "INTERFACE_LIBRARY")
    target_compile_options(${target} INTERFACE
      $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall -Wextra -Wpedantic>
      $<$<CXX_COMPILER_ID:MSVC>:/W4>
    )
  else()
    target_compile_options(${target} PRIVATE
      $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall -Wextra -Wpedantic>
      $<$<CXX_COMPILER_ID:MSVC>:/W4>
    )
  endif()
endfunction()
