cmake_minimum_required(VERSION 3.0)
project(ReconWorkspace)

include(qt5.cmake)

add_subdirectory(gfxmath/simdmath)
add_subdirectory(gfxmath/vectormath)
add_subdirectory(framework)

add_executable(HelloWorld helloworld.cpp)
target_link_libraries(HelloWorld
  PUBLIC
  ReconFramework
  Qt5::Core
  Qt5::Gui
)
