#pragma once

#include <QtGlobal>
#include <stdint.h>

namespace voxel {

inline bool is_pow2(uint32_t a)
{
  return (a & (a-1)) == 0;
}

inline uint32_t to_pow2(uint32_t a)
{
  if (is_pow2(a))
    return a;

  uint32_t x = a;
  while (x & (x-1))
    x = x & (x-1);

  return x << 1;
}

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

//===================================================================
//
// Convert XYZ to Morton Index
//
// Reference: http://www.forceflow.be/2013/10/07/morton-encodingdecoding-through-bit-interleaving-implementations/
//

static inline uint64_t _morton_split(uint32_t a)
{
  uint64_t x = a & 0x1FFFFF;
  x = (x | x << 32) & 0x001F00000000FFFFull;
  x = (x | x << 16) & 0x001F0000FF0000FFull;
  x = (x | x <<  8) & 0x100F00F00F00F00Full;
  x = (x | x <<  4) & 0x10C30C30C30C30C3ull;
  x = (x | x <<  2) & 0x1249249249249249ull;
  return x;
}

static inline uint64_t _morton_encode_magicbits(uint32_t x, uint32_t y, uint32_t z)
{
  uint64_t result;
  result  = _morton_split(x);
  result |= _morton_split(y) << 1;
  result |= _morton_split(z) << 2;
  return result;
}

inline uint64_t VoxelData::index_of(int x, int y, int z)
{
  return _morton_encode_magicbits(x, y, z);
}

}
