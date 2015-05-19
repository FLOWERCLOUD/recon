#pragma once

#include "VoxelData.h"
#include <QByteArray>

namespace voxel {

typedef uint64_t Voxel;

class VoxelBlock {
public:
  VoxelBlock();
  ~VoxelBlock();

  void allocate(int w, int h, int d);
  void clear();

  int grid_width() const;

  //const Voxel& at(int x, int y, int z);
  //Voxel& at(int x, int y, int z);

private:
  int m_RealDim[3];
  VoxelData m_Data;
  QByteArray m_RawData;
};

inline int VoxelBlock::grid_width() const
{
  return m_Data.width;
}

}
