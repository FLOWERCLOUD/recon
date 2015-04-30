project(ReconSrc)

add_executable(HelloWorld helloworld.cpp OpenGLWindow.cpp)
target_link_libraries(HelloWorld
  PUBLIC
  Recon
  Qt5::Core
  Qt5::Gui
)
