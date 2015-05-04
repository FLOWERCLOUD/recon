project(ReconSrc)

set(headers OpenGLWindow.h)
set(sources helloworld.cpp
            OpenGLWindow.cpp)

add_executable(HelloWorld ${sources} ${headers})
target_link_libraries(HelloWorld
  PUBLIC
  Recon
  Qt5::Core
  Qt5::Gui
)
