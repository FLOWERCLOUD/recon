#include "CameraLoader.h"
#include <QDir>
#include <QImageReader>
#include <QFile>
#include <QTextStream>

namespace voxel {

bool load_from_nvm(QStringList& images,
                   CameraList& cameras,
                   const QString& path)
{
  QFile file(path);
  if (!file.open(QFile::ReadOnly))
    return false;

  QTextStream stream(&file);

  // Check file type
  {
    QString magic;
    stream >> magic;
    if (magic != "NVM_V3")
      return false;
  }

  // Camera count
  int ncams;
  {
    stream >> ncams;
    if (ncams < 1)
      return false;

    cameras.clear();
    images.clear();
    cameras.reserve(ncams);
    images.reserve(ncams);
  }

  QDir bundledir(path.section(QDir::separator(), 0, -2, QString::SectionIncludeLeadingSep));

  // Camera data
  {
    QString imagename;
    float focal;
    float aspect;
    float orient[4]; // XYZW
    float center[3];
    float distortion;
    int temp;

    for (int i = 0; i < ncams; ++i) {
      stream >> imagename;
      stream >> focal;
      stream >> orient[3] >> orient[0] >> orient[1] >> orient[2];
      stream >> center[0] >> center[1] >> center[2];
      stream >> distortion;
      stream >> temp; // END of camera

      if (QDir::isRelativePath(imagename))
        imagename = bundledir.absoluteFilePath(imagename);

      images.append(imagename);

      if (QFile::exists(imagename)) {
        QImageReader reader(imagename);
        QSize dim = reader.size();
        aspect = (float)dim.width() / (float)dim.height();
      } else {
        aspect = 1.0f;
      }

      CameraData cam;
      cam.focal_length = focal;
      cam.aspect_ratio = aspect;
      cam.radial_distortion[0] = distortion;
      cam.radial_distortion[1] = 0.0f;
      memcpy(cam.center, center, sizeof(float)*3);
      memcpy(cam.orientation, orient, sizeof(float)*4);

      cam.update_extrinsic();
      cam.update_intrinsic();

      cameras.append(cam);
    }
  }

  return true;
}

}
