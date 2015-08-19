cmake_minimum_required(VERSION 3.0)
project(simdmath VERSION 0.1.0)

set(sources
  src/c/acosf4.c
  src/c/ceilf4.c
  src/c/copysignf4.c
  src/c/fabsf4.c
  src/c/floorf4.c
  src/c/fmaf4.c
  src/c/fmaxf4.c
  src/c/fminf4.c
  src/c/negatef4.c
  src/c/recipf4.c
  src/c/rsqrtf4.c
  src/c/sincosf4.c
  src/c/sqrtf4.c
  src/c/tanf4.c
  src/c/truncf4.c
)

add_library(simdmath STATIC ${sources})

target_compile_options(simdmath
  PRIVATE
  $<$<NOT:$<CONFIG:Debug>>:-O2>
)

target_include_directories(simdmath
  PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

if((CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64") OR
   (CMAKE_SYSTEM_PROCESSOR STREQUAL "AMD64"))
  target_include_directories(simdmath
    PRIVATE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/sse>
  )
elseif(CMAKE_SYSTEM_PROCESSOR STREQUAL "arm")
  target_include_directories(simdmath
    PRIVATE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/neon>
  )
endif()

set_target_properties(simdmath PROPERTIES
  VERSION ${PROJECT_VERSION}
  SOVERSION ${PROJECT_VERSION}
  AUTOMOC OFF
  AUTOUIC OFF
  AUTORCC OFF
)

install(TARGETS simdmath
  EXPORT simdmath
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
)
install(DIRECTORY include/ DESTINATION include)
install(EXPORT simdmath
  DESTINATION lib/cmake/simdmath
)
export(EXPORT simdmath)
