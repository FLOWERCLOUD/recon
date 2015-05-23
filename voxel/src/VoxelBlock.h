#pragma once

#include "AABB.h"
#include "VoxelData.h"
#include "morton_code.h"
#include <QVector>

namespace recon {

struct VoxelBlock {
  uint64_t morton_begin;
  uint64_t morton_end;
  uint32_t origin[3];
  uint32_t width;
  AABB bbox;
  QVector<VoxelData> data;

  AABB voxelbox(uint64_t morton) const;
  const VoxelData& voxeldata(uint64_t morton) const;
  VoxelData& voxeldata(uint64_t morton);

  template<typename Func>
  void each_voxel(Func f);
};

class VoxelBlockGenerator {
public:
  VoxelBlockGenerator(const AABB& boundingbox);
  ~VoxelBlockGenerator();

  bool generate(VoxelBlock& block);

private:
  //AABB m_BoundingBox;
  AABB m_VirtualBox;
  int m_GridWidth;
  int m_BlockWidth;

  size_t m_NumVoxelsPerBlock;
  uint64_t m_MortonEnd;
  uint64_t m_MortonStep;

  uint64_t m_CurrentMorton;
};

inline AABB VoxelBlock::voxelbox(uint64_t morton) const
{
  Q_ASSERT(morton >= morton_begin && morton < morton_end);

  uint32_t x, y, z;
  morton_decode(morton - morton_begin, x, y, z);

  vec3 vsiz = bbox.get_size() / (float)width;
  vec3 vpos = vec3((float)(x), (float)(y), (float)(z));
  vec3 vmin = bbox.get_minpos() + vsiz.x() * vpos;

  AABB vbox;
  vmin.store(vbox.minpos);
  (vmin + vsiz).store(vbox.maxpos);

  return vbox;
}

inline const VoxelData& VoxelBlock::voxeldata(uint64_t morton) const
{
  Q_ASSERT(morton >= morton_begin && morton < morton_end);
  return data[morton - morton_begin];
}

inline VoxelData& VoxelBlock::voxeldata(uint64_t morton)
{
  Q_ASSERT(morton >= morton_begin && morton < morton_end);
  return data[morton - morton_begin];
}

template<typename Func>
inline void VoxelBlock::each_voxel(Func f)
{
  //vec3 vsiz = bbox.get_size() / (float)width;
  //vec3 vorigin = bbox.get_minpos();

  for (uint64_t m = morton_begin; m < morton_end; ++m) {
    //uint32_t x, y, z;
    //morton_decode(m - morton_begin, x, y, z);
    f(m, this);
  }
}

}
