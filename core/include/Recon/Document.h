#pragma once

#include <vectormath.h>
#include <QObject>
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

class Document : public QObject {
  Q_OBJECT
  Q_PROPERTY(QUrl baseUrl READ baseUrl WRITE setBaseUrl NOTIFY baseUrlChanged)
  Q_PROPERTY(QString basePath READ basePath NOTIFY basePathChanged)
  Q_PROPERTY(int imageCount READ imageCount NOTIFY imageCountChanged)
  Q_PROPERTY(QList<QUrl> imageUrls READ imageUrls NOTIFY imageUrlsChanged)
public:
  Document(QObject* parent = 0);
  ~Document();

  bool isValid() const;

  QUrl baseUrl() const;
  void setBaseUrl(const QUrl& url);
  QString basePath() const;

  const QVector<Camera>& cameras() const;
  const QVector<Feature>& features() const;

  int imageCount() const;
  QList<QUrl> imageUrls() const;

  Q_INVOKABLE bool importImage(const QUrl& url);

  void reload();
  void save();

signals:
  void baseUrlChanged(QUrl url);
  void basePathChanged(QString path);

  void imageUrlsChanged(QList<QUrl> urls);
  void imageCountChanged(int count);

  void imageAdded(QUrl url);

private slots:
  void onBaseUrlChanged(QUrl url);
  void onImageAdded(QUrl url);

private:
  friend class NVMLoader;
  void swapCameras(QVector<Camera>& v);
  void swapFeatures(QVector<Feature>& v);

private:
  QUrl m_BaseUrl;

  QVector<Camera> m_Cameras;
  QVector<Feature> m_Features;

  QList<QUrl> m_ImageUrls;
};

}
