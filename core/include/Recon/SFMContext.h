#pragma once

#include <QObject>
#include <QProcess>
#include <QTemporaryDir>

namespace recon {

class ImageSet;

class SFMContext : public QObject {
  Q_OBJECT
  Q_PROPERTY(recon::ImageSet* images READ images WRITE setImages)
  //Q_PROPERTY(CameraSet* cameras)
public:
  SFMContext(QObject* parent = 0);
  virtual ~SFMContext();

  ImageSet* images() const;
  void setImages(ImageSet*);

  void start();

signals:
  void finished();

private slots:
  void onProcFinished(int code,QProcess::ExitStatus status);

private:
  ImageSet* m_SourceImages;
  QProcess* m_Process;
  QTemporaryDir m_TempDir;
};

}
