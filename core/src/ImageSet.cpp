#include <Recon/ImageSet.h>
#include <QUuid>
#include <QDir>
#include <QFile>

namespace recon {

ImageSet::ImageSet(QObject* parent)
: FileSet(parent)
{
}

ImageSet::~ImageSet()
{
}

int ImageSet::count() const
{
  return m_Names.size();
}

QStringList ImageSet::names() const
{
  return m_Names;
}

bool ImageSet::importImage(QUrl url)
{
  if (!isValid() || !url.isLocalFile())
    return false;

  if (!url.toString().endsWith(".jpg", Qt::CaseInsensitive))
    return false;

  QDir dir(basePath());

  QUuid uuid;
  QString name;
  do {
    uuid = QUuid::createUuid();
    name = QString("%1.jpg").arg(uuid.toString());
  } while (dir.exists(name));

  QString newpath = dir.absoluteFilePath(name);

  if (QFile::copy(url.toLocalFile(), newpath)) {
    m_Names << name;
    emit namesChanged(m_Names);
    emit countChanged(m_Names.size());
    return true;
  }

  return false;
}

QUrl ImageSet::urlFromName(const QString& name) const
{
  if (!isValid() || name.isEmpty())
    return QUrl();

  return QUrl::fromLocalFile(QDir(basePath()).absoluteFilePath(name));
}

}
