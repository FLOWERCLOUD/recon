cmake_minimum_required(VERSION 3.0)
project(ReconWorkspace)

if(APPLE)
  find_program(Homebrew brew
    HINTS
    /usr/local/bin
    /opt/bin
    /opt/local/bin
  )
  #message(STATUS ${Homebrew})
  if(Homebrew)
    execute_process(COMMAND ${Homebrew} --prefix qt5
      OUTPUT_VARIABLE Qt5_DIR
    )
    string(STRIP ${Qt5_DIR} Qt5_DIR)
    set(Qt5_DIR "${Qt5_DIR}/lib/cmake/Qt5")
    #message(STATUS ${Qt5_DIR})
  endif()
endif()

find_package(Qt5 REQUIRED COMPONENTS Core Widgets)

add_executable(HelloWorld helloworld.cpp)
target_link_libraries(HelloWorld Qt5::Core Qt5::Widgets)
