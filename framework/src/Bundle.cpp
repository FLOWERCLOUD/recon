#include "Bundle.h"
#include <QtGlobal>
#include <QtDebug>
#include <QTextStream>

namespace recon {

Bundle::Bundle()
: m_CamerasNum(0), m_CamerasCapacity(0), m_Cameras(NULL),
  m_FeaturesNum(0), m_FeaturesCapacity(0), m_Features(NULL)
{
}

Bundle::~Bundle()
{
  clear();
}

int Bundle::camera_count() const
{
  return m_CamerasNum;
}

const Camera& Bundle::get_camera(int index) const
{
  Q_ASSERT(index >= 0 && index < camera_count());
  return m_Cameras[index];
}

int Bundle::feature_count() const
{
  return m_FeaturesNum;
}

const FeatureVertex* Bundle::get_features() const
{
  return m_Features;
}

void Bundle::clear()
{
  if (m_Cameras) {
    free(m_Cameras);

    m_CamerasNum = 0;
    m_CamerasCapacity = 0;
    m_Cameras = NULL;
  }

  if (m_Features) {
    free(m_Features);

    m_FeaturesNum = 0;
    m_FeaturesCapacity = 0;
    m_Features = NULL;
  }
}

bool Bundle::load_nvm(QIODevice* io)
{
  QString token;
  QTextStream stream(io);

  // Magic
  stream >> token;
  if (token != "NVM_V3") {
    return false;
  }

  stream >> token;
  if (token == "FixedK") { // optional calibration
    float fx, cx, fy, cy;
    stream >> fx >> cx >> fy >> cy >> token;
  }

  int num_cameras, num_points;

  // Cameras

  num_cameras = token.toInt();
  if (num_cameras <= 0) {
    qWarning() << "Invalid number of cameras (value = "
               << num_cameras << ")";
    return false;
  }
  if (num_cameras > 10000) {
    qWarning() << "The bundle has too many cameras";
    return false;
  }

  allocate_cameras(num_cameras);

  for (int i = 0; i < num_cameras; ++i) {
    Camera& cam = m_Cameras[i];
    QString imagename;
    load_from_nvm(cam, imagename, stream);
    cam.index = i;
  }
  m_CamerasNum = num_cameras;

  // Features

  stream >> num_points;
  if (num_points > 500000) {
    qWarning() << "The bundle has too many features";
    clear();
    return false;
  }

  allocate_features(num_points);

  for (int i = 0; i < num_points; ++i) {
    float pos[3];
    int rgb[3]; // each component is in range of 0-255
    int num_measurements;
    int image_index, feature_index;
    float pos2d[2];

    stream >> pos[0] >> pos[1] >> pos[2];
    stream >> rgb[0] >> rgb[1] >> rgb[2];
    stream >> num_measurements;
    for (int j = 0; j < num_measurements; ++j) {
      stream >> image_index >> feature_index;
      stream >> pos2d[0] >> pos2d[1];
    }

    FeatureVertex* feature = m_Features + i;
    memcpy(feature->pos, pos, sizeof(float)*3);
    feature->color[0] = rgb[0];
    feature->color[1] = rgb[1];
    feature->color[2] = rgb[2];
  }
  m_FeaturesNum = num_points;

  return true;
}

void Bundle::allocate_cameras(int n)
{
  if (n > m_CamerasCapacity) {
    int cap = n;

    Camera* addr = (Camera*)malloc(sizeof(Camera) * cap);
    Q_CHECK_PTR(addr);

    if (m_CamerasNum > 0) {
      Camera* src = m_Cameras;
      memcpy(addr, src, sizeof(Camera) * m_CamerasNum);
    }
    free(m_Cameras);

    (void)m_CamerasNum; // untouched
    m_CamerasCapacity = cap;
    m_Cameras = addr;
  }
}

void Bundle::allocate_features(int n)
{
  if (n > m_FeaturesCapacity) {
    int cap = n;

    FeatureVertex* addr = (FeatureVertex*)malloc(sizeof(FeatureVertex) * cap);
    Q_CHECK_PTR(addr);

    if (m_CamerasNum > 0) {
      FeatureVertex* src = m_Features;
      memcpy(addr, src, sizeof(FeatureVertex) * m_FeaturesNum);
    }
    free(m_Features);

    (void)m_FeaturesNum; // untouched
    m_FeaturesCapacity = cap;
    m_Features = addr;
  }
}

}
