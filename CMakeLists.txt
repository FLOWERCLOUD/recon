cmake_minimum_required(VERSION 3.0)
project(ReconWorkspace)

include(qt5.cmake)
#include(boost.cmake)

add_subdirectory(gfxmath/simdmath)
add_subdirectory(gfxmath/vectormath)
#add_subdirectory(abandoned/core)
#add_subdirectory(abandoned/app)
add_subdirectory(voxel)
