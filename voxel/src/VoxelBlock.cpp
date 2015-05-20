#include "VoxelBlock.h"

#include <stddef.h>

namespace voxel {

VoxelBlock::VoxelBlock()
{
  memset(m_RealDim, 0, sizeof(int)*3);
  memset(&m_Data, 0, sizeof(VoxelData));
}

VoxelBlock::~VoxelBlock()
{
  clear();
}

void VoxelBlock::allocate(int w, int h, int d)
{
  m_RealDim[0] = w;
  m_RealDim[1] = h;
  m_RealDim[2] = d;

  int v = std::max(w, std::max(h, d));
  v = to_pow2(v);

  size_t siz = sizeof(Voxel) * v * v * v;
  m_RawData.resize(siz);
  m_RawData.fill(0);

  m_Data.width = v;
  m_Data.stride = sizeof(Voxel);
  m_Data.data = m_RawData.data();
}

void VoxelBlock::clear()
{
  memset(m_RealDim, 0, sizeof(int)*3);
  memset(&m_Data, 0, sizeof(VoxelData));
  m_RawData.clear();
}

void VoxelBlock::set_world(const AABB& bounding)
{
  using vectormath::aos::load_vec3;
  using vectormath::aos::make_scalar;
  vec3 minpos = bounding.get_minpos();
  vec3 maxpos = bounding.get_maxpos();
  vec3 center = lerp(make_scalar(0.5f), minpos, maxpos);
  vec3 size = abs(maxpos - minpos);
  store_vec3(m_WorldSize, size);
  store_vec3(m_WorldCenter, center);
}

}
