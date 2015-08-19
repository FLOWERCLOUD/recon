#pragma once

namespace vectormath {
namespace aos {
namespace utils {

using namespace aos;

struct Point3 {
  Vec3 data;

  inline Point3() noexcept = default;
  inline Point3(const Point3&) noexcept = default;
  inline Point3& operator=(const Point3&) noexcept = default;

  explicit inline Point3(Vec3) noexcept;
  explicit inline Point3(float, float, float) noexcept;
  inline explicit operator Vec3() const noexcept;

  static inline Point3 zero() noexcept;

  static inline Point3 load(const float*) noexcept;
  inline void store(float*) const noexcept;

  inline PFloat x() const noexcept;
  inline PFloat y() const noexcept;
  inline PFloat z() const noexcept;
};

inline Point3 operator+(Point3, Vec3) noexcept;
inline Point3 operator-(Point3, Vec3) noexcept;
inline Vec3 operator-(Point3, Point3) noexcept;
inline Vec3 transform(Mat3, Point3) noexcept;
inline Vec4 transform(Mat4, Point3) noexcept;

// MARK: implementation

inline Point3::Point3(Vec3 a) noexcept
: data(a)
{
}

inline Point3::Point3(float x, float y, float z) noexcept
: Point3(Vec3(x, y, z))
{
}

inline Point3::operator Vec3() const noexcept
{
  return data;
}

inline Point3 Point3::zero() noexcept
{
  return Point3{ Vec3::zero() };
}

inline Point3 Point3::load(const float* pxyz) noexcept
{
  return Point3{ Vec3::load(pxyz) };
}

inline void Point3::store(float* pxyz) const noexcept
{
  data.store(pxyz);
}

inline PFloat Point3::x() const noexcept
{
  return data.x();
}

inline PFloat Point3::y() const noexcept
{
  return data.y();
}

inline PFloat Point3::z() const noexcept
{
  return data.z();
}

inline Point3 operator+(Point3 pt, Vec3 d) noexcept
{
  return Point3{ pt.data + d };
}

inline Point3 operator-(Point3 pt, Vec3 d) noexcept
{
  return Point3{ pt.data - d };
}

inline Vec3 operator-(Point3 pt0, Point3 pt1) noexcept
{
  return pt0.data - pt1.data;
}

inline Vec3 transform(Mat3 m, Point3 p) noexcept
{
  return m * p.data;
}

inline Vec4 transform(Mat4 m, Point3 p) noexcept
{
  return m * Vec4(p.data, 1.0f);
}

}
}
}
