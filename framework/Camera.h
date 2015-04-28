#pragma once

#include <stdint.h>

namespace recon {

struct __declspec(align(16)) Camera {
  int32_t index;
  int32_t flag;
  int32_t _reserved[2];

  float focial_length;
  float pixel_aspect_ratio; // pixel_width / pixel_height
  float principal_point[2];
  float radial_distortion[2];
  float _padding[2];

  float extrinsic[12]; // 4x3 column-major extrinsic matrix
};



}
