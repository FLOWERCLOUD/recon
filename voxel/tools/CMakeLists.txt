project(recon-voxel-tools)

add_executable(voxel main.cpp)
target_link_libraries(voxel recon-voxel)

#add_executable(ncc-curve ncc-curve.cpp)
#target_link_libraries(ncc-curve recon-voxel)

#add_executable(ncc-map ncc-map.cpp)
#target_link_libraries(ncc-map recon-voxel)

add_executable(dump-data dump-data.cpp)
target_link_libraries(dump-data recon-voxel)

add_executable(build-graph build-graph.cpp)
target_link_libraries(build-graph recon-voxel)
