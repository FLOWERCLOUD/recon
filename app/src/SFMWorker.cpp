#include "SFMWorker.h"
#include <QDir>

namespace reconapp {

SFMWorker::SFMWorker(QObject* parent)
: QObject(parent)
{
  m_Proc.setProgram("VisualSFM");

  QStringList args;
  args << "sfm+shared"
       << "images"
       << "sparse.nvm";
  m_Proc.setArguments(args);

  connect(&m_Proc, SIGNAL(finished(int, QProcess::ExitStatus)),
          this, SLOT(onFinished()));

  m_Proc.setProcessChannelMode(QProcess::MergedChannels);
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
  if (m_Proc.state() != QProcess::NotRunning)
    return;

  QDir rootDir(m_RootPath);

  m_Proc.setWorkingDirectory(rootDir.absolutePath());
  m_Proc.start();
}

void SFMWorker::onFinished()
{

}

}
