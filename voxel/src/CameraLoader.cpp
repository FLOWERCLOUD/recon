#include "CameraLoader.h"
#include <QDir>
#include <QImageReader>
#include <QFile>
#include <QTextStream>
#include <QImage>
#include <QPainter>
#include <QPen>
#include <QBrush>
#include <string.h>
#include <math.h>

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

    m_Features.clear();
    m_Features.reserve(npoints);
  }

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

      FeatureData feat;
      feat.pos[0] = pos[0];
      feat.pos[1] = pos[1];
      feat.pos[2] = pos[2];
      feat.color = qRgb(rgb[0], rgb[1], rgb[2]);
      m_Features.append(feat);

      if (i == 0)
        m_FeatureAABB.fill(pos);
      else
        m_FeatureAABB.add(pos);
    }
  }

#if true
  debug_render_features("debug_features-0.png", 0);
#endif

  return true;
}

void CameraLoader::debug_render_features(const QString& path, int camera_id) const
{
  using vectormath::aos::load_vec3;

  if (camera_id < 0 || camera_id >= m_Cameras.size())
    return;

  const CameraData& cam = m_Cameras[camera_id];

  QImage canvas = QImage(m_ImagePaths[camera_id]);
  if (canvas.isNull())
    return;

  int width = canvas.width();
  int height = canvas.height();
  int npoints = m_Features.size();

  for (int i = 0; i < height; ++i) {
    for (int j = 0; j < width; ++j) {
      int g = qGray(canvas.pixel(j, i));
      canvas.setPixel(j, i, qRgb(g, g, g));
    }
  }

  QPainter painter(&canvas);

  // Draw Feature Bounding Box

  // Draw Feature Points
  for (int i = 0; i < npoints; ++i) {
    const FeatureData& feat = m_Features[i];

    vec3 pt = cam.world_to_image(load_vec3(feat.pos), width, height);

    float ptdata[3];
    store_vec3(ptdata, pt);

    int penwidth = 10;
    painter.setPen(QPen(QBrush(QColor((QRgb)feat.color)), penwidth));
    painter.drawPoint(ptdata[0], ptdata[1]);
  }

  canvas.save(path);
}

}
