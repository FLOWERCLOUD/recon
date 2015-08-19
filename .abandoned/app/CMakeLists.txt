project(ReconApp)

file(GLOB sources src/*.cpp)
file(GLOB resources src/*.qrc)
file(GLOB qmlsources src/*.qml)

add_executable(ReconApp ${sources} ${resources} ${qmlsources})

target_compile_options(ReconApp
  PUBLIC
  -std=c++11
  $<$<NOT:$<CONFIG:Debug>>:-O2>
)

target_link_libraries(ReconApp
  PUBLIC
  Qt5::Core
  Qt5::Gui
  Qt5::Quick
)

set_target_properties(ReconApp PROPERTIES
  AUTOMOC ON
  AUTOUIC OFF
  AUTORCC ON
)

if(APPLE)
  set_target_properties(ReconApp PROPERTIES
    MACOSX_BUNDLE TRUE
    MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/Info.plist
  )
endif()
