#pragma once

#include <vectormath/cpp/vectormath_aos.h>
#include <stdint.h>

namespace recon {

using Vectormath::Aos::Vector3;
using Vectormath::Aos::Matrix3;
using Vectormath::Aos::Quat;

struct Camera {
  int32_t index;
  int32_t flag;

  float focial_length;
  float pixel_aspect_ratio; // pixel_width / pixel_height
  float principal_point[2];
  float radial_distortion[2];

  float orientation[9]; // 3x3 column-major rotation matrix
  float position[3]; // 3-component camera center position
};

inline void set_orientation(Camera& cam, const Quat& q)
{
  Matrix3 mat(q);
  cam.orientation[0] = mat.getElem(0,0);
  cam.orientation[1] = mat.getElem(0,1);
  cam.orientation[2] = mat.getElem(0,2);
  cam.orientation[3] = mat.getElem(1,0);
  cam.orientation[4] = mat.getElem(1,1);
  cam.orientation[5] = mat.getElem(1,2);
  cam.orientation[6] = mat.getElem(2,0);
  cam.orientation[7] = mat.getElem(2,1);
  cam.orientation[8] = mat.getElem(2,2);
}

inline void set_position(Camera& cam, const Vector3& v)
{
  cam.position[0] = v.getX();
  cam.position[1] = v.getY();
  cam.position[2] = v.getZ();
}

}
