#pragma once

#include <vectormath.h>
#include <QObject>
#include <QVector>
#include <QString>
#include <stdint.h>

namespace recon {

struct Camera {
  int32_t index;

  float focal_length;
  float radial_distortion[2];
  float _padding;

  float center[3];
  float intrinsic[9]; // 3x3 column-major intrinsic matrix K
  float extrinsic[12]; // 4x3 column-major extrinsic matrix [R|T]
  // map 3d points from world space to view space
};

struct Feature {
  float pos[3];
  uint8_t color[3];
};

class Document : public QObject {
  Q_OBJECT
public:
  Document(const QString& basepath, QObject* parent = 0);
  ~Document();

  const QString& basePath() const;

  const QVector<Camera>& cameras() const;
  const QVector<Feature>& features() const;

  //int numberOfImages() const;
  // getImage()

  //void importImage(const char* path);
  //void importImage(const QString& path);

  void reload();
  void save();

private:
  friend class NVMLoader;
  void swapCameras(QVector<Camera>& v);
  void swapFeatures(QVector<Feature>& v);

private:
  QString m_BasePath;
  QVector<Camera> m_Cameras;
  QVector<Feature> m_Features;
};

}
