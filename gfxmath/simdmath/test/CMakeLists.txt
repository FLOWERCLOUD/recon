project(simdmath_test)

file(GLOB sources src/*.cpp)

add_executable(simdmath_test ${sources})

target_link_libraries(simdmath_test simdmath testutils)

add_test(NAME simdmath_test
         COMMAND simdmath_test)
