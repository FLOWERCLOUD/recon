#pragma once

#ifndef _VOXEL_CAMERADATA_H_
#define _VOXEL_CAMERADATA_H_

#include <vectormath.h>

namespace voxel {

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
};

inline vec3 CameraData::get_position() const
{
  using vectormath::aos::load_vec3;
  return load_vec3(center);
}

inline void CameraData::store_position(vec3 pos)
{
  store_vec3(center, pos);
}

inline quat CameraData::get_orientation() const
{
  using vectormath::aos::load_quat;
  return load_quat(orientation);
}

inline void CameraData::store_orientation(quat q)
{
  store_quat(orientation, q);
}

inline mat4 CameraData::get_extrinsic() const
{
  using vectormath::aos::load_mat3;
  using vectormath::aos::load_vec3;
  using vectormath::aos::make_mat4;
  return make_mat4(load_mat3(extrinsic), load_vec3(extrinsic+9));
}

inline mat3 CameraData::get_intrinsic() const
{
  using vectormath::aos::load_mat3;
  return load_mat3(intrinsic);
}

void CameraData::update_extrinsic()
{
  mat3 rot = to_mat3(get_orientation());
  vec3 pos = get_position();
  vec3 trans = -(rot * pos);
  store_mat3(extrinsic+0, rot);
  store_vec3(extrinsic+9, trans);
}

void CameraData::update_intrinsic()
{
  float f = focal_length;
  float a = aspect_ratio;
  intrinsic[0] = f / a * 2.0f;
  intrinsic[1] = 0.0f;
  intrinsic[2] = 0.0f;
  intrinsic[3] = 0.0f;
  intrinsic[4] = f * 2.0f;
  intrinsic[5] = 0.0f;
  intrinsic[6] = 0.0f;
  intrinsic[7] = 0.0f;
  intrinsic[8] = 1.0f;
}

}

#endif /* end of include guard: _VOXEL_CAMERADATA_H_ */
