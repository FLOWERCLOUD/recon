#pragma once

#include <vectormath.h>
#include <QObject>
#include <QVector2D>
#include <QVector3D>
#include <QQuaternion>
#include <QMatrix4x4>

namespace recon {

struct CameraData {
  float focal_length; // as if height = 1
  float aspect_ratio; // width / height
  float radial_distortion[2];

  float center[3];
  float orientation[4];

  float intrinsic[9]; // 3x3 column-major intrinsic matrix K
  float extrinsic[12]; // 4x3 column-major extrinsic matrix [R|T]
  // map 3d points from world space to view space
};

void update_intrinsic(CameraData* camdata);
void update_extrinsic(CameraData* camdata);

class Camera : public QObject {
  Q_OBJECT
  Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)

  Q_PROPERTY(QVector3D position READ position WRITE setPosition)
  Q_PROPERTY(QQuaternion orientation READ orientation WRITE setOrientation)
  Q_PROPERTY(float focal READ focal WRITE setFocal)
  Q_PROPERTY(QVector2D distortion READ distortion WRITE setDistortion)
  Q_PROPERTY(QMatrix4x4 world2view READ world2view)
  Q_PROPERTY(QMatrix4x4 view2world READ view2world)
  Q_PROPERTY(QMatrix4x4 calibration READ calibration)

public:
  Camera(QObject* parent = 0);
  virtual ~Camera();

  QString name() const;
  void setName(const QString& name);

  QVector3D position() const;
  void setPosition(const QVector3D&);
  QQuaternion orientation() const;
  void setOrientation(const QQuaternion&);

  float focal() const;
  void setFocal(float focal);
  QVector2D distortion() const;
  void setDistortion(const QVector2D&);

  QMatrix4x4 world2view() const;
  QMatrix4x4 view2world() const;
  QMatrix4x4 calibration() const;

signals:
  void nameChanged(QString name);

private:
  void updateExtrinsic() const;
  void updateIntrinsic() const;

private:
  QString m_Name;
  mutable CameraData m_Data;
  mutable bool m_Intrinsic_DirtyFlag;
  mutable bool m_Extrinsic_DirtyFlag;
};

}
