project(recon-voxel-tools)

add_executable(build-graph build-graph.cpp)
target_link_libraries(build-graph recon-voxel)

add_executable(optimize-graph optimize.cpp)
target_link_libraries(optimize-graph recon-voxel)

add_executable(vishull vishull.cpp)
target_link_libraries(vishull recon-voxel)

add_executable(repair-voxel repair.cpp)
target_link_libraries(repair-voxel recon-voxel)

find_package(OpenCV 2.4)
if(OpenCV_FOUND)
  add_executable(proj_test proj_test.cpp)
  target_link_libraries(proj_test recon-voxel ${OpenCV_LIBS})

  add_executable(ncc_test ncc_test.cpp)
  target_link_libraries(ncc_test recon-voxel ${OpenCV_LIBS})

  add_executable(score_test score_test.cpp)
  target_link_libraries(score_test recon-voxel ${OpenCV_LIBS})
  target_include_directories(score_test
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../include/recon
  )

  add_executable(vote_test vote_test.cpp)
  target_link_libraries(vote_test recon-voxel)
  target_include_directories(vote_test
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../include/recon
  )

  add_executable(vote_vis vote_vis.cpp)
  target_link_libraries(vote_vis recon-voxel ${OpenCV_LIBS})
  target_include_directories(vote_vis
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../include/recon
  )
endif()
