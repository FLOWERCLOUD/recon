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

VoxelList graph_cut(const VoxelGraph& vgraph, const SkeletonField& field,
                    double lambda, double mju)
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

    double w = wb / (1.0 + field.field[(uint)m]);

    if (vgraph.foreground[m]) {
      graph.set_terminal_cap(node, w, 0.0);
    } else {
      graph.set_terminal_cap(node, 0.0, INFINITY);
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

}
