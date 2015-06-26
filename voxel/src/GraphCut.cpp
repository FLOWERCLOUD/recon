#include <vectormath.h>
#include <vectormath/aos/utils/ray3.h>
#include "Camera.h"
#include "morton_code.h"
#include "GraphCut.h"
#include "VisualHull.h"
#include "Correlation.h"
#include <GridCut/GridGraph_3D_6C.h>
#include <QList>
#include <QImage>
#include <algorithm>
#include <vector>
#include <float.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.141592653589793
#endif

#ifndef M_RSQRTPI
#define M_RSQRTPI 0.5641895835477563
#endif

#ifndef M_RSQRT2
#define M_RSQRT2 0.7071067811865476
#endif

namespace recon {

using vectormath::aos::vec3;
using vectormath::aos::vec4;
using vectormath::aos::utils::point3;
using vectormath::aos::utils::ray3;

static float gaussian(float x, float sigma = 1.0f)
{
  float a = 1.0f / sigma * M_RSQRTPI * M_RSQRT2;
  float c = sigma * sigma;
  return expf(-0.5f * (x*x) / c) * a;
}

struct PhotoConsistency {
  AABox model_box;
  float voxel_size;
  QList<Camera> cameras;
  std::vector<QList<int>> closest_camera_lists;
  std::vector<QImage> images;

  PhotoConsistency(const VoxelModel& model, const QList<Camera>& cams)
  : model_box(model.real_box)
  , voxel_size((float)model.virtual_box.extent().x() / model.width)
  , cameras(cams)
  , closest_camera_lists(cams.size())
  , images(cams.size())
  {
  }

  QList<int> closest_cameras(int cam_i)
  {
    if (closest_camera_lists[cam_i].isEmpty()) {
      QList<int> cams;

      int n = cameras.size();
      for (int i = 0; i < n; ++i) {
        if (i == cam_i)
          continue;
        vec3 v0 = cameras[cam_i].center() - model_box.center();
        vec3 v1 = cameras[i].center() - model_box.center();
        if ((float)dot(normalize(v0), normalize(v1)) >= float(M_RSQRT2)) {
          cams.append(i);
        }
      }

      closest_camera_lists[cam_i] = cams;
    }
    return closest_camera_lists[cam_i];
  }

  QImage image(int cam_id)
  {
    if (images[cam_id].isNull()) {
      images[cam_id] = QImage(cameras[cam_id].imagePath());
    }
    return images[cam_id];
  }

  SampleWindow sample(int cam_i, point3 pos)
  {
    Camera cam = cameras[cam_i];
    QImage img = image(cam_i);
    mat4 txfm = cam.intrinsicForImage(img.width(), img.height());
    txfm = txfm * cam.extrinsic();
    return SampleWindow(img, proj_vec3(transform(txfm, pos)));
  }

  float vote(int cam_i, point3 pos)
  {
    ray3 o = ray3::make(pos, cameras[cam_i].center());
    SampleWindow wi = sample(cam_i, pos);

    float c[16];
    for (int k = 0; k < 16; ++k) {
      c[k] = 0.0f;
      float d = (float)k / 16.0f;
      for (int cam_j : closest_cameras(cam_i)) {
        c[k] += NormalizedCrossCorrelation(wi, sample(cam_j, o[d]));
      }
    }

    float c0 = c[0];

    for (int k = 1; k < 16; ++k) {
      if (c0 < c[k])
        return 0.0f;
    }

    return c0;
  }

