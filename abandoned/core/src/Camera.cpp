#include <Recon/Camera.h>
#include <vectormath.h>

namespace recon {

using vectormath::aos::vec3;
using vectormath::aos::vec4;
using vectormath::aos::quat;
using vectormath::aos::mat3;
using vectormath::aos::mat4;
using vectormath::aos::make_zero_vec3;
using vectormath::aos::load_vec3;
using vectormath::aos::store_vec3;
using vectormath::aos::load_mat3;
using vectormath::aos::store_mat3;
using vectormath::aos::load_quat;
using vectormath::aos::make_mat4;
using vectormath::aos::load_mat4;
using vectormath::aos::store_mat4;

void update_intrinsic(CameraData* camdata)
{
  float f = camdata->focal_length;
  float a = camdata->aspect_ratio;
  camdata->intrinsic[0] = f / a * 2.0f;
  camdata->intrinsic[1] = 0.0f;
  camdata->intrinsic[2] = 0.0f;
  camdata->intrinsic[3] = 0.0f;
  camdata->intrinsic[4] = f * 2.0f;
  camdata->intrinsic[5] = 0.0f;
  camdata->intrinsic[6] = 0.0f;
  camdata->intrinsic[7] = 0.0f;
  camdata->intrinsic[8] = 1.0f;
}

void update_extrinsic(CameraData* camdata)
{
  mat3 rot = to_mat3(load_quat(camdata->orientation));
  vec3 pos = load_vec3(camdata->center);
  vec3 trans = -(rot * pos);
  store_mat3(camdata->extrinsic+0, rot);
  store_vec3(camdata->extrinsic+9, trans);
}

Camera::Camera(QObject* parent)
: QObject(parent)
{
  m_Intrinsic_DirtyFlag = false;
  m_Extrinsic_DirtyFlag = false;
}

Camera::~Camera()
{
}

QString Camera::name() const
{
  return m_Name;
}

void Camera::setName(const QString& name)
{
  m_Name = name;
  emit nameChanged(name);
}

QVector3D Camera::position() const
{
  const float* pos = m_Data.center;
  return QVector3D(pos[0], pos[1], pos[2]);
}

void Camera::setPosition(const QVector3D& pos)
{
  m_Data.center[0] = pos.x();
  m_Data.center[1] = pos.y();
  m_Data.center[2] = pos.z();

  m_Extrinsic_DirtyFlag = true;
}

QQuaternion Camera::orientation() const
{
  const float* orient = m_Data.orientation;
  return QQuaternion(orient[3], orient[0], orient[1], orient[2]);
}

void Camera::setOrientation(const QQuaternion& orient)
{
  m_Data.orientation[0] = orient.x();
  m_Data.orientation[1] = orient.y();
  m_Data.orientation[2] = orient.z();
  m_Data.orientation[3] = orient.scalar();

  m_Extrinsic_DirtyFlag = true;
}

float Camera::focal() const
{
  return m_Data.focal_length;
}

void Camera::setFocal(float focal)
{
  m_Data.focal_length = focal;

  m_Intrinsic_DirtyFlag = true;
}

QVector2D Camera::distortion() const
{
  const float* dist = m_Data.radial_distortion;
  return QVector2D(dist[0], dist[1]);
}

void Camera::setDistortion(const QVector2D& dist)
{
  m_Data.radial_distortion[0] = dist.x();
  m_Data.radial_distortion[1] = dist.y();

  m_Intrinsic_DirtyFlag = true;
}

QMatrix4x4 Camera::world2view() const
{
  updateExtrinsic();

  mat3 rot = load_mat3(m_Data.extrinsic);
  vec3 trans = load_vec3(m_Data.extrinsic+9);

  QMatrix4x4 result;
  float* dst = result.data();
  store_mat4(dst, make_mat4(rot, trans));

  return result;
}

QMatrix4x4 Camera::view2world() const
{
  updateExtrinsic();

  mat3 rot = load_mat3(m_Data.extrinsic);
  vec3 trans = load_vec3(m_Data.extrinsic+9);

  rot = transpose(rot);
  trans = rot * (-trans);

  QMatrix4x4 result;
  float* dst = result.data();
  store_mat4(dst, make_mat4(rot, trans));

  return result;
}

QMatrix4x4 Camera::calibration() const
{
  updateIntrinsic();

  mat3 m = load_mat3(m_Data.intrinsic);

  QMatrix4x4 result;
  float* dst = result.data();
  store_mat4(dst, make_mat4(m, make_zero_vec3()));

  return result;
}

void Camera::updateExtrinsic() const
{
  if (m_Extrinsic_DirtyFlag) {
    update_extrinsic(&m_Data);
  }
  m_Extrinsic_DirtyFlag = false;
}

void Camera::updateIntrinsic() const
{
  if (m_Intrinsic_DirtyFlag) {
    update_intrinsic(&m_Data);
  }
  m_Intrinsic_DirtyFlag = false;
}

}
