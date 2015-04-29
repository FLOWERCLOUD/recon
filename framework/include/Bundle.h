#pragma once

#include "Camera.h"
#include <QIODevice>

namespace recon {

class Bundle {
public:
  Bundle();
  ~Bundle();

  int camera_count() const;
  Camera& get_camera(int index);

  void clear();

  bool load_nvm(QIODevice* io);

private:
  Bundle(const Bundle&) = delete;
  void operator=(const Bundle&) = delete;

  void allocate_cameras(int n);

private:
  int m_CamerasNum;
  int m_CamerasCapacity;
  Camera* m_Cameras;
  //std::vector<Feature> m_Features;
};

}