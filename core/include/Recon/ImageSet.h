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
  Q_PROPERTY(QStringList names READ names NOTIFY namesChanged)
public:
  ImageSet(QObject* parent = 0);
  virtual ~ImageSet();

  int count() const;
  QStringList names() const;

  Q_INVOKABLE bool importImage(QUrl url);

  Q_INVOKABLE QUrl urlFromName(const QString& name) const;

signals:
  void countChanged(int count);
  void namesChanged(QStringList names);

private:
  QStringList m_Names;
};

}
