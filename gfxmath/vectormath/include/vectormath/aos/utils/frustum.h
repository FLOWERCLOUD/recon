#pragma once

#include "plane.h"

namespace vectormath {
namespace aos {
namespace utils {

using namespace aos;

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
  inline explicit Frustum(Mat4 mat);

  inline bool contains(Point3) const;
};

inline Frustum::Frustum(Mat4 mat)
{
  Mat4 t = transpose(mat);
  left    = Plane{ t.column3() + t.column0() };
  right   = Plane{ t.column3() - t.column0() };
  bottom  = Plane{ t.column3() + t.column1() };
  top     = Plane{ t.column3() - t.column1() };
  near    = Plane{ t.column3() + t.column2() };
  far     = Plane{ t.column3() - t.column2() };
}

inline bool Frustum::contains(Point3 pt) const
{
  return (float)left[pt] > 0.0f &&
         (float)right[pt] > 0.0f &&
         (float)top[pt] > 0.0f &&
         (float)bottom[pt] > 0.0f &&
         (float)near[pt] > 0.0f &&
         (float)far[pt] > 0.0f;
}

}
}
}
