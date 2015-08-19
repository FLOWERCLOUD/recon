#pragma once

#include <simdmath.h>
#include <math.h>

#ifndef M_PI
#  define M_PI 3.1415926535897932384626433832795
#endif

union float4_float {
  float4 vf;
  float f[4];
};
