#pragma once

#include <QObject>
#include <QProcess>
#include <QString>
#include <QStringList>

namespace reconapp {

class SFMWorker : public QObject { // QProcess?
  Q_OBJECT
public:
  SFMWorker(QObject* parent = 0);
  virtual ~SFMWorker();

  QString rootPath() const;
  void setRootPath(const QString& rootPath);

  const QStringList& imagePaths() const;
  void setImagePaths(const QStringList& imagePaths);

  void start();

signals:
  void finish();

private:
  QString m_RootPath;
  QStringList m_ImagePaths;
};

}
