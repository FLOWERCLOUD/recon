#pragma once

#include "Plane.h"

namespace vectormath {
namespace utils {

struct Frustum {
  Plane left;
  Plane right;
  Plane top;
  Plane bottom;
  Plane near;
  Plane far;
  // Normal vectors of all planes above point to the inside halfspaces

  inline Frustum() = default;
  inline Frustum(const Frustum&) = default;
  inline Frustum& operator=(const Frustum&) = default;

  // If mat is a projection matrix P, gives clipping planes in view space
  // If mat is equal to V*P, gives clipping planes in model space
  inline explicit Frustum(mat4 mat);

  inline bool contains(vec3) const;
};

inline Frustum::Frustum(mat4 mat)
{
  mat4 t = transpose(mat);
  left    = Plane{ t.column3() + t.column0() };
  right   = Plane{ t.column3() - t.column0() };
  bottom  = Plane{ t.column3() + t.column1() };
  top     = Plane{ t.column3() - t.column1() };
  near    = Plane{ t.column3() + t.column2() };
  far     = Plane{ t.column3() - t.column2() };
}

inline bool Frustum::contains(vec3 pt) const
{
  return (float)left.dot(pt) > 0.0f &&
         (float)right.dot(pt) > 0.0f &&
         (float)top.dot(pt) > 0.0f &&
         (float)bottom.dot(pt) > 0.0f &&
         (float)near.dot(pt) > 0.0f &&
         (float)far.dot(pt) > 0.0f;
}

}
}
