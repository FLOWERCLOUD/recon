#pragma once

#include <vectormath.h>
#include <vectormath/aos/utils/point3.h>

#include <QtDebug>
#include <ostream>

namespace recon {

using vectormath::aos::utils::Vec3;
using vectormath::aos::utils::Vec4;
using vectormath::aos::utils::Mat4;
using vectormath::aos::utils::Point3;

}

inline std::ostream& operator<<(std::ostream& ost, const recon::Vec3& v)
{
  ost << "Vec3("
      << (float)v.x() << ", "
      << (float)v.y() << ", "
      << (float)v.z() << ")";
  return ost;
}

inline std::ostream& operator<<(std::ostream& ost, const recon::Vec4& v)
{
  ost << "Vec3("
      << (float)v.x() << ", "
      << (float)v.y() << ", "
      << (float)v.z() << ", "
      << (float)v.w() << ")";
  return ost;
}

inline std::ostream& operator<<(std::ostream& ost, const recon::Point3& v)
{
  ost << "Vec3("
      << (float)v.x() << ", "
      << (float)v.y() << ", "
      << (float)v.z() << ")";
  return ost;
}