  float compute(vec3 pos)
  {
    const float param_mju = 0.05f;

    //return 0.0000001f; // TODO: remove it

    float sum = 0.0f;
    for (int i = 0, n = cameras.size(); i < n; ++i) {
      sum += vote(i, (point3)pos);
    }
    //printf("c(%f,%f,%f) = %f\n", (float)pos.x(), (float)pos.y(), (float)pos.z(), sum);
    return expf(-param_mju * sum);
  }

};

VoxelList graph_cut(const VoxelModel& model, const QList<Camera>& cameras)
{
  // Allocate Graph
  using GridGraph = GridGraph_3D_6C<float, float, double>;
  GridGraph graph(model.width, model.height, model.depth);

  //
  float voxel_h = (float)model.virtual_box.extent().x() / model.width;
  float param_lambda = 0.5f; // TODO: 0.5f

  // vhull map
  std::vector<bool> foreground(model.morton_length, false);

  // Setup Graph - Visual Hull
  {
    QList<uint64_t> voxels = visual_hull(model, cameras);
    float weight = param_lambda * voxel_h * voxel_h * voxel_h;
    printf("Wb = %f\n", weight);

    for (uint64_t m : voxels)
      foreground[m] = true;

    uint64_t s = 0, n = voxels.size();
    for (uint64_t i = 0; i < n; ++i) {
      uint32_t x, y, z;
      uint64_t m = voxels[i];
      for (; s < m; ++s) {
        morton_decode(s, x, y, z);
        graph.set_terminal_cap(graph.node_id(x, y, z), 0.0f, INFINITY);
      }

      morton_decode(m, x, y, z);
      graph.set_terminal_cap(graph.node_id(x, y, z), weight, 0.0f);

      s = m + 1;
    }

    for (; s < n; ++s) {
      uint32_t x, y, z;
      morton_decode(s, x, y, z);
      graph.set_terminal_cap(graph.node_id(x, y, z), 0.0f, INFINITY);
    }
  }

  // Setup Graph - Photo Consistency
  {
    PhotoConsistency pc(model, cameras);
    float weight = 4.0f / 3.0f * float(M_PI) * voxel_h * voxel_h;

    for (uint64_t m = 0, n = model.morton_length; m < n; ++m) {
      uint32_t x, y, z;
      morton_decode(m, x, y, z);
      int node = graph.node_id(x, y, z);
      AABox vbox = model.element_box(m);
      vec3 center = (vec3)vbox.center();
      vec3 minpos = (vec3)vbox.minpos;
      //printf("m=%lld\n", m);
      if (x > 0) {
        if (foreground[m] || foreground[morton_encode(x-1,y,z)]) {
          float w = weight * pc.compute(copy_x(center, minpos));
          printf("W([%d,%d,%d] [%d,%d,%d]) = %f\n", x, y, z, x-1, y, z, w);
          graph.set_neighbor_cap(node,-1, 0, 0, w);
        }
      }
      if (y > 0) {
        if (foreground[m] || foreground[morton_encode(x,y-1,z)]) {
          float w = weight * pc.compute(copy_y(center, minpos));
          printf("W([%d,%d,%d] [%d,%d,%d]) = %f\n", x, y, z, x, y-1, z, w);
          graph.set_neighbor_cap(node, 0,-1, 0, w);
        }
      }
      if (z > 0) {
        if (foreground[m] || foreground[morton_encode(x,y,z-1)]) {
          float w = weight * pc.compute(copy_z(center, minpos));
          printf("W([%d,%d,%d] [%d,%d,%d]) = %f\n", x, y, z, x, y, z-1, w);
          graph.set_neighbor_cap(node, 0, 0,-1, w);
        }
      }
    }
  }

  // Optimize
  graph.compute_maxflow();
  printf("flow = %lf\n", graph.get_flow());

  // Get Result
  QList<uint64_t> result;
  int w = model.width, h = model.height, d = model.depth;
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

QList<float> photoconsist_test(const VoxelModel& model, const QList<Camera>& cameras)
{
  QList<float> colors;

  PhotoConsistency pc(model, cameras);
  float voxel_h = pc.voxel_size;
  float weight = 4.0f / 3.0f * float(M_PI) * voxel_h * voxel_h;

  for (uint64_t m = 0, n = model.morton_length; m < n; ++m) {
    uint32_t x, y, z;
    morton_decode(m, x, y, z);
    AABox vbox = model.element_box(m);
    vec3 center = (vec3)vbox.center();
    vec3 minpos = (vec3)vbox.minpos;
    //printf("m=%lld\n", m);
    if (x > 0) {
      // midpoint of [x,y,z] and [x-1,y,z]
      //printf("m=%lld [%d %d %d] [%d %d %d]\n", m, x, y, z, x-1, y, z);
      float w = weight * pc.compute(copy_x(center, minpos));

    }
    if (y > 0) {
      // midpoint of [x,y,z] and [x,y-1,z]
      //printf("m=%lld [%d %d %d] [%d %d %d]\n", m, x, y, z, x, y-1, z);
      float w = weight * pc.compute(copy_y(center, minpos));
    }
    if (z > 0) {
      // midpoint of [x,y,z] and [x,y,z-1]
      //printf("m=%lld [%d %d %d] [%d %d %d]\n", m, x, y, z, x, y, z-1);
      float w = weight * pc.compute(copy_z(center, minpos));
    }
  }

  return colors;
}

}
