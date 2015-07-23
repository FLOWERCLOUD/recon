#include "Skeleton.h"
#include <QFile>
#include <QTextStream>
#include <QtDebug>
#include <math.h>

namespace recon {

SkeletonField::SkeletonField(int lv)
: level(lv)
, skeleton()
, field((1 << (lv*3)), 0.0f)
{
}

void SkeletonField::computeField()
{
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
