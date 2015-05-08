#include <Recon/Document.h>
#include <QDir>
#include <QFile>
#include <QUuid>
#include <QTextStream>
//#include <QtDebug>

namespace recon {

Document::Document(QObject* parent)
: QObject(parent)
{
  //qDebug("Document is created");

  connect(this, &Document::baseUrlChanged,
          this, &Document::onBaseUrlChanged,
          Qt::DirectConnection);

  connect(this, &Document::imageAdded,
          this, &Document::onImageAdded,
          Qt::DirectConnection);
}

Document::~Document()
{
  //qDebug("Document is destroyed");
}

bool Document::isValid() const
{
  return m_BaseUrl.isValid();
}

QUrl Document::baseUrl() const
{
  return m_BaseUrl;
}

QString Document::basePath() const
{
  return baseUrl().toLocalFile();
}

void Document::setBaseUrl(const QUrl& url)
{
  if (m_BaseUrl.isValid())
    return;

  //qDebug() << url.toLocalFile() << ", " << url.toString();
  //qDebug() << QUrl::fromLocalFile("tmp");

  if (url.isValid() && url.isLocalFile()) {
    QDir dir(url.toLocalFile());
    dir.makeAbsolute();

    if (!dir.exists()) {
      if (!dir.mkpath("images")) {
        return;
      }
    }

    m_BaseUrl = QUrl::fromLocalFile(dir.path());
    emit baseUrlChanged(m_BaseUrl);
  }
}

const QVector<Camera>& Document::cameras() const
{
  // TODO: cached + lazy initialization
  return m_Cameras;
}

const QVector<Feature>& Document::features() const
{
  // TODO: cached + lazy initialization
  return m_Features;
}

int Document::imageCount() const
{
  return m_ImageUrls.size();
}

QList<QUrl> Document::imageUrls() const
{
  return m_ImageUrls;
}

bool Document::importImage(const QUrl& url)
{
  if (!isValid() || !url.isLocalFile())
    return false;

  if (!url.toString().endsWith(".jpg", Qt::CaseInsensitive))
    return false;

  QDir dir(basePath());
  dir.cd("images");

  QUuid uuid;
  QString name;
  do {
    uuid = QUuid::createUuid();
    name = QString("%1.jpg").arg(uuid.toString());
  } while (dir.exists(name));

  QUrl newurl = QUrl::fromLocalFile(dir.absoluteFilePath(name));

  if (QFile::copy(url.toLocalFile(), newurl.toLocalFile())) {
    m_ImageUrls << newurl;
    emit imageAdded(newurl);
    return true;
  }

  return false;
}

void Document::onBaseUrlChanged(QUrl url)
{
  emit basePathChanged(url.toLocalFile());
}

void Document::onImageAdded(QUrl url)
{
  emit imageUrlsChanged(m_ImageUrls);
  emit imageCountChanged(m_ImageUrls.size());
}

/*
void Document::reload()
{
}

void Document::save()
{
  QDir dir(basePath());
  if (!dir.exists()) {
    dir.mkpath(".");
  }

  // Save cameras
  {
    QFile file(dir.filePath("cameras.txt"));
    file.open(QIODevice::WriteOnly);

    QTextStream stream(&file);
    for (auto it = m_Cameras.cbegin(), itend = m_Cameras.cend(); it != itend; ++it) {
      stream << it->index << " "
             << it->focal_length << " "
             << it->radial_distortion[0] << " "
             << it->radial_distortion[1] << " ";
      for (int i = 0; i < 3; ++i) {
        stream << it->center[i] << " ";
      }
      for (int i = 0; i < 9; ++i) {
        stream << it->intrinsic[i] << " ";
      }
      for (int i = 0; i < 12; ++i) {
        stream << it->extrinsic[i] << " ";
      }
      stream << "\n";
    }

    file.close();
  }

  // Save Features
  {
    QFile file(dir.filePath("features.txt"));
    file.open(QIODevice::WriteOnly);

    QTextStream stream(&file);
    for (auto it = m_Features.cbegin(), itend = m_Features.cend(); it != itend; ++it) {
      for (int i = 0; i < 3; ++i) {
        stream << it->pos[i] << " ";
      }
      for (int i = 0; i < 3; ++i) {
        stream << it->color[i] << " ";
      }
      stream << "\n";
    }

    file.close();
  }
}*/

void Document::swapCameras(QVector<Camera>& v)
{
  m_Cameras.swap(v);
}

void Document::swapFeatures(QVector<Feature>& v)
{
  m_Features.swap(v);
}

}
