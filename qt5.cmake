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
elseif(MSVC)
  set(QTDIR "QTDIR-NOTFOUND" CACHE PATH "Where is Qt5 (ex: C:/Qt/5.4/msvc2013_64)")
  if(IS_DIRECTORY ${QTDIR})
    message(STATUS "Use QTDIR=${QTDIR}")
  else()
    message(FATAL_ERROR "QTDIR must be defined")
  endif()
  set(CMAKE_PREFIX_PATH ${QTDIR})
endif()

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC OFF)
set(CMAKE_AUTORCC OFF)

find_package(Qt5 REQUIRED COMPONENTS Core Gui Qml Quick)
