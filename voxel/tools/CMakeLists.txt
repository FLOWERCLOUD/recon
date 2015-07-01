project(recon-voxel-tools)

add_executable(dump-data dump-data.cpp)
target_link_libraries(dump-data recon-voxel)

add_executable(build-graph build-graph.cpp)
target_link_libraries(build-graph recon-voxel)

add_executable(optimize-graph optimize.cpp)
target_link_libraries(optimize-graph recon-voxel)
