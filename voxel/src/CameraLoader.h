#pragma once

#include "Camera.h"
#include <vectormath/aos/utils/aabox.h>
#include <QList>
#include <QString>
#include <QStringList>
#include <QSize>

namespace recon {

using vectormath::aos::utils::AABox;

class CameraLoader {
public:
  CameraLoader();
  ~CameraLoader();

  bool load_from_nvm(const QString& path);

  const QList<Camera>& cameras() const;
  const AABox& model_boundingbox() const;

  void debug_render_features(const QString& path, int camera_id) const;

private:
  struct FeatureData {
    float pos[3];
    uint32_t color;
  };

private:
  AABox m_ModelBox;
  QList<FeatureData> m_Features;
  QList<Camera> m_Cameras;
};

}
