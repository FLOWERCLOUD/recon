cmake_minimum_required(VERSION 3.0)
project(ReconWorkspace)

include(qt5.cmake)

add_subdirectory(gfxmath/simdmath)
add_subdirectory(gfxmath/vectormath)
add_subdirectory(voxel)

#add_subdirectory(visualsfm)
add_subdirectory(app)
