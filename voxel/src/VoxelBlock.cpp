#include "VoxelBlock.h"
#include "morton_code.h"
#include "Debug.h"
#include <math.h>
#include <string.h>

namespace recon {

VoxelBlockGenerator::VoxelBlockGenerator(const AABB& boundingbox)
: m_GridWidth(16)
, m_BlockWidth(256)
{
  // Setup Virtual Bounding Box
  float siz[3];
  boundingbox.get_size().store(siz);
  float maxsiz = fmaxf(siz[0], fmaxf(siz[1], siz[2]));

  m_VirtualBox.set_size_center(vec3(maxsiz, maxsiz, maxsiz), boundingbox.get_center());

  //qDebug() << "m_BoundingBox = " << m_BoundingBox;
  //qDebug() << "m_VirtualBox = " << m_VirtualBox;

  // Setup dimensions and ending morton
  int bwidth = m_BlockWidth;
  int total_width = m_GridWidth * bwidth;
  m_NumVoxelsPerBlock = (uint64_t)bwidth * bwidth * bwidth;
  m_MortonEnd = morton_encode(total_width-1, total_width-1, total_width-1)+1;
  m_MortonStep = morton_encode(bwidth-1, bwidth-1, bwidth-1)+1;

  //qDebug() << "total_width = " << total_width;
  //qDebug() << "m_NumVoxelsPerBlock = " << m_NumVoxelsPerBlock;
  //qDebug() << "m_MortonEnd = " << m_MortonEnd;
  //qDebug() << "m_MortonStep = " << m_MortonStep;
  //{
  //  uint32_t x, y, z;
  //  morton_decode(m_MortonStep-1, x, y, z);
  //  qDebug() << "decode => " << x << ", " << y << ", " << z;
  //}

  // Set starting morton
  m_CurrentMorton = 0;
}

VoxelBlockGenerator::~VoxelBlockGenerator()
{
}

bool VoxelBlockGenerator::generate(VoxelBlock& block)
{
  uint64_t morton = m_CurrentMorton;
  if (morton >= m_MortonEnd)
    return false;
  m_CurrentMorton += m_MortonStep;

  uint32_t x, y, z;
  morton_decode(morton, x, y, z);

  int gridsiz = m_GridWidth;
  int blksiz = m_BlockWidth;
  AABB bbox;
  vec3 bsiz = m_VirtualBox.get_size() / (float)gridsiz;
  vec3 bpos_gridspace = vec3((float)(x/blksiz), (float)(y/blksiz), (float)(z/blksiz));
  vec3 bmin = m_VirtualBox.get_minpos() + bsiz.x() * bpos_gridspace;
  bmin.store(bbox.minpos);
  (bmin + bsiz).store(bbox.maxpos);

  //qDebug() << "Block size = " << bsiz;
  //qDebug() << "Block in grid = " << bpos_gridspace;

  block.morton_begin = morton;
  block.morton_end = morton + m_MortonStep;
  block.origin[0] = x;
  block.origin[1] = y;
  block.origin[2] = z;
  block.width = m_BlockWidth;
  block.bbox = bbox;
  block.data.resize(m_NumVoxelsPerBlock);

  for (int i = 0; i < block.data.size(); ++i) {
    VoxelData& v = block.data[i];
    v.morton = morton + i;
    v.flag = 0;
    v.color = 0;
  }

  return true;
}

}
