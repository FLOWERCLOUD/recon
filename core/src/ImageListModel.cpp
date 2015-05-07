#include <Recon/ImageListModel.h>
#include <QtDebug>

namespace recon {

ImageListModel::ImageListModel(QObject* parent)
: QAbstractListModel(parent), m_RowCount(0)
{
}

ImageListModel::~ImageListModel()
{
}

void ImageListModel::addImageSource(const QUrl& path)
{
  beginInsertRows(QModelIndex(), m_RowCount, m_RowCount);
  m_Paths << path;
  m_RowCount += 1;
  endInsertRows();
  emit countChanged(m_RowCount);
}

int ImageListModel::rowCount(const QModelIndex& parent) const
{
  Q_UNUSED(parent);
  return m_RowCount;
}

QVariant ImageListModel::data(const QModelIndex& index, int role) const
{
  if (!index.isValid())
    return QVariant();

  if (index.row() >= m_RowCount)
    return QVariant();

  switch (role) {
    case SourcePathRole:
      return m_Paths.at(index.row());
    default:
      break;
  }

  return QVariant();
}

QHash<int,QByteArray> ImageListModel::roleNames() const
{
  QHash<int, QByteArray> roles;
  roles[SourcePathRole] = "sourcePath";
  return roles;
}

int ImageListModel::count() const
{
  return m_RowCount;
}

}
