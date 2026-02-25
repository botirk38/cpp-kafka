if(ENABLE_CPP_MODULES)
  # C++ module for kafka_errors
  add_library(kafka_common STATIC)
  target_sources(kafka_common
    PUBLIC
    FILE_SET CXX_MODULES
    BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}
    FILES modules/kafka_errors.ixx
  )
  target_include_directories(kafka_common PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
else()
  # Header-based kafka_errors
  add_library(kafka_common INTERFACE)
  target_include_directories(kafka_common INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
endif()
kafka_enable_warnings(kafka_common)
