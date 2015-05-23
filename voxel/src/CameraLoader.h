#pragma once

#include "CameraData.h"
#include "AABB.h"

#include <QList>
#include <QString>
#include <QStringList>
#include <QSize>

namespace recon {

typedef QList<CameraData> CameraList;

class CameraLoader {
public:
  CameraLoader();
  ~CameraLoader();

  bool load_from_nvm(const QString& path);

  const QStringList& image_paths() const;
  const CameraList& cameras() const;
  const AABB& feature_boundingbox() const;

  void debug_render_features(const QString& path, int camera_id) const;

private:
  struct FeatureData {
    float pos[3];
    uint32_t color;
  };

private:
  AABB m_FeatureAABB;
  QList<FeatureData> m_Features;
  QStringList m_ImagePaths;
  CameraList m_Cameras;
};

}
