#include <vectormath.h>
#include <vectormath/aos/utils/ray3.h>

#include "Camera.h"
#include "morton_code.h"
#include "BuildGraph.h"
#include "VisualHull.h"
#include "VoxelScore1.h"
//#include "VoxelScore2.h"
#include "PhotoConsistency.h"

#include <QList>
#include <QImage>
#include <QTextStream>
#include <QFile>
#include <QtDebug>
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
  model.virtual_box.minpos.store(graph.voxel_minpos);
  model.virtual_box.maxpos.store(graph.voxel_maxpos);

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

    PhotoConsistency pc(model, cameras);
    for (uint64_t m = 0, n = model.morton_length; m < n; ++m) {
      uint32_t x, y, z;
      morton_decode(m, x, y, z);
      AABox vbox = model.element_box(m);
      Vec3 center = (Vec3)vbox.center();
      Vec3 minpos = (Vec3)vbox.minpos;
      //printf("current voxel = %d %d %d\n", x, y, z);
      //if (__builtin_expect(m % (64) == 0, 0))
      printf("Building Graph: %.2f %%\r", (float)m/(float)n*100.0f);

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

bool load_graph(VoxelGraph& graph, const QString& path)
{
  QFile file(path);
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    qDebug() << "Cannot open graph file!";
    return false;
  }

  QTextStream stream(&file);

  stream >> graph.level
         >> graph.width
         >> graph.voxel_size
         >> graph.voxel_minpos[0]
         >> graph.voxel_minpos[1]
         >> graph.voxel_minpos[2]
         >> graph.voxel_maxpos[0]
         >> graph.voxel_maxpos[1]
         >> graph.voxel_maxpos[2];

  uint64_t length = graph.width * graph.width * graph.width;
  graph.foreground.resize(length, false);
  graph.x_edges.resize(length, 0.0);
  graph.y_edges.resize(length, 0.0);
  graph.z_edges.resize(length, 0.0);

  QString xyz, dir;
  for (uint64_t i = 0; i < length; ++i) {
    int x, y, z;
    int flag;
    stream >> x >> y >> z >> flag;
    graph.foreground[morton_encode(x, y, z)] = flag;
  }

  for (uint64_t i = 0; i < length*3; ++i) {
    int x, y, z;
    double w;
    stream >> x >> y >> z >> w >> dir;
    if (dir == "+x") {
      graph.x_edges[morton_encode(x, y, z)] = w;
    } else if (dir == "+y") {
      graph.y_edges[morton_encode(x, y, z)] = w;
    } else if (dir == "+z") {
      graph.z_edges[morton_encode(x, y, z)] = w;
    }
  }

  return true;
}

bool save_graph(const VoxelGraph& graph, const QString& path)
{
  QFile outfile(path);
  if (!outfile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
    qDebug() << "Cannot open output file: " << path;
    return false;
  }

  QTextStream stream(&outfile);
  stream.setRealNumberNotation(QTextStream::ScientificNotation);
  stream.setRealNumberPrecision(15);

  stream << graph.level << "\n"
         << graph.width << "\n"
         << graph.voxel_size << "\n"
         << graph.voxel_minpos[0] << " "
         << graph.voxel_minpos[1] << " "
         << graph.voxel_minpos[2] << "\n"
         << graph.voxel_maxpos[0] << " "
         << graph.voxel_maxpos[1] << " "
         << graph.voxel_maxpos[2] << "\n";

  uint64_t length = graph.width * graph.width * graph.width;

  for (uint64_t m = 0; m < length; ++m) {
    uint32_t x, y, z;
    morton_decode(m, x, y, z);
    stream << x << " " << y << " " << z << " "
           << graph.foreground[m] << "\n";
  }
  for (uint64_t m = 0; m < length; ++m) {
    uint32_t x, y, z;
    morton_decode(m, x, y, z);
    stream << x << " " << y << " " << z << " "
           << graph.x_edges[m] << " +x\n";
    stream << x << " " << y << " " << z << " "
           << graph.y_edges[m] << " +y\n";
    stream << x << " " << y << " " << z << " "
           << graph.z_edges[m] << " +z\n";
  }

  stream.flush();
  outfile.close();
  return true;
}

}
