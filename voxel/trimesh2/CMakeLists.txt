project(trimesh2)

file(GLOB headers include/trimesh2/*.h)
file(GLOB sources src/*.cc)

add_library(trimesh2 STATIC ${sources} ${headers})

target_include_directories(trimesh2
  INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  PRIVATE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/trimesh2>
)

set_target_properties(trimesh2 PROPERTIES
  AUTOMOC OFF
  AUTOUIC OFF
  AUTORCC OFF
)
