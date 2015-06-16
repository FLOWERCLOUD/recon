#pragma once

#include "morton_code.h"
#include <vectormath.h>
#include <vectormath/aos/utils/aabox.h>
#include <vectormath/aos/utils/ray.h>
#include <math.h>
#include <utility>
#include <QtGlobal>

namespace recon {

using vectormath::aos::vec3;
using vectormath::aos::vec4;
using vectormath::aos::mat3;
using vectormath::aos::mat4;
using vectormath::aos::utils::AABox;
using vectormath::aos::utils::Ray;

struct VoxelData {
  enum Flag {
    VISUALHULL_FLAG = 0x1,
    SURFACE_FLAG = 0x2,
  };

  uint32_t flag;
  uint32_t color;
};

class VoxelModel {
public:
  VoxelModel(uint32_t level, const AABox& bbox);
  ~VoxelModel();

  inline uint64_t size() const;
  inline const VoxelData& operator[](uint64_t morton) const;
  inline VoxelData& operator[](uint64_t morton);

  const VoxelData* get(uint32_t, uint32_t, uint32_t) const;
  VoxelData* get(uint32_t, uint32_t, uint32_t);

  inline AABox boundingbox() const;
  inline AABox boundingbox(uint64_t morton) const;

  //static uint64_t encode(uint32_t, uint32_t, uint32_t);
  //static void decode(uint64_t, uint32_t&, uint32_t&, uint32_t&);

  inline bool intersects(uint64_t& morton, const Ray&) const;

private:
  uint32_t m_Level;
  uint32_t m_Width;
  uint32_t m_Height;
  uint32_t m_Depth;
  uint64_t m_MortonLength;
  AABox m_ModelBox;
  AABox m_VoxelBox;
  VoxelData* m_Data;
};

inline uint64_t VoxelModel::size() const
{
  return m_MortonLength;
}

inline const VoxelData& VoxelModel::operator[](uint64_t morton) const
{
  Q_ASSERT(morton < m_MortonLength);
  return m_Data[morton];
}

inline VoxelData& VoxelModel::operator[](uint64_t morton)
{
  Q_ASSERT(morton < m_MortonLength);
  return m_Data[morton];
}

inline AABox VoxelModel::boundingbox() const
{
  return m_VoxelBox;
}

inline AABox VoxelModel::boundingbox(uint64_t morton) const
{
  Q_ASSERT(morton < m_MortonLength);

  uint32_t x, y, z, w, h, d;
  morton_decode(morton, x, y, z);
  w = m_Width;
  h = m_Height;
  d = m_Depth;

  float fx0, fy0, fz0, fx1, fy1, fz1;
  fx0 = (float)x / (float)w;
  fy0 = (float)y / (float)h;
  fz0 = (float)z / (float)d;
  fx1 = (float)(x+1) / (float)w;
  fy1 = (float)(y+1) / (float)h;
  fz1 = (float)(z+1) / (float)d;

  AABox vbox = m_VoxelBox;

  return AABox{
    vbox.lerp(fx0, fy0, fz0),
    vbox.lerp(fx1, fy1, fz1)
  };
}

inline bool VoxelModel::intersects(uint64_t& morton, const Ray& inray) const
{
  // Use Digital Differential Analysis
  // See also: Bresenham's Line Drawing Algorithm
  AABox vbox = m_VoxelBox;
  vec3 vextent = vbox.extent();

  // relative to voxel space
  Ray ray = inray.relative_to(vbox.minpos);
  // make vbox be relative to voxel space
  vbox.maxpos = vbox.maxpos - vbox.minpos;
  vbox.minpos = vec3::zero();

  // determine which axis to walk along
  float adx = fabsf((float)ray.direction.x());
  float ady = fabsf((float)ray.direction.y());
  float adz = fabsf((float)ray.direction.z());
  float t0, t1, resolution;

  if (adx >= ady && adx >= adz) {
    // along x-axis
    t0 = ray.proj_x((float)vbox.minpos.x());
    t1 = ray.proj_x((float)vbox.maxpos.x());
    resolution = m_Width;
  } else if (ady >= adz) {
    // along y-axis
    t0 = ray.proj_y((float)vbox.minpos.y());
    t1 = ray.proj_y((float)vbox.maxpos.y());
    resolution = m_Height;
  } else {
    // along z-axis
    t0 = ray.proj_z((float)vbox.minpos.z());
    t1 = ray.proj_z((float)vbox.maxpos.z());
    resolution = m_Depth;
  }

  if ((t0 < 0.0f && t1 < 0.0f)) {
    qDebug("out of sight");
    return false;
  }
  if (t0 > t1)
    std::swap(t0, t1);
  float tstep = (t1 - t0) / resolution;

  //t0 = fmaxf(t0, 0.0f);
  for (; t0 < t1; t0 += tstep) {
    if (t0 < 0.0f)
      continue;

    vec3 pt = ray[t0] - vbox.minpos;
    int ix = (float)pt.x() / (float)vextent.x() * (float)m_Width;
    int iy = (float)pt.y() / (float)vextent.y() * (float)m_Height;
    int iz = (float)pt.z() / (float)vextent.z() * (float)m_Depth;
    if (ix < 0 || iy < 0 || iz < 0)
      continue;
    if (ix >= m_Width || iy >= m_Height || iz >= m_Depth)
      continue;

    uint64_t m = morton_encode(ix, iy, iz);
    const VoxelData& voxel = operator[](m);
    if (voxel.flag) {
      morton = m;
      return true;
    }
  }

  return false;
}

}
