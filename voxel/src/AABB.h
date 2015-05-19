#pragma once

#include <string.h>

namespace voxel {

/* Axis-Aligned Bounding Box
 *
 */
struct AABB {
  float minpos[3];
  float maxpos[3];

  inline void fill(const float* pos);
  inline void add(const float* pos);
};

inline void AABB::fill(const float* pos)
{
  memcpy(minpos, pos, sizeof(float)*3);
  memcpy(maxpos, pos, sizeof(float)*3);
}

inline void AABB::add(const float* pos)
{
  using vectormath::aos::load_vec3;
  vec3 pt = load_vec3(pos);

  store_vec3(minpos, min_per_elem(load_vec3(minpos), pt));
  store_vec3(maxpos, max_per_elem(load_vec3(maxpos), pt));
}

}
