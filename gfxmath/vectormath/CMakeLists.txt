cmake_minimum_required(VERSION 3.0)
project(vectormath VERSION 0.1.0)

add_library(vectormath INTERFACE)

target_compile_options(vectormath
  INTERFACE
  "-Wall"
  $<$<CXX_COMPILER_ID:GNU>:-std=c++11>
  $<$<CXX_COMPILER_ID:Clang>:-std=c++11>
  $<$<CXX_COMPILER_ID:AppleClang>:-std=c++11>
)

target_include_directories(vectormath
  INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

target_link_libraries(vectormath INTERFACE simdmath)

#add_library(vectormath_optimized INTERFACE)
#target_link_libraries(vectormath_optimized INTERFACE vectormath)
#target_compile_options(vectormath_optimized
#  INTERFACE
#  $<$<NOT:$<CONFIG:Debug>>:-O2>
#)

install(TARGETS vectormath
  EXPORT vectormath
)
install(DIRECTORY include/ DESTINATION include)
install(EXPORT vectormath
  DESTINATION lib/cmake/vectormath
)
export(EXPORT vectormath)
