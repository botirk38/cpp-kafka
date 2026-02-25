# Coverage.cmake - Code coverage instrumentation
include_guard(GLOBAL)

function(kafka_enable_coverage target)
  if(NOT ENABLE_COVERAGE)
    return()
  endif()
  target_compile_options(${target} PRIVATE
    $<$<CXX_COMPILER_ID:GNU>:--coverage>
    $<$<CXX_COMPILER_ID:Clang>:-fprofile-instr-generate -fcoverage-mapping>
  )
  target_link_options(${target} PRIVATE
    $<$<CXX_COMPILER_ID:GNU>:--coverage>
    $<$<CXX_COMPILER_ID:Clang>:-fprofile-instr-generate -fcoverage-mapping>
  )
endfunction()
