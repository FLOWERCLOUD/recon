#include "Camera.h"
#include <QSharedData>
#include <QString>
#include <QList>

namespace recon {

struct CameraData : public QSharedData {
  float focal;
  float aspect;
  float distortion[2];
  float center[4];
  float orientation[4];

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

vec3 Camera::center() const
{
  return vec3::load(data->center);
}

void Camera::setCenter(vec3 pos)
{
  pos.store(data->center);
}

quat Camera::orientation() const
{
  return quat::load(data->orientation);
}

void Camera::setOrientation(quat orient)
{
  orient.store(data->orientation);
}

mat4 Camera::extrinsic() const
{
  mat3 rot = (mat3)orientation();
  vec3 pos = center();
  vec3 trans = -(rot * pos);
  return mat4(rot, trans);
}

mat4 Camera::intrinsicForViewport() const
{
  float f = data->focal;
  float a = data->aspect;

  return mat4{ // column major
    vec4{ (f/a*2.0f), 0.0f, 0.0f, 0.0f },
    vec4{ 0.0f, (-f*2.0f), 0.0f, 0.0f },
    vec4{ 0.0f, 0.0f, 0.0f, 1.0f },
    vec4{ 0.0f, 0.0f, 1.0f, 0.0f }
  };
}

mat4 Camera::intrinsicForImage(int width, int height) const
{
  float f = data->focal;
  float a = data->aspect;

  float fx = f / a * width;
  float fy = f * height;
  float dx = 0.5f * width;
  float dy = 0.5f * height;

  return mat4{ // column major
    vec4{ fx, 0.0f, 0.0f, 0.0f },
    vec4{ 0.0f, fy, 0.0f, 0.0f },
    vec4{ dx, dy, 0.0f, 1.0f },
    vec4{ 0.0f, 0.0f, 1.0f, 0.0f }
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

QString Camera::maskPath() const
{
  return data->mask_path;
}

void Camera::setMaskPath(const QString& path)
{
  data->mask_path = path;
}

// ====================================================================

CameraData::CameraData()
{
  focal = 0.0f;
  aspect = 0.0f;
  memset(distortion, 0, sizeof(float)*2);
  memset(center, 0, sizeof(float)*3);
  memset(orientation, 0, sizeof(float)*4);
}

CameraData::CameraData(const CameraData& other)
: QSharedData(other)
{
  focal = other.focal;
  aspect = other.aspect;
  memcpy(distortion, other.distortion, sizeof(float)*2);
  memcpy(center, other.center, sizeof(float)*3);
  memcpy(orientation, other.orientation, sizeof(float)*4);
  image_path = other.image_path;
  mask_path = other.mask_path;
}

CameraData::~CameraData()
{
}

}
