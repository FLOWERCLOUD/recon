#pragma once

#include "point3.h"

namespace vectormath {
namespace aos {
namespace utils {

using namespace aos;

struct Plane {
  Vec4 coeff;

  inline Plane() = default;
  inline Plane(const Plane&) = default;
  inline Plane& operator=(const Plane&) = default;

  inline explicit Plane(Vec4 c);
  inline explicit Plane(Vec3 normal, Float dist);
  inline explicit Plane(Vec3 normal, Point3 pos);

  inline Float operator[](Point3 pt) const;

  //inline Mat4 reflection() const;
};

inline Plane::Plane(Vec4 c)
: coeff(c)
{
}

inline Plane::Plane(Vec3 normal, Float dist)
: coeff(normal, dist)
{
}

inline Plane::Plane(Vec3 normal, Point3 pos)
: coeff(normal, -dot(normal, pos.data))
{
}

inline Float Plane::operator[](Point3 pt) const
{
  return dot(coeff, Vec4(pt.data, 1.0f));
}

}
}
}
