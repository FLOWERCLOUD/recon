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
  QCoreApplication::setApplicationName("voxel");
  QCoreApplication::setApplicationVersion("1.0");

  QCommandLineParser parser;
  parser.setApplicationDescription("Visualize NCC values");
  parser.addHelpOption();
  parser.addVersionOption();
  parser.addPositionalArgument("bundle", "Input bundle file");

  QCommandLineOption optVoxelX("voxel_x", "Voxel X");
  QCommandLineOption optVoxelY("voxel_y", "Voxel Y");
  QCommandLineOption optVoxelZ("voxel_z", "Voxel Z");
  QCommandLineOption optCamI("cam_i", "Camera i");
  QCommandLineOption optCamJ("cam_j", "Camera j");
  //QCommandLineOption optGnuplot("gnuplot", "Show GNUPlot Window");
  optVoxelX.setDefaultValue("55");
  optVoxelY.setDefaultValue("30");
  optVoxelZ.setDefaultValue("58");
  optCamI.setDefaultValue("34");
  optCamJ.setDefaultValue("36");
  parser.addOption(optVoxelX);
  parser.addOption(optVoxelY);
  parser.addOption(optVoxelZ);
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

  //int vx = 55, vy = 30, vz = 58;
  //int cam_i = 34, cam_j = 36;
  int vx = parser.value(optVoxelX).toInt();
  int vy = parser.value(optVoxelY).toInt();
  int vz = parser.value(optVoxelZ).toInt();
  int cam_i = parser.value(optCamI).toInt();
  int cam_j = parser.value(optCamJ).toInt();
  QList<QPointF> data = ncc_curve(model, cameras, vx, vy, vz, cam_i, cam_j);
  printf("# X Y\n");
  for (QPointF p : data){
    printf("%.10g %.10g\n", p.x(), p.y());
  }

  return 0;
}
