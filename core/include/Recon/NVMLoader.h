#pragma once

#include <QObject>
#include <QString>

class QTextStream;

namespace recon {

class Document;
struct Camera;
struct Feature;

class NVMLoader : public QObject {
  Q_OBJECT
public:
  NVMLoader(const QString& path, QObject* parent = 0);

  bool load(Document* document);

private:
  const QString& path() const;
  bool exist() const;

  bool _loadFromStream(Document* document, QTextStream& stream);

private:
  QString m_Path;
};

}
