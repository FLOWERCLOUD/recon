#pragma once

#include "point3.h"

namespace vectormath {
namespace aos {
namespace utils {

using namespace aos;

struct Ray3 {
  Point3 start;
  Vec3 diff;

  inline Ray3() = default;
  inline Ray3(const Ray3&) = default;
  inline Ray3& operator=(const Ray3&) = default;

  explicit inline Ray3(Point3, Vec3);
  static inline Ray3 make(Point3, Point3);

  inline Point3 operator[](float t) const;

  inline Float projection(Point3) const;
};

inline Ray3::Ray3(Point3 pt0, Vec3 d)
: start(pt0), diff(d)
{
}

inline Ray3 Ray3::make(Point3 pt0, Point3 pt1)
{
  return Ray3(pt0, normalize(pt1-pt0));
}

inline Point3 Ray3::operator[](float t) const
{
  return start + diff * t;
}

inline Float Ray3::projection(Point3 pt) const
{
  return dot(diff, (pt - start));
}

}
}
}
