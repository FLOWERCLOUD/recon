#include <Recon/Document.h>
#include <QDir>
#include <QFile>
#include <QTextStream>

namespace recon {

Document::Document(const QString& basepath, QObject* parent)
: QObject(parent)
, m_BasePath(QDir::isAbsolutePath(basepath) ? basepath : QDir(basepath).absolutePath())
{
}

Document::~Document()
{
}

const QString& Document::basePath() const
{
  return m_BasePath;
}

const QVector<Camera>& Document::cameras() const
{
  // TODO: cached + lazy initialization
  return m_Cameras;
}

const QVector<Feature>& Document::features() const
{
  // TODO: cached + lazy initialization
  return m_Features;
}

void Document::reload()
{
  // TODO: clear cache

  QDir dir(basePath());
  if (dir.exists()) {
  }
}

void Document::save()
{
  QDir dir(basePath());
  if (!dir.exists()) {
    dir.mkpath(".");
  }

  // Save cameras
  {
    QFile file(dir.filePath("cameras.txt"));
    file.open(QIODevice::WriteOnly);

    QTextStream stream(&file);
    for (auto it = m_Cameras.cbegin(), itend = m_Cameras.cend(); it != itend; ++it) {
      stream << it->index << " "
             << it->focal_length << " "
             << it->radial_distortion[0] << " "
             << it->radial_distortion[1] << " ";
      for (int i = 0; i < 3; ++i) {
        stream << it->center[i] << " ";
      }
      for (int i = 0; i < 9; ++i) {
        stream << it->intrinsic[i] << " ";
      }
      for (int i = 0; i < 12; ++i) {
        stream << it->extrinsic[i] << " ";
      }
      stream << "\n";
    }

    file.close();
  }

  // Save Features
  {
    QFile file(dir.filePath("features.txt"));
    file.open(QIODevice::WriteOnly);

    QTextStream stream(&file);
    for (auto it = m_Features.cbegin(), itend = m_Features.cend(); it != itend; ++it) {
      for (int i = 0; i < 3; ++i) {
        stream << it->pos[i] << " ";
      }
      for (int i = 0; i < 3; ++i) {
        stream << it->color[i] << " ";
      }
      stream << "\n";
    }

    file.close();
  }
}

void Document::swapCameras(QVector<Camera>& v)
{
  m_Cameras.swap(v);
}

void Document::swapFeatures(QVector<Feature>& v)
{
  m_Features.swap(v);
}

}
