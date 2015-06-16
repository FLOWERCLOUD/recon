#include "VoxelModel.h"
#include "morton_code.h"
#include <QtGlobal>
#include <math.h>
#include <stdlib.h>

namespace recon {

VoxelModel::VoxelModel(uint32_t level, const AABox& bbox)
: m_Level(level)
, m_Width(0x1 << level)
, m_Height(0x1 << level)
, m_Depth(0x1 << level)
, m_MortonLength(0x1ull << level*3)
, m_ModelBox(bbox)
, m_VoxelBox()
, m_Data(NULL)
{
  if (level > 20) {
    qFatal("%s:%d: level is too high (level = %d)", __FILE__, __LINE__, level);
  }

  // compute m_VoxelBox
  {
    float siz[3], vsiz;
    m_ModelBox.extent().store(siz);
    vsiz = fmaxf(siz[0], fmaxf(siz[1], siz[2]));

    vec3 start = m_ModelBox.minpos, extent = vec3(vsiz, vsiz, vsiz);
    m_VoxelBox = AABox(start, start + extent);
  }

  // allocate voxel data
  {
    void *ptr;
    ptr = malloc(sizeof(VoxelData) * m_MortonLength);
    if (!ptr) {
      qFatal("%s:%d: failed to allocate voxel data", __FILE__, __LINE__);
    }
    m_Data = (VoxelData*)ptr;
  }
}

VoxelModel::~VoxelModel()
{
  // free voxel data
  free(m_Data);
}

const VoxelData* VoxelModel::get(uint32_t x, uint32_t y, uint32_t z) const
{
  uint64_t morton = morton_encode(x, y, z);
  if (morton >= size())
    return NULL;
  return &(operator[](morton));
}

VoxelData* VoxelModel::get(uint32_t x, uint32_t y, uint32_t z)
{
  uint64_t morton = morton_encode(x, y, z);
  if (morton >= size())
    return NULL;
  return &(operator[](morton));
}

}
