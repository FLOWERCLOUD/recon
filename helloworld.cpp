#include <Bundle.h>
#include <QtCore>
#include <QtDebug>

int main(int argc, char* argv[])
{
  //QCoreApplication app(argc, argv);

  QString cwd = QDir::currentPath();
  qDebug() << "Current work directory = " << cwd;

  recon::Bundle bundle;
  {
    QFile file("data/e100vs/bundle.nvm");
    file.open(QIODevice::ReadOnly);
    bundle.load_nvm(&file);
    file.close();
  }
  qDebug() << bundle.camera_count() << " cameras";

  return 0;
  //return app.exec();
}
