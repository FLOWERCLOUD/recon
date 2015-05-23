#pragma once

#include <vectormath.h>

namespace recon {

using vectormath::aos::vec3;
using vectormath::aos::vec4;
using vectormath::aos::quat;
using vectormath::aos::mat3;
using vectormath::aos::mat4;

struct CameraData {
  float focal_length; // as if height = 1
  float aspect_ratio; // width / height
  float radial_distortion[2];

  float center[3];
  float orientation[4];

  float intrinsic[9]; // 3x3 column-major intrinsic matrix K
  float extrinsic[12]; // 4x3 column-major extrinsic matrix [R|T]
  // map 3d points from world space to view space

  inline vec3 get_position() const;
  inline void store_position(vec3);

  inline quat get_orientation() const;
  inline void store_orientation(quat);

  inline mat4 get_extrinsic() const;
  inline mat3 get_intrinsic() const;

  //inline mat4 get_world2view() const; // extrinsic
  //inline mat4 get_view2world() const; // extrinsic

  inline void update_extrinsic();
  inline void update_intrinsic();

  inline vec3 world_to_viewport(vec3 pos) const;
  inline vec3 world_to_image(vec3 pos, int width, int height) const;
};

inline vec3 CameraData::get_position() const
{
  return vec3::load(center);
}

inline void CameraData::store_position(vec3 pos)
{
  pos.store(center);
}

inline quat CameraData::get_orientation() const
{
  return quat::load(orientation);
}

inline void CameraData::store_orientation(quat q)
{
  q.store(orientation);
}

inline mat4 CameraData::get_extrinsic() const
{
  return mat4(mat3::load(extrinsic), vec3::load(extrinsic+9));
}

inline mat3 CameraData::get_intrinsic() const
{
  return mat3::load(intrinsic);
}

inline void CameraData::update_extrinsic()
{
  mat3 rot = (mat3)get_orientation();
  vec3 pos = get_position();
  vec3 trans = -(rot * pos);
  rot.store(extrinsic+0);
  trans.store(extrinsic+9);
}

inline void CameraData::update_intrinsic()
{
  float f = focal_length;
  float a = aspect_ratio;
  intrinsic[0] = f / a * 2.0f;
  intrinsic[1] = 0.0f;
  intrinsic[2] = 0.0f;
  intrinsic[3] = 0.0f;
  intrinsic[4] = -f * 2.0f;
  intrinsic[5] = 0.0f;
  intrinsic[6] = 0.0f;
  intrinsic[7] = 0.0f;
  intrinsic[8] = 1.0f;
}

inline vec3 CameraData::world_to_viewport(vec3 pos) const
{
  vec3 pt = to_vec3(get_extrinsic() * vec4(pos, 1.0f));
  pt = get_intrinsic() * pt;
  pt = pt / pt.z();
  return pt;
}

inline vec3 CameraData::world_to_image(vec3 pos, int width, int height) const
{
  vec3 pt = world_to_viewport(pos);
  pt = pt * 0.5f + vec3(0.5f, -0.5f, 0.0f);

  mat3 scale = mat3::scaling((float)width, (float)-height, 1.0f);
  pt = scale * pt;
  return pt;
}

}
