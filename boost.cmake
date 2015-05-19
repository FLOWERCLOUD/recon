# Import Boost Library

if(WIN32)
  set(BOOST_ROOT
    "C:\\Boost\\1.58.0\\msvc12"
    CACHE PATH "Boost Root Path"
  )
endif()

set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_LIBS ON)
find_package(Boost 1.55 REQUIRED
  COMPONENTS filesystem program_options system thread
)

if(Boost_FOUND)
  add_library(Boost INTERFACE)
  target_include_directories(Boost INTERFACE
    ${Boost_INCLUDE_DIRS}
  )
  target_link_libraries(Boost INTERFACE
    ${Boost_LIBRARIES}
  )
endif()
