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

using vectormath::aos::Vec3;
using vectormath::aos::Vec4;
using vectormath::aos::utils::Point3;
using vectormath::aos::utils::Ray3;

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
  //std::vector<QList<int>> closest_camera_lists;
  std::vector<QImage> images;

  PhotoConsistency(const VoxelModel& model, const QList<Camera>& cams)
  : model_box(model.real_box)
  , voxel_size((float)model.virtual_box.extent().x() / model.width)
  , cameras(cams)
  //, closest_camera_lists(cams.size())
  , images(cams.size())
  {
  }

  QList<int> closest_cameras(int cam_i, Point3 x)
  {
    QList<int> jcams;
    Camera ci = cameras[cam_i];
    Vec3 ni = normalize(ci.center() - x);
    for (int j = 0, n = cameras.size(); j < n; ++j) {
      Camera cj = cameras[j];
      Vec3 nj = normalize(cj.center() - x);
      float dp = (float)dot(ni, nj);
      if (dp <= 0.984807753012208f && dp >= 0.9396926207859084f) {
        jcams.append(j);
      }
    }
    return jcams;
  }

  QImage image(int cam_id)
  {
    if (images[cam_id].isNull()) {
      images[cam_id] = QImage(cameras[cam_id].imagePath());
    }
    return images[cam_id];
  }

  SampleWindow sample(int cam_i, Point3 pos)
  {
    Camera cam = cameras[cam_i];
    QImage img = image(cam_i);
    Mat4 txfm = cam.intrinsicForImage(img.width(), img.height());
    txfm = txfm * cam.extrinsic();
    return SampleWindow(img, Vec3::proj(transform(txfm, pos)));
  }

  struct VoxelScore {
    QList<QImage> image_js;
    QList<Mat4> txfm_js;
    Ray3 o;
    SampleWindow sw_i;

    VoxelScore(PhotoConsistency* pc, int cam_i, Point3 x)
    : image_js()
    , txfm_js()
    , o()
    , sw_i()
    {
      {
        Camera ci = pc->cameras[cam_i];
        QImage image_i = pc->image(cam_i);
        int w = image_i.width(), h = image_i.height();
        Mat4 txfm_i = ci.intrinsicForImage(w, h) * ci.extrinsic();
        sw_i = SampleWindow(image_i, Vec3::proj(transform(txfm_i, x)));

        Vec3 dir = normalize(ci.center() - x);
        dir = dir * pc->voxel_size * 0.5f;
        o = Ray3(x, dir);
      }

      for (int cam_j : pc->closest_cameras(cam_i, x)) {
        Camera cj = pc->cameras[cam_j];
        QImage image_j = pc->image(cam_j);
        int w = image_j.width(), h = image_j.height();
        Mat4 txfm_j = cj.intrinsicForImage(w, h) * cj.extrinsic();
        image_js.append(image_j);
        txfm_js.append(txfm_j);
      }
    }

    inline double operator()(float d) const
    {
      int nj = image_js.size();
      double sum = 0.0;
      for (int j = 0; j < nj; ++j) {
        QImage image_j = image_js[j];
        Mat4 txfm_j = txfm_js[j];
        // Compute NCC
        SampleWindow sw_j(image_j, Vec3::proj(transform(txfm_j, o[d])));
        sum += NormalizedCrossCorrelation(sw_i, sw_j);
      }
      return sum / (double)nj;
    }
  };

  double vote(int cam_i, Point3 pos)
  {
    static const float drange[] = {
      -5.0f,-4.9f,-4.8f,-4.7f,-4.6f,-4.5f,-4.4f,-4.3f,-4.2f,-4.1f,
      -4.0f,-3.9f,-3.8f,-3.7f,-3.6f,-3.5f,-3.4f,-3.3f,-3.2f,-3.1f,
      -3.0f,-2.9f,-2.8f,-2.7f,-2.6f,-2.5f,-2.4f,-2.3f,-2.2f,-2.1f,
      -2.0f,-1.9f,-1.8f,-1.7f,-1.6f,-1.5f,-1.4f,-1.3f,-1.2f,-1.1f,
      -1.0f,-0.9f,-0.8f,-0.7f,-0.6f,-0.5f,-0.4f,-0.3f,-0.2f,-0.1f,
      0.0f, 0.1f, 0.2f, 0.3f, 0.4f, 0.5f, 0.6f, 0.7f, 0.8f, 0.9f,
      1.0f, 1.1f, 1.2f, 1.3f, 1.4f, 1.5f, 1.6f, 1.7f, 1.8f, 1.9f,
      2.0f, 2.1f, 2.2f, 2.3f, 2.4f, 2.5f, 2.6f, 2.7f, 2.8f, 2.9f,
      3.0f, 3.1f, 3.2f, 3.3f, 3.4f, 3.5f, 3.6f, 3.7f, 3.8f, 3.9f,
      4.0f, 4.1f, 4.2f, 4.3f, 4.4f, 4.5f, 4.6f, 4.7f, 4.8f, 4.9f,
      5.0f
    };
    static const int dsamples = sizeof(drange) / sizeof(float);

    double c[dsamples];
    VoxelScore score(this, cam_i, pos);
    for (int k = 0; k < dsamples; ++k) {
      float d = drange[k];
      c[k] = score(d);
    }

    double c0 = 0.0;
    for (int k = 0; k < dsamples; ++k) {
      if (fabsf(drange[k]) <= 1.0f)
        c0 = fmax(c0, c[k]);
    }

    if (std::all_of(c, c+dsamples, [c0](double cd){ return c0 >= cd; }))
      return c0;
    return 0.0;
  }

  double compute(Point3 pos)
  {
    const double param_mju = 0.05 * 256.0;

    double sum = 0.0;
    for (int i = 0, n = cameras.size(); i < n; ++i) {
      sum += vote(i, pos);
    }
    return exp(-param_mju * sum);
  }

};

