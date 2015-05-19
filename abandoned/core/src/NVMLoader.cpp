#include <Recon/NVMLoader.h>
#include <Recon/Document.h>
#include <QFile>
#include <QDir>
#include <QtDebug>

namespace recon {

using vectormath::aos::vec3;
using vectormath::aos::vec4;
using vectormath::aos::quat;
using vectormath::aos::mat3;
using vectormath::aos::mat4;
using vectormath::aos::load_vec3;
using vectormath::aos::store_vec3;
using vectormath::aos::load_mat3;
using vectormath::aos::store_mat3;
using vectormath::aos::load_quat;

NVMLoader::NVMLoader(const QString& path, QObject* parent)
: QObject(parent), m_Path(path)
{
}

bool NVMLoader::exist() const
{
  return QFile::exists(path());
}

bool NVMLoader::load(Document* document)
{
  QFile file(path());
  if (!file.open(QIODevice::ReadOnly))
    return false;

  QTextStream stream(&file);
  bool result = _loadFromStream(document, stream);

  file.close();

  return result;
}

const QString& NVMLoader::path() const
{
  return m_Path;
}

bool NVMLoader::_loadFromStream(Document* document, QTextStream& stream)
{
  QString token;

  stream >> token;
  if (token != "NVM_V3")
    return false;

  int num_cam, num_points;

  stream >> num_cam;
  QVector<Camera> cams(num_cam);
  {
    QString imagename;
    float focal_length;
    float orient[4]; // XYZW
    float center[3];
    float radial_distortion;
    int temp;

    // Load camera parameters
    for (int i = 0; i < num_cam; ++i) {
      stream >> imagename;
      stream >> focal_length;
      stream >> orient[3] >> orient[0] >> orient[1] >> orient[2];
      stream >> center[0] >> center[1] >> center[2];
      stream >> radial_distortion;
      stream >> temp; // END of camera

      Camera& cam = cams[i];
      cam.index = i;
      cam.focal_length = focal_length;
      cam.radial_distortion[0] = radial_distortion;
      cam.radial_distortion[1] = 0.0f;

      cam.intrinsic[0] = focal_length / 4896.0f * 2.0f;
      cam.intrinsic[1] = 0.0f;
      cam.intrinsic[2] = 0.0f;
      cam.intrinsic[3] = 0.0f;
      cam.intrinsic[4] = focal_length / 3264.0f * -2.0f;
      cam.intrinsic[5] = 0.0f;
      cam.intrinsic[6] = 0.0f;
      cam.intrinsic[7] = 0.0f;
      cam.intrinsic[8] = 1.0f;

      memcpy(cam.center, center, sizeof(float)*3);

      mat3 rot = to_mat3(load_quat(orient));
      vec3 pos = load_vec3(center);
      vec3 trans = -(rot * pos);
      store_mat3(cam.extrinsic+0, rot);
      store_vec3(cam.extrinsic+9, trans);
    }
  }

  // Load feature points
  stream >> num_points;
  QVector<Feature> points(num_points);
  {
    float pos[3];
    int rgb[3]; // each component is in range of 0-255
    int num_measurements;
    int image_index, feature_index;
    float pos2d[2];

    for (int i = 0; i < num_points; ++i) {
      stream >> pos[0] >> pos[1] >> pos[2];
      stream >> rgb[0] >> rgb[1] >> rgb[2];
      stream >> num_measurements;
      for (int j = 0; j < num_measurements; ++j) {
        stream >> image_index >> feature_index;
        stream >> pos2d[0] >> pos2d[1];
      }

      Feature& feature = points[i];
      memcpy(feature.pos, pos, sizeof(float)*3);
      feature.color[0] = rgb[0];
      feature.color[1] = rgb[1];
      feature.color[2] = rgb[2];
    }
  }

  // Check
  if (num_cam <= 0) {
    qWarning() << "Invalid number of cameras (value = " << num_cam << ")";
    return false;
  }
  if (num_cam > 10000) {
    qWarning() << "The bundle has too many cameras";
    return false;
  }

  // Update document
  document->swapCameras(cams);
  document->swapFeatures(points);

  return true;
}

}
