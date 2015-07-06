#include <vectormath.h>
#include <vectormath/aos/utils/ray3.h>

#include "Camera.h"
#include "morton_code.h"
#include "BuildGraph.h"
#include "VisualHull.h"
#include "VoxelScore1.h"
#include "VoxelScore2.h"
#include "PhotoConsistency.h"

#include <QList>
#include <QImage>
#include <algorithm>
#include <vector>
#include <float.h>
#include <math.h>

namespace recon {

void build_graph(VoxelGraph& graph,
                 const VoxelModel& model,
                 const QList<Camera>& cameras)
{
  float voxel_h = (float)model.virtual_box.extent().x() / model.width;

  graph.level = model.level;
  graph.width = model.width;
  graph.voxel_size = voxel_h;

  // Shape Prior (Visual Hull)
  printf("processing shape prior (visual hull)...\n");
  {
    std::vector<bool>& foreground = graph.foreground;
    foreground.resize(model.morton_length);

    QList<uint64_t> voxels = visual_hull(model, cameras);

    std::fill(foreground.begin(), foreground.end(), false);
    for (uint64_t m : voxels)
      foreground[m] = true;
  }

  // Photo-Consistency
  printf("processing surface prior (photo consistency)...\n");
  {
    std::vector<double>& x_edges = graph.x_edges;
    std::vector<double>& y_edges = graph.y_edges;
    std::vector<double>& z_edges = graph.z_edges;
    x_edges.resize(model.morton_length);
    y_edges.resize(model.morton_length);
    z_edges.resize(model.morton_length);
    std::fill(x_edges.begin(), x_edges.end(), 0.0f);
    std::fill(y_edges.begin(), y_edges.end(), 0.0f);
    std::fill(z_edges.begin(), z_edges.end(), 0.0f);

    PhotoConsistency<VoxelScore2> pc(model, cameras);
    for (uint64_t m = 0, n = model.morton_length; m < n; ++m) {
      uint32_t x, y, z;
      morton_decode(m, x, y, z);
      AABox vbox = model.element_box(m);
      Vec3 center = (Vec3)vbox.center();
      Vec3 minpos = (Vec3)vbox.minpos;
      printf("current voxel = %d %d %d\n", x, y, z);

      if (x > 0) {
        uint64_t m2 = morton_encode(x-1,y,z);
        if (graph.foreground[m] || graph.foreground[m2]) {
          Point3 midpoint = (Point3)copy_x(center, minpos);
          double v = pc.vote(midpoint);
          x_edges[m2] = v;
        }
      }
      if (y > 0) {
        uint64_t m2 = morton_encode(x,y-1,z);
        if (graph.foreground[m] || graph.foreground[m2]) {
          Point3 midpoint = (Point3)copy_y(center, minpos);
          double v = pc.vote(midpoint);
          y_edges[m2] = v;
        }
      }
      if (z > 0) {
        uint64_t m2 = morton_encode(x,y,z-1);
        if (graph.foreground[m] || graph.foreground[m2]) {
          Point3 midpoint = (Point3)copy_z(center, minpos);
          double v = pc.vote(midpoint);
          z_edges[m2] = v;
        }
      }
    }
  }

  //
  printf("finished building graph\n");
}

}
