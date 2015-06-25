#include <vectormath.h>
#include <vectormath/aos/utils/ray3.h>
#include "Camera.h"
#include "morton_code.h"
#include "GraphCut.h"
#include "VisualHull.h"
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

struct SampleWindow {
  int32_t valid;
  uint8_t red[121];
  uint8_t green[121];
  uint8_t blue[121];

  SampleWindow(const QImage& image, vec3 xy)
  : valid(false)
  {
    int width = image.width(), height = image.height();
    int px = (float)xy.x(), py = (float)xy.y();
    int px0 = px - 5, px1 = px + 5, py0 = py - 5, py1 = py + 5;

    if (px0 >= 0 && py0 >= 0 && px1 < width && py1 < height) {
      this->valid = true;
      for (int i = 0; i < 11; ++i) {
        for (int j = 0; j < 11; ++j) {
          QRgb c = image.pixel(px0 + j, py0 + i);
          this->red[i*11+j] = qRed(c);
          this->green[i*11+j] = qGreen(c);
          this->blue[i*11+j] = qBlue(c);
        }
      }
    }
  }
};

static const mat3 RGB_TO_YUV = mat3{
  vec3{ 0.299f, -0.147f, 0.615f },
  vec3{ 0.587f, -0.289f, -0.515f },
  vec3{ 0.114f, 0.436f, -0.100f }
};

struct NormalizedCrossCorrelation {
  float y, u, v, w;

  NormalizedCrossCorrelation(const SampleWindow& wi, const SampleWindow& wj)
  : y(0.0f), u(0.0f), v(0.0f), w(0.0f)
  {
    if (wi.valid && wj.valid)
      compute(wi, wj);
  }

  void compute(const SampleWindow& wi, const SampleWindow& wj)
  {
    vec3 avg1 = vec3::zero(), avg2 = vec3::zero();
    for (int i = 0; i < 121; ++i)
      avg1 = avg1 + vec3(wi.red[i], wi.green[i], wi.blue[i]) / 121.0f;
    for (int i = 0; i < 121; ++i)
      avg2 = avg2 + vec3(wj.red[i], wj.green[i], wj.blue[i]) / 121.0f;

    vec3 s1 = vec3::zero(), s2 = vec3::zero();
    for (int i = 0; i < 121; ++i) {
      vec3 v1 = vec3(wi.red[i], wi.green[i], wi.blue[i]) - avg1;
      v1 = RGB_TO_YUV * v1;
      s1 = s1 + square(v1) / 121.0f;
    }
    for (int i = 0; i < 121; ++i) {
      vec3 v2 = vec3(wj.red[i], wj.green[i], wj.blue[i]) - avg2;
      v2 = RGB_TO_YUV * v2;
      s2 = s2 + square(v2) / 121.0f;
    }
    s1 = rsqrt(s1);
    s2 = rsqrt(s2);

    vec3 ncc = vec3::zero();
    for (int i = 0; i < 121; ++i) {
      vec3 v1 = vec3(wi.red[i], wi.green[i], wi.blue[i]) - avg1;
      vec3 v2 = vec3(wj.red[i], wj.green[i], wj.blue[i]) - avg2;
      v1 = RGB_TO_YUV * v1;
      v2 = RGB_TO_YUV * v2;
      ncc = ncc + mul(mul(v1, s1), mul(v2, s2)) / 121.0f;
    }

    this->y = (float)ncc.x();
    this->u = (float)ncc.y();
    this->v = (float)ncc.z();
    this->w = 0.5f * this->y + 0.25f * this->u + 0.25f * this->v;
  }
};

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

  float depth_step(int cam_i, ray3 o)
  {
    Camera cam = cameras[cam_i];
    QImage img = image(cam_i);
    mat4 txfm = cam.intrinsicForImage(img.width(), img.height());
    txfm = txfm * cam.extrinsic();

    vec3 xy0 = proj_vec3(transform(txfm, o[0.0f]));
    vec3 xy1 = proj_vec3(transform(txfm, o[1.0f]));
    float dx = float(xy1.x() - xy0.x());
    float dy = float(xy1.y() - xy0.y());
    // Digital Differential Analysis
    if (fabsf(dx) >= fabsf(dy)) {
      return 1.0f / fabsf(dx);
    } else {
      return 1.0f / fabsf(dy);
    }
  }

  float vote(int cam_i, point3 pos)
  {
    ray3 o = ray3::make(pos, cameras[cam_i].center());
    float h = voxel_size;

    SampleWindow wi = sample(cam_i, pos);

    const int max_nk = 128;
    float sjdk[max_nk], dk[max_nk];
    int nk = 0;

    for (int cam_j : closest_cameras(cam_i)) {
      float dstep = depth_step(cam_j, o);
      bool first = true;
      float last_derivative = 1.0f;
      float last_s = 0.0f;
      for (float d = 0.0f; d <= h; d += dstep) {
        float s = NormalizedCrossCorrelation(wi, sample(cam_j, o[d])).w;
        if (first) {
          first = false;
        } else {
          float diff = (s - last_s);
          if ((diff < 0.0f && last_derivative > 0.0f) && nk < max_nk) {
            // this is local maxima
            sjdk[nk] = last_s;
            dk[nk] = d - dstep;
            nk++;
          }
          last_derivative = diff;
        }
        last_s = s;
      }
    }

    if (nk == 0)
      return 0.0f;

    // compute c0
    float c0 = 0.0f;
    for (int k = 0; k < nk; ++k) {
      c0 += sjdk[k] * gaussian(dk[k]);
    }

    // check if c0 is global maximum
    bool result = true;
    for (int i = 0; i < nk; ++i) {
      float d = dk[i], c = 0.0f;
      for (int k = 0; k < nk; ++k)
        c += sjdk[k] * gaussian(d - dk[k]);
      if (c0 < c) {
        result = false;
        break;
      }
    }

    return (result ? c0 : 0.0f);
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
  float param_lambda = 100.0f; // TODO: 0.5f

  // Setup Graph - Visual Hull
  {
    QList<uint64_t> voxels = visual_hull(model, cameras);
    float weight = param_lambda * voxel_h * voxel_h * voxel_h;
    printf("Wb = %f\n", weight);

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
        // midpoint of [x,y,z] and [x-1,y,z]
        //printf("m=%lld [%d %d %d] [%d %d %d]\n", m, x, y, z, x-1, y, z);
        float w = weight * pc.compute(copy_x(center, minpos));
        printf("W([%d,%d,%d] [%d,%d,%d]) = %f\n", x, y, z, x-1, y, z, w);
        graph.set_neighbor_cap(node,-1, 0, 0, w);
      }
      if (y > 0) {
        // midpoint of [x,y,z] and [x,y-1,z]
        //printf("m=%lld [%d %d %d] [%d %d %d]\n", m, x, y, z, x, y-1, z);
        float w = weight * pc.compute(copy_y(center, minpos));
        printf("W([%d,%d,%d] [%d,%d,%d]) = %f\n", x, y, z, x, y-1, z, w);
        graph.set_neighbor_cap(node, 0,-1, 0, w);
      }
      if (z > 0) {
        // midpoint of [x,y,z] and [x,y,z-1]
        //printf("m=%lld [%d %d %d] [%d %d %d]\n", m, x, y, z, x, y, z-1);
        float w = weight * pc.compute(copy_z(center, minpos));
        printf("W([%d,%d,%d] [%d,%d,%d]) = %f\n", x, y, z, x, y, z-1, w);
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

QList<uint32_t> photoconsist_test(const VoxelModel& model, const QList<Camera>& cameras)
{

}

}
