project(ReconCore)

set(headers include/Recon/Document.h
            include/Recon/ImageListModel.h
            include/Recon/NVMLoader.h
            include/Recon/SFMWorker.h)
set(sources src/Document.cpp
            src/ImageListModel.cpp
            src/NVMLoader.cpp
            src/SFMWorker.cpp)

add_library(ReconCore ${sources} ${headers})

target_link_libraries(ReconCore
  PUBLIC
  vectormath
  Qt5::Core
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