VoxelList graph_cut(const VoxelModel& model, const QList<Camera>& cameras)
{
  // Allocate Graph
  using GridGraph = GridGraph_3D_6C<double, double, double>;
  GridGraph graph(model.width, model.height, model.depth);

  //
  float voxel_h = (float)model.virtual_box.extent().x() / model.width;
  float param_lambda = 0.5f; // TODO: 0.5f

  // vhull map
  std::vector<bool> foreground(model.morton_length, false);

  // Setup Graph - Visual Hull
  printf("processing shape prior (visual hull)...\n");
  {
    QList<uint64_t> voxels = visual_hull(model, cameras);
    double weight = (double)param_lambda * pow(voxel_h, 3.0);
    //printf("Wb = %f\n", weight);

    for (uint64_t m : voxels)
      foreground[m] = true;

    for (uint64_t m = 0, n = model.morton_length; m < n; ++m) {
      uint32_t x, y, z;
      morton_decode(m, x, y, z);

      if (foreground[m]) {
        graph.set_terminal_cap(graph.node_id(x, y, z), weight, 0.0);
      } else {
        graph.set_terminal_cap(graph.node_id(x, y, z), 0.0, INFINITY);
        //graph.set_terminal_cap(graph.node_id(x, y, z), weight, INFINITY);
      }
    }
  }

  // Setup Graph - Photo Consistency
  printf("processing surface prior (photo consistency)...\n");
  {
    PhotoConsistency pc(model, cameras);
    double weight = 4.0 / 3.0 * M_PI * voxel_h * voxel_h;

    for (uint64_t m = 0, n = model.morton_length; m < n; ++m) {
      uint32_t x, y, z;
      morton_decode(m, x, y, z);
      int node = graph.node_id(x, y, z);
      AABox vbox = model.element_box(m);
      Vec3 center = (Vec3)vbox.center();
      Vec3 minpos = (Vec3)vbox.minpos;
      printf("current voxel = %d %d %d\n", x, y, z);
      if (x > 0) {
        double w;
        if (foreground[m] || foreground[morton_encode(x-1,y,z)]) {
          w = weight * pc.compute((Point3)copy_x(center, minpos));
        } else {
          w = INFINITY;
        }
        graph.set_neighbor_cap(node,-1, 0, 0, w);
        graph.set_neighbor_cap(graph.node_id(x-1, y, z),1,0,0, w);
      }
      if (y > 0) {
        double w;
        if (foreground[m] || foreground[morton_encode(x,y-1,z)]) {
          w = weight * pc.compute((Point3)copy_y(center, minpos));
        } else {
          w = INFINITY;
        }
        graph.set_neighbor_cap(node, 0,-1, 0, w);
        graph.set_neighbor_cap(graph.node_id(x, y-1, z),0,1,0, w);
      }
      if (z > 0) {
        double w;
        if (foreground[m] || foreground[morton_encode(x,y,z-1)]) {
          w = weight * pc.compute((Point3)copy_z(center, minpos));
        } else {
          w = INFINITY;
        }
        graph.set_neighbor_cap(node, 0, 0,-1, w);
        graph.set_neighbor_cap(graph.node_id(x, y, z-1),0,0,1, w);
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
      Point3 pos = model.element_box(morton).center();

      float v = pc.vote_1(cam_i, pos);

      int pix = (int)(fminf(fmaxf(v, 0.0f), 1.0f) * 255) & 0xFF;
      canvas.setPixel(j,i,qRgb(pix, pix, pix));
    }
  }

  canvas.setText("Y Plane", QString::number(plane_y));
  canvas.setText("Camera i", QString::number(cam_i));

  return canvas.mirrored();
}

#endif

}
