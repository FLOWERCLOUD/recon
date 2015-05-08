#include <Recon/FileSet.h>
#include <Recon/ImageSet.h>
#include <QGuiApplication>
#include <QtQml>
#include <QQmlApplicationEngine>

int main(int argc, char* argv[])
{
  QGuiApplication app(argc, argv);

  qmlRegisterType<recon::FileSet>("recon", 1, 0, "ReconFileSet");
  qmlRegisterType<recon::ImageSet>("recon", 1, 0, "ReconImageSet");

  QQmlApplicationEngine engine(QUrl(QStringLiteral("qrc:///MainWindow.qml")));

  return app.exec();
}
