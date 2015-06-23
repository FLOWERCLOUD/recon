#include <vectormath.h>
#include <vectormath/aos/utils/ray3.h>
#include "Camera.h"
#include "morton_code.h"
#include "GraphCut.h"
#include <GridCut/GridGraph_3D_6C.h>
#include <AlphaExpansion/AlphaExpansion_3D_6C.h>
#include <QList>
#include <QImage>
#include <algorithm>
#include <vector>
#include <float.h>

namespace recon {

using vectormath::aos::vec3;
using vectormath::aos::vec4;
using vectormath::aos::utils::point3;
using vectormath::aos::utils::ray3;

struct SampleWindow {
  int32_t valid;
  uint32_t red[121];
  uint32_t green[121];
  uint32_t blue[121];

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
      ncc = ncc + mul(mul(v1, s1), mul(v2, s2));
    }

    this->y = (float)ncc.x();
    this->u = (float)ncc.y();
    this->v = (float)ncc.z();
    this->w = 0.5f * this->y + 0.25f * this->u + 0.25f * this->v;
  }
};

struct GraphCutOptimizer {
  QList<Camera> cameras;
  std::vector<QList<int>> closest_camera_lists;
  std::vector<QImage> images; // cache
  //std::vector<QImage> masks; // cache
  point3 model_center;

  explicit GraphCutOptimizer(const QList<Camera>& cams);

  QImage image(int cam_id);
  //QImage mask(int cam_id);
  void clear_images();
  //void clear_masks();

  QList<int> closest_cameras(int cam_id);
  SampleWindow sample(int cam_id, point3 x);

  float compute_correlation_score(point3 x, int cam_i, float d);
  float vote(point3 x, int cam_id);
};

GraphCutOptimizer::GraphCutOptimizer(const QList<Camera>& cams)
: cameras(cams)
, closest_camera_lists(cams.size())
, images(cams.size())
//, masks(cams.size())
{
}

QImage GraphCutOptimizer::image(int cam_id)
{
  if (images[cam_id].isNull()) {
    images[cam_id] = QImage(cameras[cam_id].imagePath());
  }
  return images[cam_id];
}

/*QImage GraphCutOptimizer::mask(int cam_id)
{
  if (masks[cam_id].isNull()) {
    masks[cam_id] = QImage(cameras[cam_id].maskPath());
  }
  return masks[cam_id];
}*/

void GraphCutOptimizer::clear_images()
{
  std::fill(images.begin(), images.end(), QImage());
}

/*void GraphCutOptimizer::clear_masks()
{
  std::fill(masks.begin(), masks.end(), QImage());
}*/

QList<int> GraphCutOptimizer::closest_cameras(int cam_id)
{
  if (closest_camera_lists[cam_id].isEmpty()) {
    QList<int> cams;

    int n = cameras.size();
    for (int i = 0; i < n; ++i) {
      vec3 v0 = cameras[cam_id].center() - model_center;
      vec3 v1 = cameras[i].center() - model_center;
      if (i != cam_id && (float)dot(normalize(v0), normalize(v1)) >= 0.6f) {
        cams.append(i);
      }
    }

    closest_camera_lists[cam_id] = cams;
  }
  return closest_camera_lists[cam_id];
}

SampleWindow GraphCutOptimizer::sample(int cam_id, point3 x)
{
  QImage img = image(cam_id);
  mat4 mat = cameras[cam_id].intrinsicForImage(img.width(), img.height());
  mat = mat * cameras[cam_id].extrinsic();

  vec3 pos = proj_vec3(mat * vec4(x.data, 1.0f));
  return SampleWindow(img, pos);
}

float GraphCutOptimizer::compute_correlation_score(point3 x, int cam_i, float d)
{
  QList<int> cams = closest_cameras(cam_i);

  ray3 r = ray3{ x, cameras[cam_i].center() - x };
  SampleWindow wi = sample(cam_i, r[d]);

  float ncc_sum = 0.0f;
  for (int j = 0, n = cams.size(); j < n; ++j) {
    SampleWindow wj = sample(cams[j], r[d]);
    ncc_sum += NormalizedCrossCorrelation(wi, wj).w;
  }

  return ncc_sum;
}

