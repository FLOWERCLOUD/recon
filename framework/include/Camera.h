#pragma once

#include <vectormath.h>
#include <stdint.h>

class QString;
class QTextStream;

namespace recon {

struct Camera {
  int32_t index;

  float focal_length;
  float radial_distortion[2];
  float _padding;

  float center[3];
  float intrinsic[9]; // 3x3 column-major intrinsic matrix K
  float extrinsic[12]; // 4x3 column-major extrinsic matrix [R|T]
  // map 3d points from world space to view space
};

void load_from_nvm(Camera& cam, QString& imagename, QTextStream& stream);

}
