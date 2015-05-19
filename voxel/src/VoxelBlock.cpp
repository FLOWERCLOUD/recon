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

}
