#pragma once

#include "Camera.h"
#include <QIODevice>

namespace recon {

struct FeatureVertex {
  float pos[3];
  uint8_t color[3];
};

class Bundle {
public:
  Bundle();
  ~Bundle();

  int camera_count() const;
  const Camera& get_camera(int index) const;

  int feature_count() const;
  const FeatureVertex* get_features() const;

  void clear();

  bool load_nvm(QIODevice* io);

private:
  Bundle(const Bundle&) = delete;
  void operator=(const Bundle&) = delete;

  void allocate_cameras(int n);
  void allocate_features(int n);

private:
  int m_CamerasNum;
  int m_CamerasCapacity;
  Camera* m_Cameras;

  int m_FeaturesNum;
  int m_FeaturesCapacity;
  FeatureVertex* m_Features;
};

}
