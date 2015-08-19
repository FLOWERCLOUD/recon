project(vectormath_test)

if(MSVC AND (MSVC_VERSION LESS 1900))
  add_definitions(
    -D_ALLOW_KEYWORD_MACROS=1
    -Dnoexcept=
    -Dalignof=__alignof
  )
endif()

option(GENERATE_ASM "Generate Assembly codes" OFF)
if(GENERATE_ASM)
  add_compile_options(-save-temps -g)
endif()

add_subdirectory(aos)
add_subdirectory(aos_utils)
add_subdirectory(soa)
