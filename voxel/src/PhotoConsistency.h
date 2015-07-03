#pragma once

#include "Camera.h"
#include "VoxelModel.h"
#include "Epipolar.h"
#include "Correlation.h"
#include <QList>
#include <QImage>

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
    ray = Ray3(x, normalize(ci.center() - x) * voxel_h * 1.4f);

    sjdk.reserve(128);
    for (int i = 0; i < ccams.num; ++i) {
      compute(i);
    }
  }

  void compute(int ith_jcam)
  {
    int cam_j = ccams.cam_js[ith_jcam];
    Mat4 txfm_j = ccams.txfm_js[ith_jcam];
    const QImage& image_j = ccams._images->at(cam_j);
    int width = image_j.width(), height = image_j.height();
    Epipolar epipolar(width, height, txfm_j, ray);

    const int bufn = 5;
    float xbuf[bufn];
    double ybuf[bufn];
    int bufi = 0;
    epipolar.walk(
      [&xbuf,&ybuf,&bufi,&image_j,this]
      (float x, float y, float depth, bool inside) {
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
            sjdk.append(QPointF(xbuf[(bufn/2)], ybuf[(bufn/2)]));
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

  /*inline double operator()(float d) const
  {
    double sum = 0.0;
    for (int i = 0, n = ccams.num; i < n; ++i) {
      const QImage& image_j = ccams.images->at(ccams.cam_js[i]);
      Mat4 txfm_j = ccams.txfm_js[i];
      // Compute NCC
      SampleWindow sw_j(image_j, Vec3::proj(transform(txfm_j, ray[d])));
      sum += NormalizedCrossCorrelation(swin_i, sw_j);
    }
    return sum / (double)ccams.num;
  }*/
};

struct VoxelVote {
  ClosestCameras ccams;



};

struct PhotoConsistency {
  float voxel_size;
  QList<Camera> cameras;
  QList<QImage> images;

  PhotoConsistency(const VoxelModel& model, const QList<Camera>& cams)
  : voxel_size((float)model.virtual_box.extent().x() / model.width)
  , cameras(cams)
  {
    images.reserve(cams.size());
    for (int i = 0; i < cams.size(); ++i)
      images.append(QImage(cameras[i].imagePath()));
  }

  double vote(int cam_i, Point3 x) const
  {
  }
};

}
