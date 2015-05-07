#include <Recon/ImageListModel.h>
#include <Recon/Document.h>
#include <QGuiApplication>
#include <QtQml>
#include <QQmlApplicationEngine>

int main(int argc, char* argv[])
{
  QGuiApplication app(argc, argv);

  //qmlRegisterType<recon::ImageListModel>("recon", 1, 0, "ImageListModel");
  qmlRegisterType<recon::Document>("recon", 1, 0, "ReconDocument");

  QQmlApplicationEngine engine(QUrl(QStringLiteral("qrc:///MainWindow.qml")));

  return app.exec();
}
