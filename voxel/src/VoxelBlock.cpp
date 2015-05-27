#include "VoxelBlock.h"
#include "morton_code.h"
#include <stdlib.h>
#include <math.h>

namespace recon {

VoxelBlockManager::VoxelBlockManager(const AABox& modelBox,
                                     int gridWidth,
                                     int blockWidth)
: m_GridWidth(gridWidth)
, m_BlockWidth(blockWidth)
, m_CurrentMorton(0)
, m_MortonStep(0)
, m_MortonEnd(0)
//, m_BlockUseCount(0)
, m_BlockCapacity(0)
, m_BlockPool(NULL)
, m_FreeBlock(NULL)
, m_VoxelMemory(NULL)
{
  float siz[3];
  modelBox.get_size().store(siz);
  float maxsiz = fmaxf(siz[0], fmaxf(siz[1], siz[2]));
  m_GridBox.set_size_center(vec3(maxsiz, maxsiz, maxsiz), modelBox.get_center());

  int bwidth = m_BlockWidth;
  int total_width = m_GridWidth * bwidth;
  m_MortonEnd = morton_encode(total_width-1, total_width-1, total_width-1)+1;
  m_MortonStep = morton_encode(bwidth-1, bwidth-1, bwidth-1)+1;

  int cap = 16;
  m_BlockPool = (VoxelBlockNode*)malloc(sizeof(VoxelBlockNode) * cap);
  m_VoxelMemory = (VoxelData*)malloc(sizeof(VoxelData) * m_MortonStep * cap);
  m_BlockCapacity = cap;

  memset(m_BlockPool, 0, sizeof(VoxelBlockNode) * cap);

  for (int i = 0; i < cap-1; ++i) {
    m_BlockPool[i].next = m_BlockPool + (i+1);
  }
  m_BlockPool[cap-1].next = NULL;
  m_FreeBlock = m_BlockPool;
}

VoxelBlockManager::~VoxelBlockManager()
{
  free(m_BlockPool);
  free(m_VoxelMemory);
}

VoxelBlock* VoxelBlockManager::generate()
{
  if (m_CurrentMorton >= m_MortonEnd)
    return NULL;
  if (m_FreeBlock == NULL)
    return NULL;

  // Pop from free list
  VoxelBlockNode* node = m_FreeBlock;
  m_FreeBlock = node->next;

  // Iterate the current morton code
  uint64_t morton = m_CurrentMorton;
  m_CurrentMorton += m_MortonStep;

  // Get memory address of voxel data
  VoxelData* data = m_VoxelMemory + m_MortonStep * (node - m_BlockPool);

  uint32_t x, y, z;
  morton_decode(morton, x, y, z);

  // Compute bounding box
  int gridsiz = m_GridWidth;
  int blksiz = m_BlockWidth;
  AABox bbox;
  vec3 bsiz = m_GridBox.get_size() / (float)gridsiz;
  vec3 bpos_gridspace = vec3((float)(x/blksiz), (float)(y/blksiz), (float)(z/blksiz));
  vec3 bmin = m_GridBox.get_minpos() + bsiz.x() * bpos_gridspace;
  bmin.store(bbox.minpos);
  (bmin + bsiz).store(bbox.maxpos);

  // Initialize voxel block
  VoxelBlock* block = (VoxelBlock*)node;
  block->morton_begin = morton;
  block->morton_end = morton + m_MortonStep;
  block->origin[0] = x;
  block->origin[1] = y;
  block->origin[2] = z;
  block->width = blksiz;
  block->bounding_box = bbox;
  block->data = data;

  return block;
}

void VoxelBlockManager::release(VoxelBlock* block)
{
  VoxelBlockNode* node = (VoxelBlockNode*)((void*)block);
  node->next = m_FreeBlock;
  m_FreeBlock = node;
}

}
