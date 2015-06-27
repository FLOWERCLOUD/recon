project(recon-voxel-tools)

add_executable(voxel main.cpp)
target_link_libraries(voxel recon-voxel)

add_executable(ncc-curve ncc-curve.cpp)
target_link_libraries(ncc-curve recon-voxel)

add_executable(ncc-map ncc-map.cpp)
target_link_libraries(ncc-map recon-voxel)
