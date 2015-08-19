#include "SFMWorker.h"
#include <QDir>

namespace reconapp {

SFMWorker::SFMWorker(QObject* parent)
: QObject(parent)
{
}

SFMWorker::~SFMWorker()
{
}

QString SFMWorker::rootPath() const
{
  return m_RootPath;
}

void SFMWorker::setRootPath(const QString& rootPath)
{
  m_RootPath = rootPath;
}

const QStringList& SFMWorker::imagePaths() const
{
  return m_ImagePaths;
}

void SFMWorker::setImagePaths(const QStringList& imagePaths)
{
  m_ImagePaths = imagePaths;
}

void SFMWorker::start()
{
  QDir rootDir(m_RootPath);
  // TODO
}

}
