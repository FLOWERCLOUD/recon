project(vectormath_aos_utils_test)

file(GLOB sources *.cpp)

add_executable(${PROJECT_NAME} ${sources})

target_link_libraries(${PROJECT_NAME} vectormath testutils)

target_compile_options(${PROJECT_NAME}
  PRIVATE
  #$<$<NOT:$<CONFIG:Debug>>:-O2>
)

target_compile_definitions(${PROJECT_NAME}
  PRIVATE
  -DVECTORMATH_DEBUG
)

add_test(NAME ${PROJECT_NAME}
         COMMAND ${PROJECT_NAME})
