#pragma once

namespace vectormath {
namespace utils {

struct AABox {
  vec3 minpos;
  vec3 maxpos;

  inline AABox() = default;
  inline AABox(const AABox&) = default;
  inline AABox& operator=(const AABox&) = default;

  inline explicit AABox(vec3 pt);
  inline explicit AABox(vec3 pt1, vec3 pt2);

  inline void add(vec3 pt);
  inline vec3 center() const;
  inline vec3 size() const;
};

inline AABox::AABox(vec3 pt)
: minpos(pt), maxpos(pt)
{
}

inline AABox::AABox(vec3 pt1, vec3 pt2)
: minpos(min(pt1, pt2)), maxpos(max(pt1, pt2))
{
}

inline void AABox::add(vec3 pt)
{
  minpos = min(minpos, pt);
  maxpos = max(maxpos, pt);
}

inline vec3 AABox::center() const
{
  return lerp(0.5f, minpos, maxpos);
}

inline vec3 AABox::size() const
{
  return abs(maxpos - minpos);
}

}
}
