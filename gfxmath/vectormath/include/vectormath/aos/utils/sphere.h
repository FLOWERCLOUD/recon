#pragma once

#include "point3.h"

namespace vectormath {
namespace aos {
namespace utils {

struct Sphere {
  Vec4 data;

  inline Sphere() = default;
  inline Sphere(const Sphere&) = default;
  inline Sphere& operator=(const Sphere&) = default;

  explicit inline Sphere(Point3 center, Float radius);

  inline Point3 center() const;
  inline Float radius() const;
};

inline Sphere::Sphere(Point3 center, Float radius)
: data((Vec3)center, radius)
{
}

inline Point3 Sphere::center() const
{
  return (Point3)Vec3::cast(data);
}

inline Float Sphere::radius() const
{
  return data.w();
}

}
}
}
