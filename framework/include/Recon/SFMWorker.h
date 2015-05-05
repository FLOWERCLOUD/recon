#pragma once

#include <QString>
#include <QProcess>
#include <QTemporaryDir>

namespace recon {

class Document;

class SFMWorker : public QObject {
  Q_OBJECT
public:
  SFMWorker(Document* doc, QObject* parent = 0);
  ~SFMWorker();

  void start();

private slots:
  void jobFinished(int code, QProcess::ExitStatus status);

private:
  Document* m_Document;
  QProcess* m_Process;

  QTemporaryDir m_TempDir;
};

}
