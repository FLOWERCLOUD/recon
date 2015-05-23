#pragma once

#include "CameraData.h"
#include "AABB.h"
#include <QString>
#include <QList>
#include <QImage>

namespace recon {

struct VisualHullParams {
  QList<CameraData> cameras;
  QList<QString> mask_paths;

  VoxelBlock* block;
  AABB block_bbox;
};

void process(VisualHullParams& params);

}
