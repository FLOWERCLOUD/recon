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

  QCommandLineOption optVoxelX(QStringList() << "vx" << "voxel-x", "Voxel X", "voxel_x");
  QCommandLineOption optVoxelY(QStringList() << "vy" << "voxel-y", "Voxel Y", "voxel_y");
  QCommandLineOption optVoxelZ(QStringList() << "vz" << "voxel-z", "Voxel Z", "voxel_z");
  QCommandLineOption optCamI(QStringList() << "ci" << "cam-i", "Camera i", "cam_i");
  QCommandLineOption optCamJ(QStringList() << "cj" << "cam-j", "Camera j", "cam_j");
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

  // --vx 55 --vy 30 --vz 58 --ci 34 --cj 36

  //int vx = 55, vy = 30, vz = 58;
  //int cam_i = 34, cam_j = 36;
  int vx = parser.value(optVoxelX).toInt();
  int vy = parser.value(optVoxelY).toInt();
  int vz = parser.value(optVoxelZ).toInt();
  int cam_i = parser.value(optCamI).toInt();
  int cam_j = parser.value(optCamJ).toInt();

  QList<QPointF> data = ncc_curve(model, cameras, vx, vy, vz, cam_i, cam_j);
  QList<QPointF> d_data, d2_data;

  // Derivative
  for (int i = 0, n = data.size(); i < n; ++i) {
    int i1 = (i > 0 ? i-1 : 0);
    int i2 = (i < n-1 ? i+1 : i);
    float y = (data[i2].y() - data[i1].y()) / (data[i2].x() - data[i1].x());
    float x = data[i].x();
    d_data.append(QPointF(x, y));
  }

  // 2nd-order derivative
  for (int i = 0, n = data.size(); i < n; ++i) {
    int i1 = (i > 0 ? i-1 : 0);
    int i2 = (i < n-1 ? i+1 : i);

    float dx2 = data[i2].x() - data[i].x();
    float dx1 = data[i].x() - data[i1].x();
    float dx = data[i2].x() - data[i1].x();

    float y = dx1 * data[i2].y() - dx * data[i].y() + dx2 * data[i1].y();
    y /= 0.5f * dx2 * dx1 * dx;

    float x = data[i].x();
    d2_data.append(QPointF(x, y));
  }

  printf("# NCC Data (index 0)");
  printf("# X Y\n");
  for (QPointF p : data){
    printf("%.10g %.10g\n", p.x(), p.y());
  }
  printf("\n\n");

  printf("# 1st-order Derivative of NCC (index 1)\n");
  for (QPointF p : d_data){
    printf("%.10g %.10g\n", p.x(), p.y());
  }
  printf("\n\n");

  printf("# 2nd-order Derivative of NCC (index 2)\n");
  for (QPointF p : d2_data){
    printf("%.10g %.10g\n", p.x(), p.y());
  }

  return 0;
}
