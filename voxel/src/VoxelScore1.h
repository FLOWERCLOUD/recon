#pragma once

#include "Camera.h"
#include "VoxelModel.h"
#include "Epipolar.h"
#include "Correlation.h"
#include <QList>
#include <QImage>
#include <iterator>

namespace recon {

struct ClosestCameras {
  static const int MAX_NUM = 4;
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

    append_cameras(x, 0.9396926207859084f, 0.984807753012208f); // 10 - 20 deg
    //append_cameras(x, 0.984807753012208f, 0.9961946980917455f); // 5 - 10 deg
    //append_cameras(x, 0.9063077870366499f, 0.9396926207859084f); // 20 - 25 deg
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

struct PeakFinder {
  constexpr static int N = 5;
  float xbuf[N];
  double ybuf[N];
  int bufi = 0;

  float x() const
  {
    return xbuf[(N/2)];
  }

  double y() const
  {
    return ybuf[(N/2)];
  }

  bool valid() const
  {
    if (bufi < N)
      return false;

    bool is_maxima = true;
    for (int i = 0; i < N; ++i) {
      is_maxima = is_maxima && (ybuf[(N/2)] >= ybuf[i]);
    }
    return is_maxima;
  }

  void push(float x, double y)
  {
    if (bufi < N) {
      xbuf[bufi] = x, ybuf[bufi] = y;
      bufi++;
    } else {
      memmove(xbuf, xbuf+1, sizeof(float) * (N-1));
      memmove(ybuf, ybuf+1, sizeof(double) * (N-1));
      xbuf[N-1] = x, ybuf[N-1] = y;
    }
  }
};

struct VoxelScore1 {
  float voxel_size;
  ClosestCameras ccams;
  SampleWindow swin_i;
  Ray3 ray;
  QList<QPointF> sjdk;

  VoxelScore1(const QList<Camera>& cams,
              const QList<QImage>& imgs,
              int cam_i, Point3 x, float voxel_h)
  : voxel_size(voxel_h)
  , ccams(cams, imgs, cam_i, x)
  {
    const Camera& ci = cams.at(cam_i);
    const QImage& image_i = imgs.at(cam_i);
    swin_i = SampleWindow(image_i, Vec3::proj(transform(ccams.txfm_i, x)));
    ray = Ray3(x, normalize(ci.center() - x) * voxel_h * 0.707f);

    sjdk.reserve(16);
    for (int i = 0; i < ccams.num; ++i) {
      find_peaks(i);
    }
    std::sort(sjdk.begin(), sjdk.end(),
              [](QPointF a, QPointF b){ return a.x() < b.x(); });
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

    PeakFinder peak;
    epipolar.per_pixel<false>(
      [&peak,&image_j,this,&epipolar]
      (Vec3 pt0, Vec3 pt1) {
        // TODO : should consider voxel space....
        float depth = (float)pt0.z();
        SampleWindow swj(image_j, pt0);
        float ncc = NormalizedCrossCorrelation(swin_i, swj);
        peak.push(depth, ncc);
        if (peak.valid()) {
          //if (peak.y() > 0.5f)
          sjdk.append(QPointF(peak.x(), peak.y()));
        }
      }
    , 3.0f);
  }

  static inline double parzen_window(float x)
  {
    //const double sigma = 1.0;
    //double a = x / sigma;
    //return exp(-0.5 * a * a);
    return (fabsf(x) <= 1.0f ? 1.0 : 0.0);
  }

  inline double compute(float d) const
  {
    double sum = 0.0;
    for (QPointF ds : sjdk) {
      float dk = ds.x();
      double sidk = ds.y();
      sum += sidk * parzen_window(d - dk);
    }
    return sum;
  }

  inline double vote() const
  {
    if (ccams.num < 1)
      return 0.0;

    double c0 = compute(0.0);
    //for (QPointF s : sjdk) {
    //  if (fabs(s.x()) <= 1.0)
    //    c0 = fmax(c0, compute(s.x()));
    //}
    /*for (int i = 0; i < ccams.num; ++i) {
      auto epipolar = make_epipolar(i);
      epipolar.per_pixel<false>(
        [&c0,this](Vec3 pt0, Vec3 pt1){
          c0 = fmax(c0, compute((float)pt0.z()));
        },
      1.0f);
    }*/
    {
      auto epipolar = make_epipolar(0);
      epipolar.per_pixel<false>(
        [&c0,this](Vec3 pt0, Vec3 pt1){
          c0 = fmax(c0, compute((float)pt0.z()));
        },
      1.0f);
    }

    // NOTE: thresholding to eliminate outliers
    //if (c0 < 0.5)
    //  return 0.0;

    bool ok = true;
    //for (QPointF s : sjdk) {
    //  ok = ok && (c0 >= compute(s.x()));
    //}
    auto epipolar = make_epipolar(0);
    epipolar.per_pixel<false>(
      [c0,&ok,this](Vec3 pt0, Vec3 pt1){
        float c = compute((float)pt0.z());
        ok = ok && (c0 >= c);
      },
    3.0f);

    return (ok ? c0 : 0.0);
  }
};

}