float GraphCutOptimizer::vote(point3 x, int cam_id)
{
  const int d_num = 64;
  static const float d[d_num] = {
    0.0, 0.015625, 0.03125, 0.046875, 0.0625, 0.078125, 0.09375, 0.109375,
    0.125, 0.140625, 0.15625, 0.171875, 0.1875, 0.203125, 0.21875, 0.234375,
    0.25, 0.265625, 0.28125, 0.296875, 0.3125, 0.328125, 0.34375, 0.359375,
    0.375, 0.390625, 0.40625, 0.421875, 0.4375, 0.453125, 0.46875, 0.484375,
    0.5, 0.515625, 0.53125, 0.546875, 0.5625, 0.578125, 0.59375, 0.609375,
    0.625, 0.640625, 0.65625, 0.671875, 0.6875, 0.703125, 0.71875, 0.734375,
    0.75, 0.765625, 0.78125, 0.796875, 0.8125, 0.828125, 0.84375, 0.859375,
    0.875, 0.890625, 0.90625, 0.921875, 0.9375, 0.953125, 0.96875, 0.984375
  };

  // compute and combine correlation scores
  float s[d_num];
  for (int i = 0; i < d_num; ++i)
    s[i] = compute_correlation_score(x, cam_id, d[i]);

  // Estimation with Parzen Window
  float c[d_num];
  for (int i = 0; i < d_num; ++i) {
    float sum = 0.0f;
    sum += s[std::max(i-3,0)] * 0.004f;
    sum += s[std::max(i-2,0)] * 0.054f;
    sum += s[std::max(i-1,0)] * 0.242f;
    sum += s[i] * 0.399f;
    sum += s[std::min(i+1,d_num-1)] * 0.242f;
    sum += s[std::min(i+2,d_num-1)] * 0.054f;
    sum += s[std::min(i+3,d_num-1)] * 0.004f;
    c[i] = sum;
  }

  float c0 = c[0];
  if (std::all_of(c, c+d_num, [c0](float x)->bool{ return (c0>x); })) {
    return c0;
  } else {
    return 0.0f;
  }
}

static QList<uint64_t> visual_hull(const VoxelModel& model, const QList<Camera>& cameras)
{
  QList<uint64_t> voxels[2];
  voxels[0].reserve(model.morton_length);

  for (uint64_t m = 0, ms = model.morton_length; m < ms; ++m) {
    voxels[0].append(m);
  }

  int current_voxel_list = 0;
  for (int cam_i = 0, cam_n = cameras.size(); cam_i < cam_n; ++cam_i) {
    Camera cam = cameras[cam_i];
    QImage mask = QImage(cam.maskPath());

    mat4 extrinsic = cam.extrinsic();
    mat4 intrinsic = cam.intrinsicForImage(mask.width(), mask.height());
    mat4 transform = intrinsic * extrinsic;

    QList<uint64_t>& old_voxels = voxels[current_voxel_list];
    QList<uint64_t>& new_voxels = voxels[(current_voxel_list + 1) % 2];
    new_voxels.clear();
    new_voxels.reserve(old_voxels.size());

    for (uint64_t morton : old_voxels) {
      AABox vbox = model.element_box(morton);
      vec3 pos = vbox.center();
      pos = proj_vec3(transform * vec4(pos, 1.0f));

      QPoint pt2d = QPoint((float)pos.x(), (float)pos.y());
      if (mask.valid(pt2d)) {
        if (qGray(mask.pixel(pt2d)) > 100) {
          new_voxels.append(morton);
        }
      }
    }

    current_voxel_list = (current_voxel_list + 1) % 2;
  }

  return voxels[current_voxel_list];
}

VoxelList graph_cut(const VoxelModel& model, const QList<Camera>& cameras)
{
  GraphCutOptimizer optimizer(cameras);

  // Initialize
  optimizer.model_center = (point3)model.real_box.center();

  // Allocate Graph
  using GridGraph = GridGraph_3D_6C<float, float, double>;
  GridGraph graph(model.width, model.height, model.depth);

  // Setup Graph - Visual Hull
  {
    QList<uint64_t> voxels = visual_hull(model, cameras);

    uint64_t s = 0, n = voxels.size();
    for (uint64_t i = 0; i < n; ++i) {
      uint32_t x, y, z;
      uint64_t m = voxels[i];
      for (; s < m; ++s) {
        morton_decode(s, x, y, z);
        graph.set_terminal_cap(graph.node_id(x, y, z), 0, INFINITY);
      }

      morton_decode(m, x, y, z);
      graph.set_terminal_cap(graph.node_id(x, y, z), 1.0f, 0);

      s = m + 1;
    }

    for (; s < n; ++s) {
      uint32_t x, y, z;
      morton_decode(s, x, y, z);
      graph.set_terminal_cap(graph.node_id(x, y, z), 0, INFINITY);
    }
  }

  // Setup Graph - Photo Consistency
  {
    for (uint64_t m = 0, n = model.morton_length; m < n; ++m) {
      uint32_t x, y, z;
      morton_decode(m, x, y, z);
      int node = graph.node_id(x, y, z);
      if (x > 0)
        graph.set_neighbor_cap(node,-1, 0, 0, 0.125f);
      if (y > 0)
        graph.set_neighbor_cap(node, 0,-1, 0, 0.125f);
      if (z > 0)
        graph.set_neighbor_cap(node, 0, 0,-1, 0.125f);
    }
  }
  optimizer.clear_images();

  // Optimize
  graph.compute_maxflow();
  printf("flow = %lf\n", graph.get_flow());

  // Get Result
  QList<uint64_t> result;
  int w = model.width, h = model.height, d = model.depth;
  for (int x = 0; x < w; ++x) {
    for (int y = 0; y < h; ++y) {
      for (int z = 0; z < d; ++z) {
        int node_id = graph.node_id(x, y, z);
        if (graph.get_segment(node_id) == 0) {
          result.append(morton_encode(x, y, z));
        }
      }
    }
  }

  return result;
}

}
