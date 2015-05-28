//#include <Recon/SFMContext.h>
#include <QGuiApplication>
#include <QtQml>
#include <QQmlApplicationEngine>

int main(int argc, char* argv[])
{
  QGuiApplication app(argc, argv);

  //qmlRegisterType<recon::SFMContext>("recon", 1, 0, "ReconSFMContext");
  QQmlApplicationEngine engine(QUrl(QStringLiteral("qrc:///MainWindow.qml")));

  return app.exec();
}
