project(voxel)

set(headers src/CameraData.h
            src/CameraLoader.h
            src/VoxelOctree.h)

set(sources src/CameraLoader.cpp
            src/main.cpp)

add_executable(voxel ${sources} ${headers})
target_link_libraries(voxel vectormath Boost)
