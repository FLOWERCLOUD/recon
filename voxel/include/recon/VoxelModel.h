#pragma once

#include <vectormath.h>
#include <vectormath/aos/utils/aabox.h>
#include "morton_code.h"
#include <QList>
#include <QString>
#include <QtGlobal>
#include <stdint.h>
#include <math.h>
#include <utility>

namespace recon {

using vectormath::aos::Vec3;
using vectormath::aos::utils::Point3;
using vectormath::aos::utils::AABox;

typedef QList<uint64_t> VoxelList;

struct VoxelModel {
  uint16_t level;
  AABox real_box;
  AABox virtual_box;
  uint32_t width;
  uint32_t height;
  uint32_t depth;
  uint64_t morton_length;

  VoxelModel(uint16_t level, AABox model_box);

  inline AABox element_box(uint64_t morton) const;
};

void save_ply(const QString& path, const VoxelModel& model, const VoxelList& vlist);
void save_ply(const QString& path, const VoxelModel& model, const QList<uint32_t>& colors);

inline AABox VoxelModel::element_box(uint64_t morton) const
{
  Q_ASSERT(morton < morton_length);

  uint32_t x, y, z, w, h, d;
  morton_decode(morton, x, y, z);
  w = width, h = height, d = depth;

  float fx0, fy0, fz0, fx1, fy1, fz1;
  fx0 = (float)x / (float)w;
  fy0 = (float)y / (float)h;
  fz0 = (float)z / (float)d;
  fx1 = (float)(x+1) / (float)w;
  fy1 = (float)(y+1) / (float)h;
  fz1 = (float)(z+1) / (float)d;

  return AABox{
    virtual_box.lerp(fx0, fy0, fz0),
    virtual_box.lerp(fx1, fy1, fz1)
  };
}

}
