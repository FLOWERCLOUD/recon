#pragma once

#include "FileSet.h"

#include <vectormath.h>

#include <QVector>
#include <QString>
#include <QUrl>
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

class Document : public FileSet {
  Q_OBJECT
  Q_PROPERTY(int imageCount READ imageCount NOTIFY imageCountChanged)
  Q_PROPERTY(QList<QUrl> imageUrls READ imageUrls NOTIFY imageUrlsChanged)
public:
  Document(QObject* parent = 0);
  ~Document();

  const QVector<Camera>& cameras() const;
  const QVector<Feature>& features() const;

  int imageCount() const;
  QList<QUrl> imageUrls() const;

  Q_INVOKABLE bool importImage(const QUrl& url);

  void reload();
  void save();

signals:
  void imageUrlsChanged(QList<QUrl> urls);
  void imageCountChanged(int count);

  void imageAdded(QUrl url);

private slots:
  void onImageAdded(QUrl url);

private:
  friend class NVMLoader;
  void swapCameras(QVector<Camera>& v);
  void swapFeatures(QVector<Feature>& v);

private:
  void ensureInitImageUrls() const;

private:
  QVector<Camera> m_Cameras;
  QVector<Feature> m_Features;

  mutable bool m_ImageUrls_InitFlag;
  mutable QList<QUrl> m_ImageUrls;
};

}
