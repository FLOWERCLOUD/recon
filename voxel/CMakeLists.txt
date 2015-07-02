project(recon-voxel)

file(GLOB headers src/*.h)
file(GLOB sources src/*.cpp)

add_library(recon-voxel ${sources} ${headers})
add_subdirectory(trimesh2)
target_link_libraries(recon-voxel
  PUBLIC
  vectormath
  Qt5::Core
  Qt5::Gui
  PRIVATE
  trimesh2
)
target_compile_options(recon-voxel
  PUBLIC
  -msse4.1
  -mavx
  $<$<NOT:$<CONFIG:Debug>>:-Ofast>
  $<$<CONFIG:Debug>:-Og -g>
)
target_include_directories(recon-voxel
  PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/include/recon
  ${CMAKE_CURRENT_SOURCE_DIR}/src/3rdparty
)

add_subdirectory(tools)

# MACOSX_RPATH
# INSTALL_RPATH
