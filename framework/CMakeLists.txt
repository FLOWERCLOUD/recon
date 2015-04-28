project(ReconFramework)

set(headers Bundle.h
            Camera.h)
set(sources Bundle.cpp)

add_library(ReconFramework ${sources} ${headers})
target_link_libraries(ReconFramework PUBLIC Qt5::Core)
