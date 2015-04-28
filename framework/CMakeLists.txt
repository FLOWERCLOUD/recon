project(ReconFramework)

set(headers include/Bundle.h
            include/Camera.h)
set(sources src/Bundle.cpp)

add_library(ReconFramework ${sources} ${headers})

target_link_libraries(ReconFramework
  PUBLIC
  Qt5::Core
)

target_include_directories(ReconFramework
  PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
  PRIVATE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/recon>
)
