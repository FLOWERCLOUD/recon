project(voxel)

set(headers src/morton_code.h
            src/CameraData.h
            src/CameraLoader.h
            src/VoxelData.h
            src/VoxelBlock.h
            src/VoxelColoring.h)

set(sources src/morton_code.cpp
            src/CameraLoader.cpp
            src/VoxelBlock.cpp
            src/VoxelColoring.cpp)

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
