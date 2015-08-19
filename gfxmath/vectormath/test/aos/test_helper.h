#pragma once

#include <vectormath.h>
#include <math.h>

#ifndef M_PI
#  define M_PI 3.1415926535897932384626433832795
#endif

using vectormath::aos::Float;
using vectormath::aos::Vec3;
using vectormath::aos::Vec4;
using vectormath::aos::Quat;
using vectormath::aos::Mat3;
using vectormath::aos::Mat4;
using vectormath::aos::DQuat;

union f4_float {
  __m128 v;
  float f[4];
};

union Vec3_float {
  Vec3 v;
  float f[4];
};

union Vec4_float {
  Vec4 v;
  float f[4];
};

union Quat_float {
  Quat q;
  float f[4];
};

union Mat3_float {
  Mat3 m;
  float f[3][4];
};

union Mat4_float {
  Mat4 m;
  float f[4][4];
};
