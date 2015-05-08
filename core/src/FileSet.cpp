#include <Recon/FileSet.h>
#include <QDir>

namespace recon {

FileSet::FileSet(QObject* parent)
: QObject(parent)
{
  connect(this, &FileSet::baseUrlChanged,
          this, &FileSet::onBaseUrlChanged,
          Qt::DirectConnection);
}

FileSet::~FileSet()
{
}

bool FileSet::isValid() const
{
  return m_BaseUrl.isValid();
}

QUrl FileSet::baseUrl() const
{
  return m_BaseUrl;
}

QString FileSet::basePath() const
{
  return baseUrl().toLocalFile();
}

void FileSet::setBaseUrl(const QUrl& url)
{
  if (url.isValid() && url.isLocalFile()) {
    QDir dir(url.toLocalFile());
    dir.makeAbsolute();

    if (!dir.exists()) {
      if (!dir.mkpath(".")) {
        return;
      }
    }

    m_BaseUrl = QUrl::fromLocalFile(dir.path());
    emit baseUrlChanged(m_BaseUrl);
  }
}

void FileSet::onBaseUrlChanged(QUrl url)
{
  emit basePathChanged(url.toLocalFile());
}

}
