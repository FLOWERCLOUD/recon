#pragma once

#include <QObject>
#include <QProcess>
#include <QString>
#include <QStringList>

namespace reconapp {

class SFMWorker : public QObject {
  Q_OBJECT
public:
  SFMWorker(QObject* parent = 0);
  virtual ~SFMWorker();

  QString rootPath() const;
  void setRootPath(const QString& rootPath);

  const QStringList& imagePaths() const;
  void setImagePaths(const QStringList& imagePaths);

  void start();

private slots:
  void onFinished();

private:
  QString m_RootPath;
  QStringList m_ImagePaths;
  QProcess m_Proc;
};

}
