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

  // TODO: epipolar_walk
  // TODO: depth from 2d

  float vote_1(int cam_i, point3 pos)
  {
    point3 ci = cameras[cam_i].center();
    ray3 o = ray3(pos, 0.5f * voxel_size * normalize(ci - pos));
    SampleWindow wi = sample(cam_i, pos);

    float c0 = -1.0f;
    for (int cam_j : closest_cameras(cam_i)) {
      for (int k = -16; k <= 16; ++k) {
        float d = float(k) / 16.0f;
        SampleWindow wj = sample(cam_j, o[d]);
        float s = NormalizedCrossCorrelation(wi, wj);
        c0 = fmaxf(s, c0);
      }
    }
    return c0;
  }

  /*float compute(vec3 pos)
  {
    const float param_mju = 0.05f;

    float sum = 0.0f;
    for (int i = 0, n = cameras.size(); i < n; ++i) {
      sum += vote(i, (point3)pos);
    }
    return expf(-param_mju * sum);
  }*/

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
    //printf("Wb = %f\n", weight);

    for (uint64_t m : voxels)
      foreground[m] = true;

    for (uint64_t m = 0, n = model.morton_length; m < n; ++m) {
      uint32_t x, y, z;
      morton_decode(m, x, y, z);

      if (foreground[m]) {
        graph.set_terminal_cap(graph.node_id(x, y, z), weight, 0.0f);
      } else {
        graph.set_terminal_cap(graph.node_id(x, y, z), weight, INFINITY);
      }
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
      if (x > 0) {
        float w = 0.00001f; // TODO
        //if (foreground[m] || foreground[morton_encode(x-1,y,z)]) {
        //  w = weight * pc.compute(copy_x(center, minpos));
        //} else {
        //  w = INFINITY;
        //}
        graph.set_neighbor_cap(node,-1, 0, 0, w);
      }
      if (y > 0) {
        float w = 0.00001f; // TODO
        //if (foreground[m] || foreground[morton_encode(x,y-1,z)]) {
        //  w = weight * pc.compute(copy_y(center, minpos));
        //} else {
        //  w = INFINITY;
        //}
        graph.set_neighbor_cap(node, 0,-1, 0, w);
      }
      if (z > 0) {
        float w = 0.00001f; // TODO
        //if (foreground[m] || foreground[morton_encode(x,y,z-1)]) {
        //  w = weight * pc.compute(copy_z(center, minpos));
        //} else {
        //  w = INFINITY;
        //}
        graph.set_neighbor_cap(node, 0, 0,-1, w);
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

QList<QPointF> ncc_curve(const VoxelModel& model,
                         const QList<Camera>& cameras,
                         int voxel_x, int voxel_y, int voxel_z,
                         int cam_i, int cam_j)
{
  QList<QPointF> data;
  PhotoConsistency pc(model, cameras);

  uint64_t morton = morton_encode(voxel_x, voxel_y, voxel_z);
  point3 vpos = model.element_box(morton).center();
  point3 ci = cameras[cam_i].center();
  ray3 o = ray3(vpos, normalize(ci - vpos) * pc.voxel_size * 0.5f);

  SampleWindow wi = pc.sample(cam_i, vpos);

  data.reserve(33);
  for (int k = -16; k <= 16; ++k) {
    float d = (float) k / 16.0f;
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
      point3 pos = model.element_box(morton).center();
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

QImage vote_image(const VoxelModel& model,
                  const QList<Camera>& cameras,
                  int plane_y,
                  int cam_i)
{
  QImage canvas = QImage(model.width, model.depth, QImage::Format_ARGB32);
  PhotoConsistency pc(model, cameras);

  for (int i = 0; i < canvas.height(); ++i) {
    for (int j = 0; j < canvas.width(); ++j) {
      uint64_t morton = morton_encode(j, plane_y, i);
      point3 pos = model.element_box(morton).center();

      float v = pc.vote_1(cam_i, pos);

      int pix = (int)(fminf(fmaxf(v, 0.0f), 1.0f) * 255) & 0xFF;
      canvas.setPixel(j,i,qRgb(pix, pix, pix));
    }
  }

  canvas.setText("Y Plane", QString::number(plane_y));
  canvas.setText("Camera i", QString::number(cam_i));

  return canvas.mirrored();
}

}
