#pragma once

#include "FileSet.h"
#include <QString>
#include <QUrl>
#include <QList>
#include <QStringList>

namespace recon {

class ImageSet : public FileSet {
  Q_OBJECT
  Q_PROPERTY(int count READ count NOTIFY countChanged)
  Q_PROPERTY(QStringList names READ names)
public:
  ImageSet(QObject* parent = 0);
  virtual ~ImageSet();

  int count() const;
  QStringList names() const;

  Q_INVOKABLE bool importImage(const QUrl& url);

  Q_INVOKABLE QUrl urlFromName(const QString& name) const;

  Q_INVOKABLE void reload();

signals:
  void countChanged(int count);
  void imageAdded(QString name);

private slots:
  void onImageAdded(QString name);

private:
  QStringList m_Names;
};

}
