#pragma once

#include "AABox.h"
#include "Camera.h"
#include <QString>
#include <QList>

namespace recon {

class ModelBuilder {
public:
  ModelBuilder(const AABox& box,
               const QList<Camera>& cameras,
               const QString& octreePath,
               const QString& meshPath);
  ~ModelBuilder();

  bool execute();

private:
  //void processVisualHull();
  //void processVoxelColoring();
  //void generateMesh();

private:
  AABox m_ModelBox;
  QList<Camera> m_Cameras;
  QString m_OctreePath;
  QString m_MeshPath;
};

}
