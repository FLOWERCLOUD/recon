#include <Recon/SFMContext.h>
#include <Recon/ImageSet.h>
#include <QProcess>
#include <QDir>
#include <QTemporaryDir>

namespace recon {

SFMContext::SFMContext(QObject* parent)
: QObject(parent), m_SourceImages(NULL)
{
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
}

void SFMContext::start()
{
  QStringList args;
  QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
  QDir srcpath(images()->basePath());

  args << "sfm+shared+sort";
  args << srcpath.absolutePath();
  args << "bundle.nvm";

#if defined(_WIN32) || defined(_WIN64)
  env.insert("PATH", "C:\\Program Files\\VisualSFM\\VisualSFM CUDA");
#endif

  m_Process = new QProcess(this);
  m_Process->setProgram("VisualSFM");
  m_Process->setArguments(args);
  m_Process->setProcessEnvironment(env);
  m_Process->setWorkingDirectory(m_TempDir.path());
  m_Process->setStandardErrorFile("stderr.log");
  m_Process->setStandardOutputFile("stdout.log");

  connect(m_Process, (void (QProcess::*)(int,QProcess::ExitStatus))&QProcess::finished,
          this, &SFMContext::onProcFinished);

  m_Process->open();
}

void SFMContext::onProcFinished(int code,QProcess::ExitStatus status)
{
  if (status == QProcess::NormalExit) {
    emit finished();
  }
}

}
