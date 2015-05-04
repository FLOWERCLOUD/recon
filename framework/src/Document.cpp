#include <Recon/Document.h>
#include <QDir>

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
  return m_Cameras;
}

const QVector<Feature>& Document::features() const
{
  return m_Features;
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
