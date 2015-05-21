#pragma once

#include "VoxelData.h"
#include "AABB.h"
#include <vectormath.h>
#include <QByteArray>

namespace voxel {

using vectormath::aos::vec3;

typedef uint64_t Voxel;

class VoxelBlock {
public:
  VoxelBlock();
  ~VoxelBlock();

  void allocate(int w, int h, int d);
  void clear();

  int grid_width() const;

  void set_world(const AABB&);

  int width() const { return m_RealDim[0]; }
  int height() const { return m_RealDim[1]; }
  int depth() const { return m_RealDim[2]; }

  float world_width() const { return m_WorldSize[0]; }
  float world_height() const { return m_WorldSize[1]; }
  float world_depth() const { return m_WorldSize[2]; }

  float world_center_x() const { return m_WorldCenter[0]; }
  float world_center_y() const { return m_WorldCenter[1]; }
  float world_center_z() const { return m_WorldCenter[2]; }

  inline vec3 world_size() const
  {
    using vectormath::aos::load_vec3;
    return load_vec3(m_WorldSize);
  }

  inline vec3 world_center() const
  {
    using vectormath::aos::load_vec3;
    return load_vec3(m_WorldCenter);
  }

  inline vec3 map_to_world(int x, int y, int z);

  Voxel at(int x, int y, int z) const;
  Voxel& at(int x, int y, int z);

private:
  int m_RealDim[3];
  float m_WorldSize[3];
  float m_WorldCenter[3];
  VoxelData m_Data;
  QByteArray m_RawData;
};

inline int VoxelBlock::grid_width() const
{
  return m_Data.width;
}

inline vec3 VoxelBlock::map_to_world(int x, int y, int z)
{
  using vectormath::aos::make_vec3;
  vec3 pt = make_vec3((float)x / width(), (float)y / height(), (float)z / depth());
  pt = pt - make_vec3(0.5f, 0.5f, 0.5f);
  pt.xmm_data = _mm_mul_ps(pt.xmm_data, world_size().xmm_data);
  pt = pt + world_center();
  return pt;
}

inline Voxel VoxelBlock::at(int x, int y, int z) const
{
  return *((const Voxel*)m_Data.at(x, y, z));
}

inline Voxel& VoxelBlock::at(int x, int y, int z)
{
  return *((Voxel*)m_Data.at(x, y, z));
}

}
