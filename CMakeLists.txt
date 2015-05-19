cmake_minimum_required(VERSION 3.0)
project(ReconWorkspace)

include(qt5.cmake)
#include(boost.cmake)

add_subdirectory(gfxmath/simdmath)
add_subdirectory(gfxmath/vectormath)
#add_subdirectory(core)
#add_subdirectory(app)
add_subdirectory(voxel)
