#include <recon/CameraLoader.h>
#include <recon/VoxelModel.h>
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

static void draw_boundingbox(cv::Mat& canvas, Mat4 txfm, const AABox& box)
{
  const Point3 pos[8] = {
    box.corner0(), box.corner1(), box.corner2(), box.corner3(),
    box.corner4(), box.corner5(), box.corner6(), box.corner7()
  };
  Vec3 vpos[8];

  for (int i = 0; i < 8; ++i)
    vpos[i] = Vec3::proj(transform(txfm, pos[i]));

  cv::Mat& cvs = canvas;
  cv::Scalar color(128,128,0);
  cv::Scalar colorx(0,0,255);
  cv::Scalar colory(0,255,0);
  cv::Scalar colorz(255,0,0);
  int t = 5;
  cv::line(cvs, to_cvPoint(vpos[0]), to_cvPoint(vpos[1]), colorx, t);
  cv::line(cvs, to_cvPoint(vpos[0]), to_cvPoint(vpos[2]), colory, t);
  cv::line(cvs, to_cvPoint(vpos[1]), to_cvPoint(vpos[3]), color, t);
  cv::line(cvs, to_cvPoint(vpos[2]), to_cvPoint(vpos[3]), color, t);
  cv::line(cvs, to_cvPoint(vpos[0]), to_cvPoint(vpos[4]), colorz, t);
  cv::line(cvs, to_cvPoint(vpos[1]), to_cvPoint(vpos[5]), color, t);
  cv::line(cvs, to_cvPoint(vpos[2]), to_cvPoint(vpos[6]), color, t);
  cv::line(cvs, to_cvPoint(vpos[3]), to_cvPoint(vpos[7]), color, t);
  cv::line(cvs, to_cvPoint(vpos[4]), to_cvPoint(vpos[5]), color, t);
  cv::line(cvs, to_cvPoint(vpos[4]), to_cvPoint(vpos[6]), color, t);
  cv::line(cvs, to_cvPoint(vpos[5]), to_cvPoint(vpos[7]), color, t);
  cv::line(cvs, to_cvPoint(vpos[6]), to_cvPoint(vpos[7]), color, t);
}

int main(int argc, char** argv)
{
  QCoreApplication app(argc, argv);
  QCoreApplication::setApplicationName("proj_test");
  QCoreApplication::setApplicationVersion("1.0");

  QCommandLineParser parser;
  parser.setApplicationDescription("Test Projection");
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

  // Render Bounding Box
  draw_boundingbox(img_i, txfm_i, model.virtual_box);
  draw_boundingbox(img_j, txfm_j, model.virtual_box);

  // Transform Points
  Point3 voxel_pos = model.virtual_box.lerp(voxel_x/(model.width),
                                            voxel_y/(model.height),
                                            voxel_z/(model.depth));
  //Point3 cam_i_pos = cameras[cam_i].center();
  Vec3 vpos_i = Vec3::proj(transform(txfm_i, voxel_pos));
  Vec3 vpos_j = Vec3::proj(transform(txfm_j, voxel_pos));
  //Vec3 cpos_j = Vec3::proj(transform(txfm_j, cam_i_pos));

  // Render Epipolar Line
  //cv::line(img_j, to_cvPoint(vpos_j), to_cvPoint(cpos_j), cv::Scalar(255,0,255), 5);
  // FIXME: since camera i might sit behind camera j, the projection will go wrong

  // Render Voxel Point
  cv::circle(img_i, to_cvPoint(vpos_i), 10, cv::Scalar(0,255,255), 5);
  cv::circle(img_j, to_cvPoint(vpos_j), 10, cv::Scalar(0,255,255), 5);

  cv::namedWindow("Image I", cv::WINDOW_NORMAL);
  cv::namedWindow("Image J", cv::WINDOW_NORMAL);
  cv::imshow("Image I", img_i);
  cv::imshow("Image J", img_j);

  while (true) {
    int key = cv::waitKey(0);
    if (key == 27 || key == -1)
      break;
  }
  return 0;
}
