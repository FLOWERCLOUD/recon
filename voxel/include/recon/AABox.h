#pragma once

#include <vectormath.h>
#include <string.h>

namespace recon {

using vectormath::aos::vec3;

/* Axis-Aligned Bounding Box
 *
 */
struct AABox {
  float minpos[3];
  float maxpos[3];

  inline void fill(const float* pos);
  inline void add(const float* pos);

  inline void fill(vec3 pos)
  {
    pos.store(minpos);
    pos.store(maxpos);
  }

  inline void add(vec3 pos)
  {
    min_per_elem(vec3::load(minpos), pos).store(minpos);
    max_per_elem(vec3::load(maxpos), pos).store(maxpos);
  }

  inline vec3 get_minpos() const
  {
    return vec3::load(minpos);
  }

  inline vec3 get_maxpos() const
  {
    return vec3::load(maxpos);
  }

  inline vec3 get_center() const
  {
    return (get_minpos() + get_maxpos()) * 0.5f;
  }

  inline vec3 get_size() const
  {
    return get_maxpos() - get_minpos();
  }

  inline void set_size_center(vec3 size, vec3 center)
  {
    (center - size * 0.5f).store(minpos);
    (center + size * 0.5f).store(maxpos);
  }
};

inline void AABox::fill(const float* pos)
{
  memcpy(minpos, pos, sizeof(float)*3);
  memcpy(maxpos, pos, sizeof(float)*3);
}

inline void AABox::add(const float* pos)
{
  vec3 pt = vec3::load(pos);
  add(pt);
}

}
