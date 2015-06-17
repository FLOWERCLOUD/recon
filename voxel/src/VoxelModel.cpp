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

bool VoxelModel::intersects(uint64_t& morton, const Ray& inray) const
{
  // Use Digital Differential Analysis
  // See also: Bresenham's Line Drawing Algorithm
  AABox vbox = m_VoxelBox;
  vec3 vextent = vbox.extent();

  // relative to voxel space
  Ray ray = inray.relative_to(vbox.minpos);
  // make vbox be relative to voxel space
  vbox.maxpos = vbox.maxpos - vbox.minpos;
  vbox.minpos = vec3::zero();

  // determine which axis to walk along
  float adx = fabsf((float)ray.direction.x());
  float ady = fabsf((float)ray.direction.y());
  float adz = fabsf((float)ray.direction.z());
  float t0, t1, resolution;

  if (adx >= ady && adx >= adz) {
    // along x-axis
    t0 = ray.proj_x((float)vbox.minpos.x());
    t1 = ray.proj_x((float)vbox.maxpos.x());
    resolution = m_Width;
  } else if (ady >= adz) {
    // along y-axis
    t0 = ray.proj_y((float)vbox.minpos.y());
    t1 = ray.proj_y((float)vbox.maxpos.y());
    resolution = m_Height;
  } else {
    // along z-axis
    t0 = ray.proj_z((float)vbox.minpos.z());
    t1 = ray.proj_z((float)vbox.maxpos.z());
    resolution = m_Depth;
  }

  if ((t0 < 0.0f && t1 < 0.0f)) {
    qDebug("out of sight");
    return false;
  }
  if (t0 > t1)
    std::swap(t0, t1);
  float tstep = (t1 - t0) / resolution;

  //t0 = fmaxf(t0, 0.0f);
  for (; t0 < t1; t0 += tstep) {
    if (t0 < 0.0f)
      continue;

    vec3 pt = ray[t0] - vbox.minpos;
    int ix = (float)pt.x() / (float)vextent.x() * (float)m_Width;
    int iy = (float)pt.y() / (float)vextent.y() * (float)m_Height;
    int iz = (float)pt.z() / (float)vextent.z() * (float)m_Depth;
    if (ix < 0 || iy < 0 || iz < 0)
      continue;
    if (ix >= m_Width || iy >= m_Height || iz >= m_Depth)
      continue;

    uint64_t m = morton_encode(ix, iy, iz);
    const VoxelData& voxel = operator[](m);
    if (voxel.flag) {
      morton = m;
      return true;
    }
  }

  return false;
}

bool VoxelModel::check_visibility(vec3 eyepos, uint64_t morton) const
{
  AABox vbox = boundingbox(morton);

  uint32_t x, y, z;
  morton_decode(morton, x, y, z);

  uint64_t hitmorton;
  if (intersects(hitmorton, Ray::from_points(eyepos, vbox.center()))) {
    if (hitmorton != morton) {
      // Tolerance
      uint32_t ix, iy, iz;
      morton_decode(hitmorton, ix, iy, iz);
      int dx, dy, dz;
      dx = abs((int)ix - (int)x);
      dy = abs((int)iy - (int)y);
      dz = abs((int)iz - (int)z);
      if ((dx + dy + dz) > 1) {
        return false;
      }
    }

    return true;
  }

  return false;
}

}
