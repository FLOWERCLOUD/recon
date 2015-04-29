#include "Bundle.h"
#include <QtGlobal>
#include <QtDebug>
#include <QTextStream>

namespace recon {

Bundle::Bundle()
: m_CamerasNum(0), m_CamerasCapacity(0), m_Cameras(NULL)
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

Camera& Bundle::get_camera(int index)
{
  Q_ASSERT(index >= 0 && index < camera_count());
  return m_Cameras[index];
}

void Bundle::clear()
{
  if (m_Cameras) {
    free(m_Cameras);

    m_CamerasNum = 0;
    m_CamerasCapacity = 0;
    m_Cameras = NULL;
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
    QString filename;
    float focal_length;
    float orient[4]; // XYZW
    float center[3];
    float radial_distortion;
    int temp;

    stream >> filename;
    stream >> focal_length;
    stream >> orient[3] >> orient[0] >> orient[1] >> orient[2];
    stream >> center[0] >> center[1] >> center[2];
    stream >> radial_distortion;
    stream >> temp; // END of camera

    Camera& cam = m_Cameras[i];
    cam.focial_length = focal_length;
    cam.pixel_aspect_ratio = 1.0f;
    cam.principal_point[0] = 0.5f; // TODO
    cam.principal_point[1] = 0.5f; // TODO
    cam.radial_distortion[0] = radial_distortion;
    cam.radial_distortion[1] = 0.0f;

    Quat q;
    q.setX(orient[0]);
    q.setY(orient[1]);
    q.setZ(orient[2]);
    q.setW(orient[3]);
    set_orientation(cam, q);

    set_position(cam, Vector3(center[0], center[1], center[2]));
  }
  m_CamerasNum = num_cameras;

  stream >> num_points;
  // TODO: features

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

}
