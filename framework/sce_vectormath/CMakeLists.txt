project(sce_vectormath)

add_library(vectormath INTERFACE)

target_compile_options(vectormath
  INTERFACE
  "-Wall"
)

target_include_directories(vectormath
  INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)
