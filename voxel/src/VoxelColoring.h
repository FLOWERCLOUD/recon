#pragma once

#include "VoxelBlock.h"
#include "CameraData.h"
#include "AABB.h"
#include <QString>
#include <QList>
#include <QStringList>
#include <QImage>

namespace voxel {

class VoxelColoring {
public:
  VoxelColoring(QString bundle_path);
  ~VoxelColoring();

  bool process();

  void save_to_png_set(const QString& basename);
  //void save_to_ply(const QString& path);

private:
  void paint_voxel(int x, int y, int z);

private:
  QStringList m_ImagePaths;
  QList<CameraData> m_Cameras;
  AABB m_ModelAABB;
  QList<QImage> m_Images;

  VoxelBlock m_Voxels;
};

}
