#pragma once

#include "AABox.h"
#include "morton_code.h"
#include <QtGlobal>
#include <functional>

namespace recon {

struct VoxelData {
  //uint64_t morton;
  uint32_t flag;
  uint32_t color;
};

struct VoxelBlock {
  uint64_t morton_begin;
  uint64_t morton_end;
  uint32_t origin[3];
  uint32_t width;
  AABox bounding_box;
  VoxelData* data;

  template<typename Func>
  void each_voxel(Func f);

  inline AABox voxelbox(uint64_t morton) const;
};

class VoxelBlockManager {
public:
  VoxelBlockManager(const AABox& modelBox,
                    int gridWidth,
                    int blockWidth);
  ~VoxelBlockManager();

  VoxelBlock* generate();
  void release(VoxelBlock* block);

private:
  int m_GridWidth;
  int m_BlockWidth;
  AABox m_GridBox;

  uint64_t m_CurrentMorton;
  uint64_t m_MortonStep;
  uint64_t m_MortonEnd;

  //int m_BlockUseCount;
  int m_BlockCapacity;

  union VoxelBlockNode {
    VoxelBlock live;
    VoxelBlockNode* next;
  };

  VoxelBlockNode* m_BlockPool;
  VoxelBlockNode* m_FreeBlock;

  VoxelData* m_VoxelMemory;
};

template<typename Func>
void VoxelBlock::each_voxel(Func f)
{
  VoxelData* ptr = data;

  for (uint64_t m = morton_begin; m < morton_end; ++m, ++ptr) {
    f(m, std::ref(*ptr));
  }
}

inline AABox VoxelBlock::voxelbox(uint64_t morton) const
{
  Q_ASSERT(morton >= morton_begin && morton < morton_end);

  uint32_t x, y, z;
  morton_decode(morton - morton_begin, x, y, z);

  vec3 vsiz = bounding_box.get_size() / (float)width;
  vec3 vpos = vec3((float)(x), (float)(y), (float)(z));
  vec3 vmin = bounding_box.get_minpos() + vsiz.x() * vpos;

  AABox vbox;
  vmin.store(vbox.minpos);
  (vmin + vsiz).store(vbox.maxpos);

  return vbox;
}

}
