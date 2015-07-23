#pragma once

#include "morton_code.h"
#include <QByteArray>
#include <QVector>
#include <QString>

namespace recon {

struct SkeletonField {
  int level;
  QList<uint64_t> skeleton;
  QVector<float> field;

  explicit SkeletonField(int lv);

  void computeField();

  // read from PFSkel
  //
  // Computing Hierarchical Curve-Skeletons of 3D Objects
  // NICU D. CORNEA, DEBORAH SILVER, XIAOSONG YUAN, AND RAMAN BALASUBRAMANIAN
  // 2005
  //
  // http://coewww.rutgers.edu/www2/vizlab/NicuCornea/Skeletanization/skeletanization.html
  bool load(const QString& path);
};

}
