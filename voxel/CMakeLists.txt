project(voxel)

file(GLOB headers src/*.h)
file(GLOB sources src/*.cpp)
add_executable(voxel src/main.cpp ${sources} ${headers})

add_subdirectory(trimesh2)

target_link_libraries(voxel
  PUBLIC
  vectormath
  Qt5::Core
  Qt5::Gui
  trimesh2
)

set_target_properties(voxel PROPERTIES
  AUTOMOC OFF
  AUTOUIC OFF
  AUTORCC OFF
)
