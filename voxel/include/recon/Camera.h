#pragma once

#include <vectormath.h>
#include <vectormath/aos/utils/aabox.h>
#include <vectormath/aos/utils/point3.h>
#include <QSharedDataPointer>
#include <QString>

namespace recon {

using vectormath::aos::Vec3;
using vectormath::aos::Vec4;
using vectormath::aos::Quat;
using vectormath::aos::Mat3;
using vectormath::aos::Mat4;
using vectormath::aos::utils::Point3;
using vectormath::aos::utils::AABox;

struct CameraData;

class Camera {
public:
  Camera();
  Camera(const Camera&);
  ~Camera();
  Camera& operator=(const Camera&);

public:
  struct RadialDistortion {
    float k1;
    float k2;
  };

  float focal() const;
  void setFocal(float focal);

  float aspect() const;
  void setAspect(float aspect);

  RadialDistortion distortion() const;
  void setRadialDistortion(const RadialDistortion&);
  void setRadialDistortion(float, float);

  Point3 center() const;
  void setCenter(Point3);

  Mat3 rotation() const;
  void setRotation(Mat3);
  void setRotation(Quat);

  Vec3 direction() const;

  Mat4 extrinsic() const;
  Mat4 intrinsicForViewport() const;
  Mat4 intrinsicForImage(int width, int height) const;

  QString imagePath() const;
  void setImagePath(const QString&);

  int imageWidth() const;
  int imageHeight() const;

  QString maskPath() const;
  void setMaskPath(const QString&);

  bool canSee(Point3 pt) const;

private:
  //QSharedDataPointer<CameraData> data;
  QExplicitlySharedDataPointer<CameraData> data;
};

// TODO: undistortion

}

Q_DECLARE_TYPEINFO(recon::Camera, Q_MOVABLE_TYPE);
