#include "CameraLoader.h"
#include <QDir>
#include <QImageReader>
#include <QFile>
#include <QTextStream>
#include <QImage>
#include <QPainter>
#include <QPen>
#include <QBrush>
//#include <stdio.h>
#include <string.h>
#include <math.h>

namespace recon {

CameraLoader::CameraLoader()
{
}

CameraLoader::~CameraLoader()
{
}

const QList<Camera>& CameraLoader::cameras() const
{
  return m_Cameras;
}

const AABox& CameraLoader::model_boundingbox() const
{
  return m_ModelBox;
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
    m_Cameras.reserve(ncams);
  }

  QDir bundledir(path.section(QDir::separator(), 0, -2, QString::SectionIncludeLeadingSep));
  if (!bundledir.exists())
    return false;

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

      if (QFile::exists(imagename)) {
        QImageReader reader(imagename);
        QSize dim = reader.size();
        aspect = (float)dim.width() / (float)dim.height();
        focal /= (float)dim.height();
      } else {
        aspect = 1.0f;
      }

      Camera cam;
      cam.setFocal(focal);
      cam.setAspect(aspect);
      cam.setRadialDistortion(distortion, 0.0f);
      cam.setCenter(point3::load(center));
      cam.setRotation(quat::load(orient));
      cam.setImagePath(imagename);
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
    bool bbox_first = true;
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

      bool visible = true;
      for (const Camera& cam : m_Cameras) {
        visible = visible && cam.canSee(vec3::load(pos));
      }
      if (!visible)
        continue;

      FeatureData feat;
      feat.pos[0] = pos[0];
      feat.pos[1] = pos[1];
      feat.pos[2] = pos[2];
      feat.color = qRgb(rgb[0], rgb[1], rgb[2]);
      m_Features.append(feat);

      if (bbox_first) {
        m_ModelBox = AABox(vec3::load(pos));
        bbox_first = false;
      } else {
        m_ModelBox.add(vec3::load(pos));
      }
    }
  }

#if false
  debug_render_features("debug_features-0.png", 0);
#endif

  return true;
}

void CameraLoader::debug_render_features(const QString& path, int camera_id) const
{
  if (camera_id < 0 || camera_id >= m_Cameras.size())
    return;

  Camera cam = m_Cameras[camera_id];

  QImage canvas = QImage(cam.imagePath());
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

  mat4 extrinsic = cam.extrinsic();
  mat4 intrinsic = cam.intrinsicForImage(width, height);
  /*{
    float m[16];
    transpose(intrinsic).store(m);

    for (int i = 0; i < 16; ++i) {
      printf("%f ", m[i]);
      if (i % 4 == 3)
        printf("\n");
    }
  }*/

  // Draw Feature Bounding Box

  // Draw Feature Points
  for (int i = 0; i < npoints; ++i) {
    const FeatureData& feat = m_Features[i];

    vec3 worldpos = vec3::load(feat.pos);
    vec3 pt = proj_vec3(intrinsic * (extrinsic * vec4(worldpos, 1.0f)));
    //printf("(%f %f %f)\n", (float)pt.x(), (float)pt.y(), 1.0f / (float)pt.z());

    int penwidth = 10;
    painter.setPen(QPen(QBrush(QColor((QRgb)feat.color)), penwidth));
    painter.drawPoint((float)pt.x(), (float)pt.y());
  }

  canvas.save(path);
}

}
