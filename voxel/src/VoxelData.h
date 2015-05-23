#pragma once

#include "morton_code.h"

namespace recon {

struct VoxelData {
  uint64_t morton;
  uint32_t flag;
  uint32_t color;
};

inline bool operator<(const VoxelData& a, const VoxelData& b)
{
  return a.morton < b.morton;
}

inline bool operator>(const VoxelData& a, const VoxelData& b)
{
  return a.morton > b.morton;
}

}
