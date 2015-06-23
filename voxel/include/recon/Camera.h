#pragma once

#include <vectormath.h>
#include <vectormath/aos/utils/aabox.h>
#include <vectormath/aos/utils/point3.h>
#include <QSharedDataPointer>
#include <QString>

namespace recon {

using vectormath::aos::vec3;
using vectormath::aos::vec4;
using vectormath::aos::quat;
using vectormath::aos::mat3;
using vectormath::aos::mat4;
using vectormath::aos::utils::point3;
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

  point3 center() const;
  void setCenter(point3);

  mat3 rotation() const;
  void setRotation(mat3);
  void setRotation(quat);

  vec3 direction() const;

  mat4 extrinsic() const;
  mat4 intrinsicForViewport() const;
  mat4 intrinsicForImage(int width, int height) const;

  QString imagePath() const;
  void setImagePath(const QString&);

  int imageWidth() const;
  int imageHeight() const;

  QString maskPath() const;
  void setMaskPath(const QString&);

  bool canSee(vec3 pt) const;

private:
  //QSharedDataPointer<CameraData> data;
  QExplicitlySharedDataPointer<CameraData> data;
};

// TODO: undistortion

}

Q_DECLARE_TYPEINFO(recon::Camera, Q_MOVABLE_TYPE);
