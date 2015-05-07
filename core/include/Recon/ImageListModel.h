#pragma once

#include <QAbstractListModel>
#include <QList>
#include <QUrl>
//#include <QImage>
//#include <QTemporaryDir>

namespace recon {

class ImageListModel : public QAbstractListModel {
  Q_OBJECT
  Q_PROPERTY(int count READ count NOTIFY countChanged)

public:
  enum ImageRoles {
    SourcePathRole = Qt::UserRole + 1,
    //ThumbnailPathRole,
  };

public:
  ImageListModel(QObject* parent = 0);
  virtual ~ImageListModel() override;

  Q_INVOKABLE void addImageSource(const QUrl& path);
  /* Q_PROPERTY */ int count() const;

  virtual int rowCount(const QModelIndex& parent) const override;
  virtual QVariant data(const QModelIndex& index, int role) const override;
  virtual QHash<int,QByteArray> roleNames() const override;

signals:
  void countChanged(int);

private:
  int m_RowCount;
  QList<QUrl> m_Paths;
  //QList<QUrl> m_ThumbnailPaths;
  //QTemporaryDir m_ThumbnailDir;
};

// http://doc.qt.io/qt-5/qtqml-cppintegration-definetypes.html
// http://doc.qt.io/qt-5/qtquick-modelviewsdata-cppmodels.html

}
