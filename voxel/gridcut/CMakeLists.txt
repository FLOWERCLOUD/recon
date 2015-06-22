projecT(gridcut)

add_library(gridcut INTERFACE)

target_include_directories(gridcut
  INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)
