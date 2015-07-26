#pragma once

#include "Camera.h"
#include "VoxelModel.h"
#include "VoxelScore1.h"
#include <QList>
#include <QImage>

namespace recon {

struct PhotoConsistency {
  float voxel_size;
  QList<Camera> cameras;
  QList<QImage> images;

  PhotoConsistency(const VoxelModel& model, const QList<Camera>& cams);
  double vote(Point3 x) const;

  static bool EnableAutoThresholding;
  static double VotingThreshold;
};

}
