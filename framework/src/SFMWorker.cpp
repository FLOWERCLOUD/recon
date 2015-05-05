#include <Recon/SFMWorker.h>
#include <Recon/Document.h>
#include <Recon/NVMLoader.h>
#include <QDir>

namespace recon {

SFMWorker::SFMWorker(Document* doc, QObject* parent)
: QObject(parent), m_Document(doc)
{
  QDir doc_dir(doc->basePath());
  QStringList args;
  QProcessEnvironment env = QProcessEnvironment::systemEnvironment();

  args << "sfm+shared+sort";
  args << doc_dir.absoluteFilePath("images");
  args << QDir(m_TempDir.path()).absoluteFilePath("bundle.nvm");

#if defined(_WIN32) || defined(_WIN64)
  env.insert("PATH", "C:\\Program Files\\VisualSFM\\VisualSFM CUDA");
#endif

  m_Process = new QProcess(this);
  m_Process->setProgram("VisualSFM");
  m_Process->setArguments(args);
  m_Process->setProcessEnvironment(env);

  connect(m_Process, (void (QProcess::*)(int,QProcess::ExitStatus))&QProcess::finished,
          this, &SFMWorker::jobFinished);
}

SFMWorker::~SFMWorker()
{
  //disconnect(&m_Process, &QProcess::finished, this, &SFMWorker::jobFinished);
}

void SFMWorker::start()
{
  m_Process->start();
}

void SFMWorker::jobFinished(int code, QProcess::ExitStatus status)
{
  if (status == QProcess::NormalExit) {
    QString path = QDir(m_TempDir.path()).absoluteFilePath("bundle.nvm");
    NVMLoader(path).load(m_Document);
  }

  this->deleteLater();
}

}
