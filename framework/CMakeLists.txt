project(ReconFramework)

set(headers include/Recon/Document.h
            include/Recon/NVMLoader.h
            include/Recon/SFMWorker.h)
set(sources src/Document.cpp
            src/NVMLoader.cpp
            src/SFMWorker.cpp)

add_library(Recon ${sources} ${headers})

target_link_libraries(Recon
  PUBLIC
  vectormath
  Qt5::Core
)

target_compile_options(Recon
  PUBLIC
  "-std=c++11"
  "-Wall"
)

target_include_directories(Recon
  PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
  PRIVATE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/Recon>
)
