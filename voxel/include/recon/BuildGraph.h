#pragma once

/* MAJOR

  Multi-view Stereo via Volumetric Graph-cuts and Occlusion Robust Photo-Consistency
  George Vogiatzis, Carlos H. Esteban, Philip H. S. Torr, Roberto Cipolla
  PAMI 2007

  Multi-view Stereo via Volumetric Graph-cuts
  George Vogiatzis, P.H.S. Torr, R. Cipolla
  CVPR 2005

*/

/* MINOR

  Shape from Photographs: A Multi-view Stereo Pipeline
  C. Hernández and G. Vogiatzis
  Computer Vision: Detection, Recognition and Reconstruction
  2010 Springer-Verlag

  Automatic 3D object segmentation in multiple views using volumetric graph-cuts
  N.D.F. Campbell, G. Vogiatzis, C. Hernandez and R. Cipolla
  Image and Vision Computing 2010

*/

#include "Camera.h"
#include "morton_code.h"
#include "VoxelModel.h"
#include <QList>
#include <vector>

namespace recon {

struct VoxelGraph {
  uint32_t level;
  uint32_t width;
  float voxel_size;
  float voxel_minpos[3];
  float voxel_maxpos[3];

  // Every array in Morton order
  std::vector<bool> foreground;
  std::vector<double> x_edges;
  std::vector<double> y_edges;
  std::vector<double> z_edges;
  // x_edges[morton(x,y,z)] => (x, y, z) <--> (x+1, y, z)
  // y_edges[morton(x,y,z)] => (x, y, z) <--> (x, y+1, z)
  // z_edges[morton(x,y,z)] => (x, y, z) <--> (x, y, z+1)
};

void build_graph(VoxelGraph& graph,
                 const VoxelModel& model,
                 const QList<Camera>& cameras);

}
