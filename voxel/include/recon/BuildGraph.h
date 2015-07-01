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

  Automatic Object Segmentation from Calibrated Images
  N.D.F. Cambell, G. Vogiatzis, C. Hernández and R. Cipolla
  CVMP 2011
*/

#include "Camera.h"
#include "VoxelModel.h"
#include <QList>
#include <vector>

namespace recon {

struct VoxelGraph {
  uint32_t level;
  uint32_t width;
  float voxel_size;
  std::vector<bool> foreground;
  std::vector<double> x_edges;
  std::vector<double> y_edges;
  std::vector<double> z_edges;

  inline bool is_foreground(uint64_t m) const;
  inline bool is_foreground(uint32_t x, uint32_t y, uint32_t z) const;
  inline double edge(uint64_t m, int dx, int dy, int dz) const;
  inline double edge(uint32_t x, uint32_t y, uint32_t z, int dx, int dy, int dz) const;

private:
  inline double x_edge(uint64_t m, uint32_t x, uint32_t y, uint32_t z, int d) const;
  inline double y_edge(uint64_t m, uint32_t x, uint32_t y, uint32_t z, int d) const;
  inline double z_edge(uint64_t m, uint32_t x, uint32_t y, uint32_t z, int d) const;

};

void build_graph(VoxelGraph& graph,
                 const VoxelModel& model,
                 const QList<Camera>& cameras);

inline bool
VoxelGraph::is_foreground(uint64_t m) const
{
  return foreground[m];
}

inline bool
VoxelGraph::is_foreground(uint32_t x, uint32_t y, uint32_t z) const
{
  return is_foreground(morton_encode(x, y, z));
}

inline double
VoxelGraph::edge(uint64_t m, int dx, int dy, int dz) const
{
  uint32_t x, y, z;
  morton_decode(m, x, y, z);
  if (dx != 0 && dy == 0 && dz == 0) {
    return x_edge(m, x, y, z, dx);
  } else if (dx == 0 && dy != 0 && dz == 0) {
    return y_edge(m, x, y, z, dy);
  } else if (dx == 0 && dy == 0 && dz != 0) {
    return z_edge(m, x, y, z, dz);
  }
  return 0.0;
}

inline double
VoxelGraph::edge(uint32_t x, uint32_t y, uint32_t z, int dx, int dy, int dz) const
{
  uint64_t m = morton_encode(x,y,z);
  if (dx != 0 && dy == 0 && dz == 0) {
    return x_edge(m, x, y, z, dx);
  } else if (dx == 0 && dy != 0 && dz == 0) {
    return y_edge(m, x, y, z, dy);
  } else if (dx == 0 && dy == 0 && dz != 0) {
    return z_edge(m, x, y, z, dz);
  }
  return 0.0;
}

inline double
VoxelGraph::x_edge(uint64_t m, uint32_t x, uint32_t y, uint32_t z, int d) const
{
  Q_ASSERT(m == morton_encode(x, y, z));
  if (d == 1 && x < width-1) {
    return x_edges[m];
  } else if (d == -1 && x > 0) {
    return x_edges[morton_encode(x-1,y,z)];
  }
  return 0.0;
}

inline double
VoxelGraph::y_edge(uint64_t m, uint32_t x, uint32_t y, uint32_t z, int d) const
{
  Q_ASSERT(m == morton_encode(x, y, z));
  if (d == 1 && y < width-1) {
    return y_edges[m];
  } else if (d == -1 && y > 0) {
    return y_edges[morton_encode(x,y-1,z)];
  }
  return 0.0;
}

inline double
VoxelGraph::z_edge(uint64_t m, uint32_t x, uint32_t y, uint32_t z, int d) const
{
  Q_ASSERT(m == morton_encode(x, y, z));
  if (d == 1 && z < width-1) {
    return z_edges[m];
  } else if (d == -1 && z > 0) {
    return z_edges[morton_encode(x,y,z-1)];
  }
  return 0.0;
}

}
