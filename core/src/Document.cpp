#include <Recon/Document.h>
#include <QDir>
#include <QFile>
#include <QUuid>
#include <QTextStream>
//#include <QtDebug>

namespace recon {

Document::Document(QObject* parent)
: FileSet(parent), m_ImageUrls_InitFlag(true)
{
  //qDebug("Document is created");

  connect(this, &Document::imageAdded,
          this, &Document::onImageAdded,
          Qt::DirectConnection);
}

Document::~Document()
{
  //qDebug("Document is destroyed");
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
  ensureInitImageUrls();
  return m_ImageUrls.size();
}

QList<QUrl> Document::imageUrls() const
{
  ensureInitImageUrls();
  return m_ImageUrls;
}

void Document::ensureInitImageUrls() const
{
  if (m_ImageUrls_InitFlag && isValid()) {
    QDir dir(basePath());
    dir.cd("images");

    QStringList files = dir.entryList(QDir::Files);
    int n = files.size();

    m_ImageUrls.clear();
    m_ImageUrls.reserve(n);

    for (int i = 0; i < n; ++i) {
      m_ImageUrls << QUrl::fromLocalFile(dir.absoluteFilePath(files[i]));
    }

    m_ImageUrls_InitFlag = false;
  }
}

bool Document::importImage(const QUrl& url)
{
  if (!isValid() || !url.isLocalFile())
    return false;

  if (!url.toString().endsWith(".jpg", Qt::CaseInsensitive))
    return false;

  ensureInitImageUrls();

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
