#include <recon/CameraLoader.h>
#include <recon/VoxelModel.h>
//#include <recon/Correlation.h>
#include "../src/Correlation.h"
#include <recon/Debug.h>
#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDir>
#include <QString>
#include <QtDebug>
#include <QFile>
#include <QTextStream>
#include <stdlib.h>
#include <iostream>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

using recon::Vec3;
using recon::Vec4;
using recon::Point3;
using recon::Mat4;
using recon::AABox;
using recon::Camera;
using recon::CameraLoader;
using recon::VoxelModel;

static inline cv::Point to_cvPoint(Vec3 v)
{
  return cv::Point((float)v.x(), (float)v.y());
}

int main(int argc, char** argv)
{
  QCoreApplication app(argc, argv);
  QCoreApplication::setApplicationName("ncc_test");
  QCoreApplication::setApplicationVersion("1.0");

  QCommandLineParser parser;
  parser.setApplicationDescription("Test Normalized Cross Correlation");
  parser.addHelpOption();
  parser.addVersionOption();
  parser.addPositionalArgument("bundle", "Input bundle file");

  QCommandLineOption optLevel(QStringList() << "l" << "level", "Level", "level");
  optLevel.setDefaultValue("7");
  parser.addOption(optLevel);
  QCommandLineOption optVoxelX(QStringList() << "x" << "voxel-x", "Voxel X", "voxel-x");
  parser.addOption(optVoxelX);
  QCommandLineOption optVoxelY(QStringList() << "y" << "voxel-y", "Voxel Y", "voxel-y");
  parser.addOption(optVoxelY);
  QCommandLineOption optVoxelZ(QStringList() << "z" << "voxel-z", "Voxel Z", "voxel-z");
  parser.addOption(optVoxelZ);
  QCommandLineOption optCamI(QStringList() << "i" << "cam-i", "Camera I", "cam-i");
  parser.addOption(optCamI);
  QCommandLineOption optCamJ(QStringList() << "j" << "cam-j", "Camera J", "cam-j");
  parser.addOption(optCamJ);

  parser.process(app);

  const QStringList args = parser.positionalArguments();
  if (args.count() < 1) {
    std::cout << "Bundle path?\n";
    return 1;
  }

  const QString bundlePath = args.at(0);
  CameraLoader loader;
  if (!loader.load_from_nvm(bundlePath)) {
    qDebug() << "Cannot load cameras from " << bundlePath;
    return 1;
  }

  int level = parser.value(optLevel).toInt();
  float voxel_x = parser.value(optVoxelX).toFloat();
  float voxel_y = parser.value(optVoxelY).toFloat();
  float voxel_z = parser.value(optVoxelZ).toFloat();
  int cam_i = parser.value(optCamI).toInt();
  int cam_j = parser.value(optCamJ).toInt();

  QList<Camera> cameras = loader.cameras();
  VoxelModel model(level, loader.model_boundingbox());

  cv::Mat img_i = cv::imread(cameras[cam_i].imagePath().toStdString());
  cv::Mat img_j = cv::imread(cameras[cam_j].imagePath().toStdString());

  Mat4 txfm_i = cameras[cam_i].intrinsicForImage(img_i.cols, img_i.rows);
  txfm_i = txfm_i * cameras[cam_i].extrinsic();

  Mat4 txfm_j = cameras[cam_j].intrinsicForImage(img_j.cols, img_j.rows);
  txfm_j = txfm_j * cameras[cam_j].extrinsic();

  // Transform Points
  Point3 voxel_pos = model.virtual_box.lerp(voxel_x/(model.width),
                                            voxel_y/(model.height),
                                            voxel_z/(model.depth));
  Vec3 vpos_i = Vec3::proj(transform(txfm_i, voxel_pos));
  Vec3 vpos_j = Vec3::proj(transform(txfm_j, voxel_pos));

  // Render Voxel Point
  cv::circle(img_i, to_cvPoint(vpos_i), 10, cv::Scalar(0,255,255), 5);
  cv::circle(img_j, to_cvPoint(vpos_j), 10, cv::Scalar(0,255,255), 5);

  cv::namedWindow("Image I", cv::WINDOW_NORMAL);
  cv::namedWindow("Image J", cv::WINDOW_NORMAL);
  cv::namedWindow("Sample I", cv::WINDOW_NORMAL);
  cv::namedWindow("Sample J", cv::WINDOW_NORMAL);
  cv::imshow("Image I", img_i);
  cv::imshow("Image J", img_j);

  while (cv::waitKey(0) != 27);
  return 0;
}
