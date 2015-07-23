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

  ClosestCameras(const QList<Camera>& cams, const QList<QImage>& imgs, int i, Point3 x);
  bool append_cameras(Point3 x, float cos_min, float cos_max);
};

struct VoxelScore1 {
  float voxel_size;
  ClosestCameras ccams;
  SampleWindow swin_i;
  Ray3 ray;
  QList<QPointF> sjdk;

  VoxelScore1(const QList<Camera>& cams,
              const QList<QImage>& imgs,
              int cam_i, Point3 x, float voxel_h);
  double compute(float d) const;
  double vote() const;

private:
  inline Epipolar make_epipolar(int ith_jcam) const;
  inline void find_peaks(int ith_jcam);
  static inline double parzen_window(float x);
};

}
