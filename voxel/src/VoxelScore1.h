#pragma once

#include "Camera.h"
#include "VoxelModel.h"
#include "Epipolar.h"
#include "Correlation.h"
#include <QList>
#include <QImage>
#include <vector>

namespace recon {

struct ClosestCameras {
  static const int MAX_NUM = 8;
  int num;
  int cam_i;
  int cam_js[MAX_NUM];
  Mat4 txfm_i;
  Mat4 txfm_js[MAX_NUM];
  const QList<Camera>* _cameras;
  const QList<QImage>* _images;

  ClosestCameras(const QList<Camera>& cams, const QList<QImage>& imgs, int i, Point3 x)
  : num(0)
  , cam_i(i)
  , _cameras(&cams)
  , _images(&imgs)
  {
    Camera ci = cams.at(i);
    QImage img = imgs.at(i);
    Mat4 m = ci.intrinsicForImage(img.width(), img.height());
    txfm_i = m * ci.extrinsic();

    append_cameras(x, 0.9396926207859084f, 0.984807753012208f);
  }

  bool append_cameras(Point3 x, float cos_min, float cos_max)
  {
    if (this->num >= MAX_NUM)
      return false;

    Camera ci = _cameras->at(cam_i);
    Vec3 ni = normalize(ci.center() - x);
    int count = this->num;

    for (int j = 0, n = _cameras->size(); j < n; ++j) {
      Camera cj = _cameras->at(j);
      QImage img = _images->at(j);
      int w = img.width(), h = img.height();
      Vec3 nj = normalize(cj.center() - x);
      float dp = (float)dot(ni, nj);

      if (dp <= cos_max && dp >= cos_min) {
        Mat4 txfm = cj.intrinsicForImage(w, h) * cj.extrinsic();
        this->cam_js[count] = j;
        this->txfm_js[count] = txfm;
        count++;
        if (count == MAX_NUM)
          break;
      }
    }
    this->num = count;
    return true;
  }
};

struct VoxelScore1 {
  float voxel_size;
  ClosestCameras ccams;
  SampleWindow swin_i;
  Ray3 ray;
  std::vector<QPointF>& sjdk; // shared

  VoxelScore1(const QList<Camera>& cams,
              const QList<QImage>& imgs,
              int cam_i, Point3 x, float voxel_h,
              std::vector<QPointF>& shared_sjdk)
  : voxel_size(voxel_h)
  , ccams(cams, imgs, cam_i, x)
  , sjdk(shared_sjdk)
  {
    const Camera& ci = cams.at(cam_i);
    const QImage& image_i = imgs.at(cam_i);
    swin_i = SampleWindow(image_i, Vec3::proj(transform(ccams.txfm_i, x)));
    ray = Ray3(x, normalize(ci.center() - x) * voxel_h * 1.4f);

    sjdk.clear();
    for (int i = 0; i < ccams.num; ++i) {
      find_peaks(i);
    }
  }

  Epipolar make_epipolar(int ith_jcam) const
  {
    int cam_j = ccams.cam_js[ith_jcam];
    Mat4 txfm_j = ccams.txfm_js[ith_jcam];

    const QImage& image_j = ccams._images->at(cam_j);
    int width = image_j.width(), height = image_j.height();

    return Epipolar(width, height, txfm_j, ray);
  }

  void find_peaks(int ith_jcam)
  {
    int cam_j = ccams.cam_js[ith_jcam];
    const QImage& image_j = ccams._images->at(cam_j);
    auto epipolar = make_epipolar(ith_jcam);

    const int bufn = 5;
    float xbuf[bufn];
    double ybuf[bufn];
    int bufi = 0;
    epipolar.walk(
      [&xbuf,&ybuf,&bufi,&image_j,this]
      (float x, float y, float depth) {
        SampleWindow swj(image_j, Vec3(x,y,0.0f));
        float ncc = NormalizedCrossCorrelation(swin_i, swj);
        if (__builtin_expect(bufi == (bufn-1), 1)) {
          xbuf[bufi] = depth;
          ybuf[bufi] = ncc;
          bool is_maxima = true;
          for (int i = 0; i < bufn; ++i) {
            is_maxima = is_maxima && (ybuf[(bufn/2)] >= ybuf[i]);
          }
          if (is_maxima) {
            sjdk.emplace_back(xbuf[(bufn/2)], ybuf[(bufn/2)]);
          }
          memmove(xbuf, xbuf+1, sizeof(float) * (bufn-1));
          memmove(ybuf, ybuf+1, sizeof(double) * (bufn-1));
        } else {
          xbuf[bufi] = depth;
          ybuf[bufi] = ncc;
          bufi++;
        }
      }
    );
  }

  static inline double parzen_window(double x)
  {
    const double sigma = 1.0;
    double a = x / sigma;
    return exp(-0.5 * a * a);
  }

  inline double compute(float d) const
  {
    double sum = 0.0;

    for (QPointF ds : sjdk) {
      double dk = ds.x();
      double sidk = ds.y();
      sum += sidk * parzen_window(d - dk);
    }
    return sum;
  }

  inline double vote() const
  {
    if (ccams.num < 1)
      return 0.0;

    bool ok = true;
    float c0 = compute(0.0);
    auto epipolar = make_epipolar(0);

    epipolar.walk_ranged(1.0f,
      [&c0,this](float x, float y, float d){
        c0 = fmax(c0, compute(d));
      }
    );
    //printf("c0 = %f\n", c0);
    epipolar.walk(
      [c0,&ok,this](float x, float y, float d){
        float c = compute(d);
        //if (c0 < c) {
        //  printf("c(d=%f) = %f\n", d, c);
        //}
        ok = ok && (c0 >= c);
      }
    );

    return (ok ? c0 : 0.0);
  }
};

}
