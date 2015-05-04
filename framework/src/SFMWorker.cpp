#include <Recon/SFMWorker.h>
#include <Recon/NVMLoader.h>

namespace recon {

SFMWorker::SFMWorker(Document* doc, QObject* parent)
: QObject(parent), m_Document(doc)
{
  QDir doc_dir(doc->basePath());
  QStringList args;
  QProcessEnvironment env = QProcessEnvironment::systemEnvironment();

  args << "sfm+shared+sort";
  args << doc_dir.absoluteFilePath("images");
  args << m_TempDir.absoluteFilePath("bundle.nvm");

  env.insert("PATH", "C:\\Program Files\\VisualSFM\\VisualSFM CUDA");

  m_Process.setProgram("VisualSFM");
  m_Process.setArguments(args);
  m_Process.setProcessEnvironment(env);

  connect(&m_Process, &QProcess::finished, this, &jobFinished);
}

SFMWorker::~SFMWorker()
{
  disconnect(&m_Process, &QProcess::finished, this, &jobFinished);
}

void SFMWorker::start()
{
  m_Process.start();
}

void SFMWorker::jobFinished(int code, QProcess::ExitStatus status)
{
  if (status == QProcess::NormalExit) {
    NVMLoader(m_TempDir.absoluteFilePath("bundle.nvm")).load(m_Document);
  }

  this->deleteLater();
}

}
