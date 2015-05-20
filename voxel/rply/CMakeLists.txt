project(rply)

add_library(rply STATIC rply.c rply.h)

target_include_directories(rply
  INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)

set_target_properties(rply PROPERTIES
  AUTOMOC OFF
  AUTOUIC OFF
  AUTORCC OFF
)
