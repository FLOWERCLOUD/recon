#include "morton_code.h"
#include "GraphCut.h"
#include <GridCut/GridGraph_3D_6C.h>
#include <QList>
#include <algorithm>
#include <vector>
#include <float.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.141592653589793
#endif

namespace recon {

using vectormath::aos::Vec3;
using vectormath::aos::Vec4;
using vectormath::aos::utils::Point3;

VoxelList graph_cut(const VoxelGraph& vgraph, double lambda, double mju)
{
  // Allocate Graph
  using GridGraph = GridGraph_3D_6C<double, double, double>;
  GridGraph graph(vgraph.width, vgraph.width, vgraph.width);

  const uint64_t length = vgraph.width * vgraph.width * vgraph.width;
  const double voxel_h = vgraph.voxel_size;
  const double wb = lambda * voxel_h * voxel_h * voxel_h;
  const double wn = 4.0 / 3.0 * M_PI * voxel_h * voxel_h;

  // Setup Terminal Edges
  for (uint64_t m = 0; m < length; ++m) {
    uint32_t x, y, z;
    morton_decode(m, x, y, z);
    int node = graph.node_id(x, y, z);

    if (vgraph.foreground[m]) {
      graph.set_terminal_cap(node, wb, 0.0);
    } else {
      graph.set_terminal_cap(node, wb, INFINITY);
    }
  }

  // Setup Neighbour Edges
  for (uint64_t m = 0; m < length; ++m) {
    uint32_t x, y, z;
    morton_decode(m, x, y, z);

    if (x < vgraph.width-1) {
      int n1 = graph.node_id(x,y,z), n2 = graph.node_id(x+1,y,z);
      double w = wn * exp(-mju * vgraph.x_edges[m]);
      graph.set_neighbor_cap(n1,1,0,0, w);
      graph.set_neighbor_cap(n2,-1,0,0, w);
    }
    if (y < vgraph.width-1) {
      int n1 = graph.node_id(x,y,z), n2 = graph.node_id(x,y+1,z);
      double w = wn * exp(-mju * vgraph.y_edges[m]);
      graph.set_neighbor_cap(n1,0,1,0, w);
      graph.set_neighbor_cap(n2,0,-1,0, w);
    }
    if (z < vgraph.width-1) {
      int n1 = graph.node_id(x,y,z), n2 = graph.node_id(x,y,z+1);
      double w = wn * exp(-mju * vgraph.z_edges[m]);
      graph.set_neighbor_cap(n1,0,0,1, w);
      graph.set_neighbor_cap(n2,0,0,-1, w);
    }
  }

  // Maximum Flow
  graph.compute_maxflow();
  printf("flow = %lf\n", graph.get_flow());

  // Export Result
  QList<uint64_t> result;
  int w = vgraph.width, h = vgraph.width, d = vgraph.width;
  for (int x = 0; x < w; ++x) {
    for (int y = 0; y < h; ++y) {
      for (int z = 0; z < d; ++z) {
        bool surface =
          (x == 0 || graph.get_segment(graph.node_id(x-1,y,z)) == 1) ||
          (x == w-1 || graph.get_segment(graph.node_id(x+1,y,z)) == 1) ||
          (y == 0 || graph.get_segment(graph.node_id(x,y-1,z)) == 1) ||
          (y == h-1 || graph.get_segment(graph.node_id(x,y+1,z)) == 1) ||
          (z == 0 || graph.get_segment(graph.node_id(x,y,z-1)) == 1) ||
          (z == d-1 || graph.get_segment(graph.node_id(x,y,z+1)) == 1);

        if (graph.get_segment(graph.node_id(x, y, z)) == 0 && surface) {
          result.append(morton_encode(x, y, z));
        }
      }
    }
  }

  return result;
}

#if 0

QList<QPointF> ncc_curve(const VoxelModel& model,
                         const QList<Camera>& cameras,
                         int voxel_x, int voxel_y, int voxel_z,
                         int cam_i, int cam_j)
{
  QList<QPointF> data;
  PhotoConsistency pc(model, cameras);

  uint64_t morton = morton_encode(voxel_x, voxel_y, voxel_z);
  Point3 vpos = model.element_box(morton).center();
  Point3 ci = cameras[cam_i].center();
  Ray3 o = Ray3(vpos, normalize(ci - vpos) * pc.voxel_size * 4.0f);

  SampleWindow wi = pc.sample(cam_i, vpos);

  data.reserve(129);
  for (int k = -64; k <= 64; ++k) {
    float d = (float) k / 64.0f;
    SampleWindow wj = pc.sample(cam_j, o[d]);
    float ncc = NormalizedCrossCorrelation(wi, wj);
    data.append(QPointF(d, ncc));
  }
  return data;
}

QImage ncc_image(const VoxelModel& model,
                 const QList<Camera>& cameras,
                 int plane_y,
                 int cam_i, int cam_j)
{
  QImage canvas = QImage(model.width, model.depth, QImage::Format_ARGB32);
  PhotoConsistency pc(model, cameras);

  for (int i = 0; i < canvas.height(); ++i) {
    for (int j = 0; j < canvas.width(); ++j) {
      uint64_t morton = morton_encode(j, plane_y, i);
      Point3 pos = model.element_box(morton).center();
      SampleWindow wi = pc.sample(cam_i, pos);
      SampleWindow wj = pc.sample(cam_j, pos);

      float ncc = NormalizedCrossCorrelation(wi, wj);
      ncc = fmaxf(ncc * 0.5f + 0.5f, 0.0f);

      int pix = (int)(fmaxf(ncc, 0.0f) * 255) & 0xFF;
      canvas.setPixel(j,i,qRgb(pix, pix, pix));
    }
  }

  canvas.setText("Y Plane", QString::number(plane_y));
  canvas.setText("Camera i", QString::number(cam_i));
  canvas.setText("Camera j", QString::number(cam_j));

  return canvas.mirrored();
}

#endif

}
