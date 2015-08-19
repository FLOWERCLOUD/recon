project(vectormath_soa_test)

file(GLOB sources *.cpp)

add_executable(${PROJECT_NAME} ${sources})

target_link_libraries(${PROJECT_NAME} vectormath testutils)

target_compile_options(${PROJECT_NAME}
  PRIVATE
  #$<$<NOT:$<CONFIG:Debug>>:-O2>
)

add_test(NAME ${PROJECT_NAME}
         COMMAND ${PROJECT_NAME})
