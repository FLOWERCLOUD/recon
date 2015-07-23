#include "Skeleton.h"
#include <QFile>
#include <QTextStream>
#include <QtDebug>
#include <math.h>

namespace recon {

SkeletonField::SkeletonField(int lv)
: level(lv)
, skeleton()
, field((0x1ull << (lv*3)), 0.0f)
{
}

void SkeletonField::computeField()
{
  uint64_t n = 0x1ull << (level * 3);
  float w = float(0x1 << level);

  for (uint64_t m = 0; m < n; ++m) {
    uint32_t x, y, z;
    morton_decode(m, x, y, z);

    float min_dist = INFINITY;
    for (uint64_t m0 : skeleton) {
      uint32_t x0, y0, z0;
      morton_decode(m0, x0, y0, z0);

      float dx = ((float)x - (float)x0) / w;
      float dy = ((float)y - (float)y0) / w;
      float dz = ((float)z - (float)z0) / w;
      float d = sqrtf(dx * dx + dy * dy + dz * dz);
      min_dist = fminf(min_dist, d);
    }
    field[(uint)m] = min_dist;
  }
}

bool SkeletonField::load(const QString& path)
{
  QFile file(path);
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    qDebug() << "Cannot open input file!";
    return false;
  }

  QTextStream stream(&file);
  while (!stream.atEnd()) {
    QString line = stream.readLine();
    if (line.length() == 0)
      continue;
    else if (line.startsWith("#")) // comment
      continue;

    float spx, spy, spz, dt;
    int sps;
    QTextStream(&line) >> spx >> spy >> spz >> sps >> dt;

    uint32_t x, y, z;
    x = roundf(spx);
    y = roundf(spy);
    z = roundf(spz);
    uint64_t m = recon::morton_encode(x,y,z);
    //skeleton[(uint)m] = 0xFF;
    skeleton.append(m);
  }
  return true;
}

}
