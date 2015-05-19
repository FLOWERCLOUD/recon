#pragma once

#ifndef _VOXEL_CAMERALOADER_H_
#define _VOXEL_CAMERALOADER_H_

#include "CameraData.h"
#include "AABB.h"

#include <QList>
#include <QString>
#include <QStringList>

namespace voxel {

typedef QList<CameraData> CameraList;

class CameraLoader {
public:
  CameraLoader();
  ~CameraLoader();

  bool load_from_nvm(const QString& path);

  const QStringList& image_paths() const;
  const CameraList& cameras() const;
  const AABB& feature_bounding() const;

private:
  AABB m_FeatureAABB;
  QStringList m_ImagePaths;
  CameraList m_Cameras;
};

}

#endif /* end of include guard: _VOXEL_CAMERALOADER_H_ */
