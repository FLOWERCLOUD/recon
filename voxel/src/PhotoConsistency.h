#pragma once

#include "Camera.h"
#include "VoxelModel.h"
#include <QList>
#include <QImage>

namespace recon {

template<typename TVoxelScore>
struct PhotoConsistency {
  float voxel_size;
  QList<Camera> cameras;
  QList<QImage> images;
  mutable std::vector<QPointF> shared_sjdk;

  PhotoConsistency(const VoxelModel& model, const QList<Camera>& cams)
  : voxel_size((float)model.virtual_box.extent().x() / model.width)
  , cameras(cams)
  {
    images.reserve(cams.size());
    for (int i = 0; i < cams.size(); ++i)
      images.append(QImage(cameras[i].imagePath()));
    shared_sjdk.reserve(128);
  }

  double vote(Point3 x) const
  {
    double sum = 0.0;
    for (int i = 0, n = cameras.size(); i < n; ++i) {
      TVoxelScore score(cameras, images, i, x, voxel_size, shared_sjdk);
      sum += score.vote();
    }
    return sum;
  }
};

}
