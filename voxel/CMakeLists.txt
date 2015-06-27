project(recon-voxel)

file(GLOB headers src/*.h)
file(GLOB sources src/*.cpp)

add_library(recon-voxel ${sources} ${headers})
add_subdirectory(gridcut)
add_subdirectory(trimesh2)
target_link_libraries(recon-voxel
  PUBLIC
  vectormath
  Qt5::Core
  Qt5::Gui
  PRIVATE
  gridcut
  trimesh2
)
target_compile_options(recon-voxel
  PUBLIC
  $<$<NOT:$<CONFIG:Debug>>:-O2>
)
target_include_directories(recon-voxel
  PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/include/recon
)

add_subdirectory(tools)

# MACOSX_RPATH
# INSTALL_RPATH
