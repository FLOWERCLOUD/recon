project(ReconApp)

set(headers OpenGLWindow.h)
set(sources helloworld.cpp
            OpenGLWindow.cpp)

add_executable(ReconApp ${sources} ${headers})
target_link_libraries(ReconApp
  PUBLIC
  ReconCore
  Qt5::Core
  Qt5::Gui
)
