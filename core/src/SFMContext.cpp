#include <Recon/SFMContext.h>
#include <Recon/ImageSet.h>
#include <QProcess>
#include <QDir>
#include <QTemporaryDir>
#include <QtDebug>

namespace recon {

SFMContext::SFMContext(QObject* parent)
: QObject(parent), m_SourceImages(NULL), m_Process(NULL)
{
  qDebug() << "Temp Path: " << m_TempDir.path();
}

SFMContext::~SFMContext()
{
}

ImageSet* SFMContext::images() const
{
  return m_SourceImages;
}

void SFMContext::setImages(ImageSet* images)
{
  m_SourceImages = images;
  emit imagesChanged(images);
}

void SFMContext::start()
{
  if (m_Process || !images()) {
    qDebug() << "Not started.";
    return;
  }

  QStringList args;
  QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
  QDir srcpath(images()->basePath());
  QDir tmppath(m_TempDir.path());

  args << "sfm";
  args << srcpath.absolutePath();
  args << "bundle.nvm";
  qDebug() << args;

#if defined(_WIN32) || defined(_WIN64)
  env.insert("PATH", "C:\\Program Files\\VisualSFM\\VisualSFM CUDA");
#endif

  m_Process = new QProcess(this);
  m_Process->setProgram("VisualSFM");
  m_Process->setArguments(args);
  m_Process->setProcessEnvironment(env);
  m_Process->setWorkingDirectory(tmppath.absolutePath());
  m_Process->setStandardErrorFile(tmppath.absoluteFilePath("stderr.log"));
  m_Process->setStandardOutputFile(tmppath.absoluteFilePath("stdout.log"));

  connect(m_Process, (void (QProcess::*)(int,QProcess::ExitStatus))&QProcess::finished,
          this, &SFMContext::onProcFinished);
  connect(m_Process, (void (QProcess::*)(QProcess::ProcessError))&QProcess::error,
          this, &SFMContext::onProcError);

  m_Process->start();
}

void SFMContext::onProcFinished(int code,QProcess::ExitStatus status)
{
  if (status == QProcess::NormalExit) {
    emit finished();
  }
}

void SFMContext::onProcError(QProcess::ProcessError error)
{
  switch (error) {
    case QProcess::FailedToStart:
      qDebug() << "Failed to Start";
      break;
    case QProcess::Crashed:
      qDebug() << "Crashed";
      break;
    case QProcess::Timedout:
      qDebug() << "Time out";
      break;
    case QProcess::WriteError:
      qDebug() << "Write Error";
      break;
    case QProcess::ReadError:
      qDebug() << "Read Error";
      break;
    case QProcess::UnknownError:
      qDebug() << "Proc: Unknown Error";
      break;
  }
}

}
