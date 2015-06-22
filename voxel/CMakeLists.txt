project(voxel)

file(GLOB headers src/*.h)
file(GLOB sources src/*.cpp)
add_executable(voxel src/main.cpp ${sources} ${headers})

add_subdirectory(gridcut)
add_subdirectory(trimesh2)

target_compile_options(voxel
  PUBLIC
  $<$<NOT:$<CONFIG:Debug>>:-O2>
)

target_include_directories(voxel
  PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/include/recon
)

target_link_libraries(voxel
  PUBLIC
  vectormath
  Qt5::Core
  Qt5::Gui
  trimesh2
  gridcut
)

set_target_properties(voxel PROPERTIES
  AUTOMOC OFF
  AUTOUIC OFF
  AUTORCC OFF
)
# MACOSX_RPATH
# INSTALL_RPATH
