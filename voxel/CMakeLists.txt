project(voxel)

set(headers src/CameraData.h
            src/CameraLoader.h
            src/VoxelData.h
            src/VoxelBlock.h
            src/VoxelColoring.h)

set(sources src/CameraLoader.cpp
            src/VoxelBlock.cpp
            src/VoxelColoring.cpp)

add_executable(voxel src/main.cpp ${sources} ${headers})

add_subdirectory(rply)

target_link_libraries(voxel
  PUBLIC
  vectormath
  Qt5::Core
  Qt5::Gui
  rply
)

set_target_properties(voxel PROPERTIES
  AUTOMOC OFF
  AUTOUIC OFF
  AUTORCC OFF
)
