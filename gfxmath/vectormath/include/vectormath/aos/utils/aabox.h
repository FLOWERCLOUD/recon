#pragma once

#include "point3.h"

namespace vectormath {
namespace aos {
namespace utils {

using namespace aos;

struct AABox {
  Point3 minpos;
  Point3 maxpos;

  inline AABox() = default;
  inline AABox(const AABox&) = default;
  inline AABox& operator=(const AABox&) = default;

  inline explicit AABox(Point3 pt);
  inline explicit AABox(Point3 pt1, Point3 pt2);

  inline void add(Point3 pt);
  inline Point3 center() const;
  inline Vec3 extent() const;

  inline Point3 corner0() const; // (0, 0, 0)
  inline Point3 corner1() const; // (1, 0, 0)
  inline Point3 corner2() const; // (0, 1, 0)
  inline Point3 corner3() const; // (1, 1, 0)
  inline Point3 corner4() const; // (0, 0, 1)
  inline Point3 corner5() const; // (1, 0, 1)
  inline Point3 corner6() const; // (0, 1, 1)
  inline Point3 corner7() const; // (1, 1, 1)

  inline Point3 lerp(float x, float y, float z) const;
};

inline AABox::AABox(Point3 pt)
: minpos(pt), maxpos(pt)
{
}

inline AABox::AABox(Point3 pt1, Point3 pt2)
: minpos(min(pt1.data, pt2.data)), maxpos(max(pt1.data, pt2.data))
{
}

inline void AABox::add(Point3 pt)
{
  minpos = (Point3)min(minpos.data, pt.data);
  maxpos = (Point3)max(maxpos.data, pt.data);
}

inline Point3 AABox::center() const
{
  return Point3(minpos.data * 0.5f + maxpos.data * 0.5f);
}

inline Vec3 AABox::extent() const
{
  return abs(maxpos - minpos);
}

inline Point3 AABox::corner0() const
{
  return minpos;
}

inline Point3 AABox::corner1() const
{
  return (Point3)copy_x(minpos.data, maxpos.data);
}

inline Point3 AABox::corner2() const
{
  return (Point3)copy_y(minpos.data, maxpos.data);
}

inline Point3 AABox::corner3() const
{
  return (Point3)copy_z(maxpos.data, minpos.data);
}

inline Point3 AABox::corner4() const
{
  return (Point3)copy_z(minpos.data, maxpos.data);
}

inline Point3 AABox::corner5() const
{
  return (Point3)copy_y(maxpos.data, minpos.data);
}

inline Point3 AABox::corner6() const
{
  return (Point3)copy_x(maxpos.data, minpos.data);
}

inline Point3 AABox::corner7() const
{
  return maxpos;
}

inline Point3 AABox::lerp(float x, float y, float z) const
{
  // TODO: refactor to lerp(Vec3, Vec3, Vec3) ?
  __m128 t = _mm_setr_ps(x, y, z, 0.0f);
  __m128 v0 = minpos.data.xmm_data, v1 = maxpos.data.xmm_data;
  __m128 result = _mm_add_ps(v0, _mm_mul_ps(t, _mm_sub_ps(v1, v0)));
  return Point3(Vec3(result));
}

}
}
}
