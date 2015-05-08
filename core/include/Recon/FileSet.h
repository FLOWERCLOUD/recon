#pragma once

#include <QObject>
#include <QString>
#include <QUrl>

namespace recon {

class FileSet : public QObject {
  Q_OBJECT
  Q_PROPERTY(QUrl baseUrl READ baseUrl WRITE setBaseUrl NOTIFY baseUrlChanged)
  Q_PROPERTY(QString basePath READ basePath NOTIFY basePathChanged)
public:
  FileSet(QObject* parent = 0);
  virtual ~FileSet();

  bool isValid() const;

  QUrl baseUrl() const;
  void setBaseUrl(const QUrl& url);
  QString basePath() const;

signals:
  void baseUrlChanged(QUrl url);
  void basePathChanged(QString path);

private slots:
  void onBaseUrlChanged(QUrl url);

private:
  QUrl m_BaseUrl;
};

}
