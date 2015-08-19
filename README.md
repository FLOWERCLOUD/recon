Volumetric Reconstruction
=========================

Voxel-based 3D Model Reconstruction from Images and Silhouettes

## Dependencies

On OSX/Linux, the programs/libraries below should be installed.

- Qt 5
- cmake

## Build

    mkdir build
    cd build
    cmake ..
    make

## Before Run

    VisualSFM sfm+shared DATA/images DATA/bundle.nvm

and silhouettes should be placed in `DATA/masks`

## Run

    ./build/voxel/tools/build-graph --level 8 DATA/bundle.nvm graph.txt
    ./build/voxel/tools/optimize-graph --lambda 10 --mju 1.0 graph.txt points.ply

Note that you can adjust `--lambda` and `--mju` to obtain better result.
In practice, `--mju` can be a fixed value `1.0`.

## After Run

Open `point.ply` with MeshLab, follow these steps to reconstruct surface.

- Render/"Show Normal/Curvature"
- Filters/"Point Set"/"Compute normals for point sets"
  - Neighbour num: 12 ~ 20
  - Smooth Iteration: 2
- Filters/"Remeshing, Simplification and Reconstruction"/"Surface Reconstruction: Poisson"
  - Octree Depth: 10
  - Octree Divide: 6
  - Samples per Node: 1
  - Surface offsetting: 1

Hence you get a reconstructed model.

