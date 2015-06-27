#include <recon/CameraLoader.h>
#include <recon/GraphCut.h>
#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDir>
#include <QString>
#include <QtDebug>
#include <QImage>
#include <stdlib.h>
#include <iostream>

int main(int argc, char* argv[])
{
  QCoreApplication app(argc, argv);
  QCoreApplication::setApplicationName("ncc-curve");
  QCoreApplication::setApplicationVersion("1.0");

  QCommandLineParser parser;
  parser.setApplicationDescription("Visualize NCC values");
  parser.addHelpOption();
  parser.addVersionOption();
  parser.addPositionalArgument("bundle", "Input bundle file");

  QCommandLineOption optPlaneY(QStringList() << "py" << "plane-y", "Plane Y", "plane_y");
  QCommandLineOption optCamI(QStringList() << "ci" << "cam-i", "Camera i", "cam_i");
  QCommandLineOption optCamJ(QStringList() << "cj" << "cam-j", "Camera j", "cam_j");
  //QCommandLineOption optGnuplot("gnuplot", "Show GNUPlot Window");
  optPlaneY.setDefaultValue("30");
  optCamI.setDefaultValue("34");
  optCamJ.setDefaultValue("36");
  parser.addOption(optPlaneY);
  parser.addOption(optCamI);
  parser.addOption(optCamJ);

  parser.process(app);

  const QStringList args = parser.positionalArguments();
  if (args.count() < 1) {
    std::cout << "Bundle path?\n";
    return 0;
  }

  const QString bundlePath = args.at(0);

  recon::CameraLoader loader;
  if (!loader.load_from_nvm(bundlePath)) {
    qDebug() << "Cannot load cameras from " << bundlePath;
    return 1;
  }

  QList<recon::Camera> cameras = loader.cameras();

  QStringList mask_paths;
  mask_paths.reserve(cameras.size());
  for (recon::Camera cam : cameras) {
    QString path =  cam.imagePath();
    QString rootname = path.section(QDir::separator(), 0, -3, QString::SectionIncludeLeadingSep);
    QString filename = path.section(QDir::separator(), -1);
    QString mpath = rootname + QString(QDir::separator()) + "masks" + QString(QDir::separator()) + filename;
    cam.setMaskPath(mpath);
  }

  //for (recon::Camera cam : cameras) {
  //  qDebug() << "mask path = " << cam.maskPath();
  //}

  recon::VoxelModel model(7, loader.model_boundingbox());

  int plane_y = parser.value(optPlaneY).toInt();
  int cam_i = parser.value(optCamI).toInt();
  int cam_j = parser.value(optCamJ).toInt();

  QImage img = recon::ncc_image(model, cameras, plane_y, cam_i, cam_j);
  img.save(QString("ncc-%1-%2.png").arg(cam_i).arg(cam_j));

  return 0;
}
