#pragma once

namespace vectormath {
namespace aos {
namespace utils {

struct Ray {
  vec3 startpos;
  vec3 direction;

  inline Ray() = default;
  inline Ray(const Ray&) = default;
  inline Ray& operator=(const Ray&) = default;

  inline explicit Ray(vec3 pt0, vec3 unitvec);
  static inline Ray from_points(vec3, vec3);

  inline vec3 operator[](float t) const;

  inline scalar projection(vec3) const;
  inline float proj_x(float) const;
  inline float proj_y(float) const;
  inline float proj_z(float) const;

  inline Ray relative_to(vec3) const;
};

inline Ray::Ray(vec3 pt0, vec3 unitvec)
: startpos(pt0), direction(unitvec)
{
}

inline Ray Ray::from_points(vec3 pt0, vec3 pt1)
{
  vec3 dir = normalize(pt1 - pt0);
  return Ray{ pt0, dir };
}

inline vec3 Ray::operator[](float t) const
{
  return startpos + direction * t;
}

inline scalar Ray::projection(vec3 pt) const
{
  return dot(direction, (pt - startpos));
}

inline float Ray::proj_x(float x) const
{
  return (x - (float)startpos.x()) / (float)direction.x();
}

inline float Ray::proj_y(float y) const
{
  return (y - (float)startpos.y()) / (float)direction.y();
}

inline float Ray::proj_z(float z) const
{
  return (z - (float)startpos.z()) / (float)direction.z();
}

inline Ray Ray::relative_to(vec3 pt) const
{
  return Ray{ startpos - pt, direction };
}

}
}
}
