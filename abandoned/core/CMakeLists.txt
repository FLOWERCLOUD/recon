project(ReconCore)

set(headers include/Recon/Camera.h
            include/Recon/FileSet.h
            include/Recon/ImageSet.h
            include/Recon/SFMContext.h)
set(sources src/Camera.cpp
            src/FileSet.cpp
            src/ImageSet.cpp
            src/SFMContext.cpp)

add_library(ReconCore ${sources} ${headers})

target_link_libraries(ReconCore
  PUBLIC
  vectormath
  Qt5::Core
  Qt5::Gui
)

target_compile_options(ReconCore
  PUBLIC
  "-std=c++11"
  "-Wall"
)

target_compile_definitions(ReconCore
  PUBLIC
)

target_include_directories(ReconCore
  PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
  PRIVATE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/Recon>
)
