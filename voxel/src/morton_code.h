#pragma once

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

//===================================================================
//
// Convert XYZ to Morton Index
//
// Reference: http://www.forceflow.be/2013/10/07/morton-encodingdecoding-through-bit-interleaving-implementations/
//

uint64_t morton_encode_magicbits(uint32_t x, uint32_t y, uint32_t z);
uint64_t morton_encode_lookup(uint32_t x, uint32_t y, uint32_t z);

}
