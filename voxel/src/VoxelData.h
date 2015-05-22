#pragma once

#include "morton_code.h"

#include <QtGlobal>
#include <stdint.h>

namespace voxel {

/* Block of voxels
 *
 * A cubic block of voxels in Morton order
 *
 */
struct VoxelData {
  int width; // and the same as height, depth
  int stride;
  void* data; // size >= stride * pow(width, 3)

  inline const void* at(int x, int y, int z) const;
  inline void* at(int x, int y, int z);

  static inline uint64_t index_of(int x, int y, int z);
};

inline const void* VoxelData::at(int x, int y, int z) const
{
  Q_CHECK_PTR(data);
  Q_ASSERT(stride > 0);
  Q_ASSERT(x >= 0 && x < width);
  Q_ASSERT(y >= 0 && y < width);
  Q_ASSERT(z >= 0 && z < width);

  const char* ptr = (const char*)data;
  return (const void*)(ptr + (ptrdiff_t)stride * index_of(x, y, z));
}

inline void* VoxelData::at(int x, int y, int z)
{
  Q_CHECK_PTR(data);
  Q_ASSERT(stride > 0);
  Q_ASSERT(x >= 0 && x < width);
  Q_ASSERT(y >= 0 && y < width);
  Q_ASSERT(z >= 0 && z < width);

  char* ptr = (char*)data;
  return (void*)(ptr + (ptrdiff_t)stride * index_of(x, y, z));
}

inline uint64_t VoxelData::index_of(int x, int y, int z)
{
  return morton_encode_lookup(x, y, z);
}

}
