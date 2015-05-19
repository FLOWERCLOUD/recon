project(voxel)

set(headers src/CameraData.h
            src/CameraLoader.h
            src/VoxelData.h
            src/VoxelBlock.h)

set(sources src/CameraLoader.cpp
            src/VoxelBlock.cpp
            src/main.cpp)

add_executable(voxel ${sources} ${headers})
target_link_libraries(voxel
  PUBLIC
  vectormath
  Qt5::Core
  Qt5::Gui
)
