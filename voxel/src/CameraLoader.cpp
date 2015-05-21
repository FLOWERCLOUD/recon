#include "CameraLoader.h"
#include <QDir>
#include <QImageReader>
#include <QFile>
#include <QTextStream>
#include <string.h>
#include <math.h>

#define DEBUG_RENDER_FEATURES 1
#define DEBUG_CAMERA_ID 0
#if DEBUG_RENDER_FEATURES
#  include <QImage>
#  include <QPainter>
#  include <QPen>
#  include <QBrush>
#  include <QtDebug>
#endif

namespace voxel {

CameraLoader::CameraLoader()
{
}

CameraLoader::~CameraLoader()
{
}

const QStringList& CameraLoader::image_paths() const
{
  return m_ImagePaths;
}

const CameraList& CameraLoader::cameras() const
{
  return m_Cameras;
}

const AABB& CameraLoader::feature_boundingbox() const
{
  return m_FeatureAABB;
}

bool CameraLoader::load_from_nvm(const QString& path)
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

    m_Cameras.clear();
    m_ImagePaths.clear();
    m_Cameras.reserve(ncams);
    m_ImagePaths.reserve(ncams);
  }

  QDir bundledir(path.section(QDir::separator(), 0, -2, QString::SectionIncludeLeadingSep));

#if DEBUG_RENDER_FEATURES
  QImage dst0;
  QImage src0;
#endif

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

      m_ImagePaths.append(imagename);

      if (QFile::exists(imagename)) {
        QImageReader reader(imagename);
        QSize dim = reader.size();
        aspect = (float)dim.width() / (float)dim.height();
        focal /= (float)dim.height();

#if DEBUG_RENDER_FEATURES
        if (i == DEBUG_CAMERA_ID) {
          dst0 = QImage(dim, QImage::Format_ARGB32);
          dst0.fill(qRgb(0, 0, 0));
          src0 = QImage(imagename);
        }
#endif
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

      m_Cameras.append(cam);
    }
  }

  // Feature count
  int npoints;
  {
    stream >> npoints;
    if (npoints < 1)
      return false;
  }

#if DEBUG_RENDER_FEATURES
  QPainter dstpaint(&dst0);
  QPainter srcpaint(&src0);
#endif

  // Feature data
  {
    float pos[3];
    int rgb[3]; // each component is in range of 0-255
    int num_measurements;
    int image_index, feature_index;
    float pos2d[2];

    for (int i = 0; i < npoints; ++i) {
      stream >> pos[0] >> pos[1] >> pos[2];
      stream >> rgb[0] >> rgb[1] >> rgb[2];
      stream >> num_measurements;
      for (int j = 0; j < num_measurements; ++j) {
        stream >> image_index >> feature_index;
        stream >> pos2d[0] >> pos2d[1];
      }

      if (i == 0)
        m_FeatureAABB.fill(pos);
      else
        m_FeatureAABB.add(pos);

#if DEBUG_RENDER_FEATURES
      using vectormath::aos::load_vec3;
      const CameraData& cam = m_Cameras[DEBUG_CAMERA_ID];
      vec3 pt = cam.world_to_image(load_vec3(pos), src0.width(), src0.height());
      float ptdata[3];
      store_vec3(ptdata, pt);

      {
        int penwidth = 10;
        srcpaint.setPen(QPen(QBrush(Qt::green), penwidth));
        srcpaint.drawPoint(ptdata[0], ptdata[1]);
        dstpaint.setPen(QPen(QBrush(QColor(rgb[0], rgb[1], rgb[2])), penwidth));
        dstpaint.drawPoint(ptdata[0], ptdata[1]);
      }
#endif
    }
  }

  src0.setPixel(100, 100, qRgb(255, 0, 0));

#if DEBUG_RENDER_FEATURES
  dst0.save("debug-dst0.png");
  src0.save("debug-src0.png");
#endif

  return true;
}

}
