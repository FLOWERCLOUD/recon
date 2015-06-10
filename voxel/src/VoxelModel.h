#pragma once

#include "morton_code.h"
#include <vectormath/utils.h>
#include <QtGlobal>

namespace recon {

using namespace vectormath::utils;

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

  vec3 minpt = vec3{
    (float)lerp(fx0, vbox.minpos, vbox.maxpos).x(),
    (float)lerp(fy0, vbox.minpos, vbox.maxpos).y(),
    (float)lerp(fz0, vbox.minpos, vbox.maxpos).z()
  };
  vec3 maxpt = vec3{
    (float)lerp(fx1, vbox.minpos, vbox.maxpos).x(),
    (float)lerp(fy1, vbox.minpos, vbox.maxpos).y(),
    (float)lerp(fz1, vbox.minpos, vbox.maxpos).z()
  };

  return AABox(minpt, maxpt);
}

}
