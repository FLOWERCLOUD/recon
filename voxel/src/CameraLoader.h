#pragma once

#include "Camera.h"
#include "AABox.h"

#include <QList>
#include <QString>
#include <QStringList>
#include <QSize>

namespace recon {

class CameraLoader {
public:
  CameraLoader();
  ~CameraLoader();

  bool load_from_nvm(const QString& path);

  const QList<Camera>& cameras() const;
  const AABox& feature_boundingbox() const;

  void debug_render_features(const QString& path, int camera_id) const;

private:
  struct FeatureData {
    float pos[3];
    uint32_t color;
  };

private:
  AABox m_FeatureAABB;
  QList<FeatureData> m_Features;
  QList<Camera> m_Cameras;
};

}
