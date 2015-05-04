#pragma once

#include <QString>
#include <QProcess>

namespace recon {

class Document;

class SFMWorker : public QObject {
  Q_OBJECT
public:
  SFMWorker(Document* doc, QObject* parent = 0);

  void start();

private slots:
  void jobFinished(int code, QProcess::ExitStatus status);

private:
  QString m_WorkPath;
  QString m_ResultFile;
  QProcess m_Process;

};

}
