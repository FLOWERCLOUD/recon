#pragma once

namespace vectormath {
namespace utils {

struct Plane {
  vec4 coeff;

  inline Plane() = default;
  inline Plane(const Plane&) = default;
  inline Plane& operator=(const Plane&) = default;

  explicit inline Plane(vec3 normal, float d);
  explicit inline Plane(vec3 normal, vec3 pt);
  explicit inline Plane(vec4);

  inline scalar dot(vec3 pt) const;
};

inline Plane::Plane(vec3 normal, float d)
: coeff(normal, d)
{
}

inline Plane::Plane(vec3 normal, vec3 pt)
: coeff(normal, (float)aos::dot(-normal, pt))
{
}

inline Plane::Plane(vec4 v)
: coeff(v)
{
}

inline scalar Plane::dot(vec3 pt) const
{
  return aos::dot(coeff, vec4(pt, 1.0f));
}

}
}
