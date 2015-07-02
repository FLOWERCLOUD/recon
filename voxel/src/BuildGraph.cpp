#include <vectormath.h>
#include <vectormath/aos/utils/ray3.h>

#include "Camera.h"
#include "morton_code.h"
#include "BuildGraph.h"
#include "VisualHull.h"
#include "Correlation.h"

#include <QList>
#include <QImage>
#include <algorithm>
#include <vector>
#include <float.h>
#include <math.h>

namespace recon {

using vectormath::aos::Vec3;
using vectormath::aos::Vec4;
using vectormath::aos::utils::Point3;
using vectormath::aos::utils::Ray3;

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

  const QImage& image(int cam_id)
  {
    if (images[cam_id].isNull()) {
      images[cam_id] = QImage(cameras[cam_id].imagePath());
    }
    return images[cam_id];
  }

  struct VoxelScore {
    static const int MAX_J = 4;
    int num_j;
    int cam_js[MAX_J];
    QImage image_js[MAX_J];
    Mat4 txfm_js[MAX_J];
    Ray3 o;
    SampleWindow sw_i;

    VoxelScore(PhotoConsistency* pc, int cam_i, Point3 x)
    : num_j(0)
    , o()
    , sw_i()
    {
      {
        const Camera& ci = pc->cameras[cam_i];
        const QImage& image_i = pc->image(cam_i);
        int w = image_i.width(), h = image_i.height();
        Mat4 txfm_i = ci.intrinsicForImage(w, h) * ci.extrinsic();
        sw_i = SampleWindow(image_i, Vec3::proj(transform(txfm_i, x)));

        Vec3 dir = normalize(ci.center() - x);
        dir = dir * pc->voxel_size * 0.5f;
        o = Ray3(x, dir);
      }

      closest_cameras(pc->cameras, cam_i, x);

      for (int i = 0; i < num_j; ++i) {
        int cam_j = cam_js[i];
        const Camera& cj = pc->cameras[cam_j];
        const QImage& image_j = pc->image(cam_j);
        int w = image_j.width(), h = image_j.height();
        Mat4 txfm_j = cj.intrinsicForImage(w, h) * cj.extrinsic();
        image_js[i] = image_j;
        txfm_js[i] = txfm_j;
      }
    }

    void closest_cameras(const QList<Camera>& cameras, int cam_i, Point3 x)
    {
      int count = 0;
      const Camera& ci = cameras[cam_i];
      Vec3 ni = normalize(ci.center() - x);

      for (int j = 0, n = cameras.size(); j < n; ++j) {
        const Camera& cj = cameras[j];
        Vec3 nj = normalize(cj.center() - x);

        float dp = (float)dot(ni, nj);
        if (dp <= 0.984807753012208f && dp >= 0.9396926207859084f) {
          cam_js[count++] = j;
          if (count >= MAX_J)
            break;
        }
      }

      num_j = count;
    }

    inline double operator()(float d) const
    {
      int nj = num_j;
      double sum = 0.0;
      for (int j = 0; j < nj; ++j) {
        const QImage& image_j = image_js[j];
        Mat4 txfm_j = txfm_js[j];
        // Compute NCC
        SampleWindow sw_j(image_j, Vec3::proj(transform(txfm_j, o[d])));
        sum += NormalizedCrossCorrelation(sw_i, sw_j);
      }
      return sum / (double)nj;
    }

    inline bool valid() const
    {
      return sw_i.valid;
    }
  };

  double vote(int cam_i, Point3 pos)
  {
    static const float drange[] = {
#if 0
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
#else
      -5.0f,-4.5f,-4.0f,-3.5f,-3.0f,-2.5f,-2.0f,-1.5f,-1.0f,-0.5f,
      0.0f, 0.5f, 1.0f, 1.5f, 2.0f, 2.5f, 3.0f, 3.5f, 4.0f, 4.5f,
      5.0f
#endif
    };
    static const int dsamples = sizeof(drange) / sizeof(float);

    VoxelScore score(this, cam_i, pos);
    //if (!score.valid())
    //  return 0.0;

    double c[dsamples];
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

  double total_votes(Point3 pos)
  {
    double sum = 0.0;
    for (int i = 0, n = cameras.size(); i < n; ++i) {
      sum += vote(i, pos);
    }
    return sum;
  }

};

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

    PhotoConsistency pc(model, cameras);
    for (uint64_t m = 0, n = model.morton_length; m < n; ++m) {
      uint32_t x, y, z;
      morton_decode(m, x, y, z);
      AABox vbox = model.element_box(m);
      Vec3 center = (Vec3)vbox.center();
      Vec3 minpos = (Vec3)vbox.minpos;
      //printf("current voxel = %d %d %d\n", x, y, z);

      if (x > 0) {
        uint64_t m2 = morton_encode(x-1,y,z);
        if (graph.foreground[m] || graph.foreground[m2]) {
          Point3 midpoint = (Point3)copy_x(center, minpos);
          double v = pc.total_votes(midpoint);
          x_edges[m2] = v;
        }
      }
      if (y > 0) {
        uint64_t m2 = morton_encode(x,y-1,z);
        if (graph.foreground[m] || graph.foreground[m2]) {
          Point3 midpoint = (Point3)copy_y(center, minpos);
          double v = pc.total_votes(midpoint);
          y_edges[m2] = v;
        }
      }
      if (z > 0) {
        uint64_t m2 = morton_encode(x,y,z-1);
        if (graph.foreground[m] || graph.foreground[m2]) {
          Point3 midpoint = (Point3)copy_z(center, minpos);
          double v = pc.total_votes(midpoint);
          z_edges[m2] = v;
        }
      }
    }
  }

  //
  printf("finished building graph\n");
}

}
