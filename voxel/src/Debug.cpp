#include "Debug.h"
#include <QDebug>

QDebug& operator<<(QDebug& dbg, const vectormath::aos::vec3& v)
{
  dbg.nospace() << "("  << (float)v.x()
                << ", " << (float)v.y()
                << ", " << (float)v.z()
                <<  ")";
  return dbg.space();
}

QDebug& operator<<(QDebug& dbg, const recon::AABB& aabb)
{
  dbg.nospace() << "AABB[min=" << aabb.get_minpos()
                << ", max=" << aabb.get_maxpos()
                << "]";
  return dbg.space();
}
