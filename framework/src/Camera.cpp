#include "Camera.h"
#include <vectormath.h>
#include <QString>
#include <QTextStream>

namespace recon {

using vectormath::aos::vec3;
using vectormath::aos::vec4;
using vectormath::aos::quat;
using vectormath::aos::mat3;
using vectormath::aos::mat4;
using vectormath::aos::load_vec3;
using vectormath::aos::store_vec3;
using vectormath::aos::load_mat3;
using vectormath::aos::store_mat3;
using vectormath::aos::load_quat;

void load_from_nvm(Camera& cam, QString& imagename, QTextStream& stream)
{
  float focal_length;
  float orient[4]; // XYZW
  float center[3];
  float radial_distortion;
  int temp;

  stream >> imagename;
  stream >> focal_length;
  stream >> orient[3] >> orient[0] >> orient[1] >> orient[2];
  stream >> center[0] >> center[1] >> center[2];
  stream >> radial_distortion;
  stream >> temp; // END of camera

  cam.focal_length = focal_length;
  cam.radial_distortion[0] = radial_distortion;
  cam.radial_distortion[1] = 0.0f;

  cam.intrinsic[0] = focal_length;
  cam.intrinsic[1] = 0.0f;
  cam.intrinsic[2] = 0.0f;
  cam.intrinsic[3] = 0.0f;
  cam.intrinsic[4] = focal_length;
  cam.intrinsic[5] = 0.0f;
  cam.intrinsic[6] = 0.0f;
  cam.intrinsic[7] = 0.0f;
  cam.intrinsic[8] = focal_length;

  memcpy(cam.center, center, sizeof(float)*3);

  mat3 rot = to_mat3(load_quat(orient));
  vec3 pos = load_vec3(center);
  vec3 trans = -(rot * pos);
  store_mat3(cam.extrinsic+0, rot);
  store_vec3(cam.extrinsic+9, trans);
}

}
