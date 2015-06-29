#include "Camera.h"
#include <QSharedData>
#include <QString>
#include <QList>
#include <QImageReader>
#include <math.h>
#include <stdio.h>

namespace recon {

struct CameraData : public QSharedData {
  float focal;
  float aspect;
  float distortion[2];
  float center[3];
  float rotation[9]; // 3x3 column-major matrix

  QString image_path;
  QString mask_path;

  CameraData();
  CameraData(const CameraData&);
  ~CameraData();
};

Camera::Camera()
: data(new CameraData())
{
}

Camera::Camera(const Camera& other)
: data(other.data)
{
}

Camera::~Camera()
{
}

Camera& Camera::operator=(const Camera& other)
{
  data = other.data;
  return *this;
}

float Camera::focal() const
{
  return data->focal;
}

void Camera::setFocal(float focal)
{
  data->focal = focal;
}

float Camera::aspect() const
{
  return data->aspect;
}

void Camera::setAspect(float aspect)
{
  data->aspect = aspect;
}

Camera::RadialDistortion Camera::distortion() const
{
  float k1 = data->distortion[0];
  float k2 = data->distortion[1];
  return RadialDistortion{ k1, k2 };
}

void Camera::setRadialDistortion(const Camera::RadialDistortion& d)
{
  data->distortion[0] = d.k1;
  data->distortion[1] = d.k2;
}

void Camera::setRadialDistortion(float k1, float k2)
{
  data->distortion[0] = k1;
  data->distortion[1] = k2;
}

Point3 Camera::center() const
{
  return Point3::load(data->center);
}

void Camera::setCenter(Point3 pos)
{
  pos.store(data->center);
}

Mat3 Camera::rotation() const
{
  return Mat3::load(data->rotation);
}

void Camera::setRotation(Mat3 rot)
{
  rot.store(data->rotation);
}

void Camera::setRotation(Quat rot)
{
  setRotation((Mat3)rot);
}

Vec3 Camera::direction() const
{
  return rotation() * Vec3(0.0f, 0.0f, 1.0f);
}

Mat4 Camera::extrinsic() const
{
  Mat3 rot = rotation();
  Point3 pos = center();
  Vec3 trans = -transform(rot, pos);
  return Mat4(rot, trans);
}

Mat4 Camera::intrinsicForViewport() const
{
  float f = data->focal;
  float a = data->aspect;

  return Mat4{ // column major
    Vec4{ (f/a*2.0f), 0.0f, 0.0f, 0.0f },
    Vec4{ 0.0f, (-f*2.0f), 0.0f, 0.0f },
    Vec4{ 0.0f, 0.0f, 0.0f, 1.0f },
    Vec4{ 0.0f, 0.0f, 1.0f, 0.0f }
  };
}

Mat4 Camera::intrinsicForImage(int width, int height) const
{
  float f = data->focal;
  float a = data->aspect;

  float fx = f / a * width;
  float fy = f * height;
  float dx = 0.5f * width;
  float dy = 0.5f * height;

  return Mat4{ // column major
    Vec4{ fx, 0.0f, 0.0f, 0.0f },
    Vec4{ 0.0f, fy, 0.0f, 0.0f },
    Vec4{ dx, dy, 0.0f, 1.0f },
    Vec4{ 0.0f, 0.0f, 1.0f, 0.0f }
  };
}

QString Camera::imagePath() const
{
  return data->image_path;
}

void Camera::setImagePath(const QString& path)
{
  data->image_path = path;
}

int Camera::imageWidth() const
{
  QImageReader reader(imagePath());
  return reader.size().width();
}

int Camera::imageHeight() const
{
  QImageReader reader(imagePath());
  return reader.size().height();
}

QString Camera::maskPath() const
{
  return data->mask_path;
}

void Camera::setMaskPath(const QString& path)
{
  data->mask_path = path;
}

bool Camera::canSee(Point3 pt) const
{
  Mat4 tfm = intrinsicForViewport() * extrinsic();
  Vec3 pos = Vec3::proj(transform(tfm, pt));
  //printf("pos.z = %f\n", (float)pos.z());
  return fabsf((float)pos.x()) <= 1.0f && fabsf((float)pos.y()) <= 1.0f;
  //return true;
}

// ====================================================================

CameraData::CameraData()
{
  focal = 0.0f;
  aspect = 0.0f;
  memset(distortion, 0, sizeof(float)*2);
  memset(center, 0, sizeof(float)*3);
  memset(rotation, 0, sizeof(float)*9);
}

CameraData::CameraData(const CameraData& other)
: QSharedData(other)
{
  focal = other.focal;
  aspect = other.aspect;
  memcpy(distortion, other.distortion, sizeof(float)*2);
  memcpy(center, other.center, sizeof(float)*3);
  memcpy(rotation, other.rotation, sizeof(float)*9);
  image_path = other.image_path;
  mask_path = other.mask_path;
}

CameraData::~CameraData()
{
}

}
