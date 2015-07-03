project(recon-voxel-tools)

add_executable(dump-data dump-data.cpp)
target_link_libraries(dump-data recon-voxel)

add_executable(build-graph build-graph.cpp)
target_link_libraries(build-graph recon-voxel)

add_executable(optimize-graph optimize.cpp)
target_link_libraries(optimize-graph recon-voxel)

add_executable(vishull vishull.cpp)
target_link_libraries(vishull recon-voxel)

find_package(OpenCV 2.4)
if(OpenCV_FOUND)
  add_executable(proj_test proj_test.cpp)
  target_link_libraries(proj_test recon-voxel ${OpenCV_LIBS})

  add_executable(ncc_test ncc_test.cpp)
  target_link_libraries(ncc_test recon-voxel ${OpenCV_LIBS})
endif()
