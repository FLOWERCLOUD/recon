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

  PhotoConsistency(const VoxelModel& model, const QList<Camera>& cams)
  : voxel_size((float)model.virtual_box.extent().x() / model.width)
  , cameras(cams)
  {
    images.reserve(cams.size());
    for (int i = 0; i < cams.size(); ++i) {
      QImage img = QImage(cameras[i].imagePath());
      //if (img.width() > 640)
      //  img = img.scaledToWidth(640, Qt::SmoothTransformation);
      if (img.width() > 960)
        img = img.scaledToWidth(img.width()/2, Qt::SmoothTransformation);
      images.append(img);
    }
  }

  double vote(Point3 x) const
  {
    double sum = 0.0;
    for (int i = 0, n = cameras.size(); i < n; ++i) {
      TVoxelScore score(cameras, images, i, x, voxel_size);
      sum += score.vote();
    }
    return sum;
  }
};

}
