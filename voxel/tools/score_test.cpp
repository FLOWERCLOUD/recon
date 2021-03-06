#include <recon/CameraLoader.h>
#include <recon/VoxelModel.h>
#include "../src/VoxelScore1.h"
//#include "../src/VoxelScore2.h"
#include "../src/PhotoConsistency.h"
#include <recon/Debug.h>
#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDir>
#include <QString>
#include <QtDebug>
#include <QFile>
#include <QTextStream>
#include <QProcess>
#include <stdlib.h>
#include <iostream>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

using recon::Vec3;
using recon::Vec4;
using recon::Point3;
using recon::Mat4;
using recon::Ray3;
using recon::AABox;
using recon::Camera;
using recon::CameraLoader;
using recon::VoxelModel;
using Score = recon::VoxelScore1;

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

  QList<Camera> cameras = loader.cameras();
  VoxelModel model(level, loader.model_boundingbox());
  recon::PhotoConsistency<Score> pcs(model, cameras);

  cv::Mat img_i = cv::imread(cameras[cam_i].imagePath().toStdString());
  //Mat4 txfm_i = cameras[cam_i].intrinsicForImage(img_i.cols, img_i.rows);
  //txfm_i = txfm_i * cameras[cam_i].extrinsic();

  // Transform Points
  //float voxel_h = (float)model.virtual_box.extent().x() / model.width;
  Point3 voxel_pos = model.virtual_box.lerp(voxel_x/(model.width),
                                            voxel_y/(model.height),
                                            voxel_z/(model.depth));
  //Vec3 voxel_dir = normalize(cameras[cam_i].center() - voxel_pos) * 1.4f * voxel_h;
  //Vec3 vpos_i = Vec3::proj(transform(txfm_i, voxel_pos));

  // Render Voxel Point
  //cv::circle(img_i, to_cvPoint(vpos_i), 10, cv::Scalar(0,255,255), 5);

  cv::namedWindow("Image I", cv::WINDOW_NORMAL);
  cv::imshow("Image I", img_i);

  // Voxel Score
  Score score(pcs.cameras, pcs.images, cam_i, voxel_pos, pcs.voxel_size);

  for (int i = 0, n = score.ccams.num; i < n; ++i) {
    int cam_j = score.ccams.cam_js[i];
    //Mat4 txfm_j = score.ccams.txfm_js[i];

    char cam_j_name[64];
    sprintf(cam_j_name, "cam_j[%d] = %d", i, cam_j);
    printf("%s\n", cam_j_name);

    cv::Mat img_j = cv::imread(cameras[cam_j].imagePath().toStdString());
    auto epipolar = score.make_epipolar(i);
    epipolar.per_pixel(
      [&img_j](Vec3 pt0, Vec3 pt1) {
        float depth = (float)pt0.z();
        int px = roundf((float)pt0.x()), py = roundf((float)pt0.y());
        if (px >= 0 && py >= 0 && px < img_j.cols && py < img_j.rows) {
          if (fabsf(depth) <= 1.0f)
            img_j.at<cv::Vec3b>(py, px) =  cv::Vec3b(0, 0, 255);
          else
            img_j.at<cv::Vec3b>(py, px) =  cv::Vec3b(0, 255, 0);
        }
      }
    );

    cv::namedWindow(cam_j_name, cv::WINDOW_NORMAL);
    cv::imshow(cam_j_name, img_j);
  }

  // Print vote
  printf("vote = %f\n", score.vote());

  // Visualize Score Curve
  if (score.ccams.num >= 1) {
    QProcess proc;
    proc.setProcessChannelMode(QProcess::ForwardedChannels);
    proc.start("python2.7");
    if (proc.waitForStarted()) {
      QTextStream stream(&proc);
      stream.setRealNumberNotation(QTextStream::ScientificNotation);
      stream.setRealNumberPrecision(15);
      stream << "import numpy as np, matplotlib.pyplot as plt\n"
             << "data = np.array([\n";
      auto epipolar = score.make_epipolar(0);
      epipolar.per_pixel(
        [&stream,&score](Vec3 pt0, Vec3 pt1) {
          float d = (float)pt0.z();
          stream << "[float(\"" << d
                 << "\"), float(\"" << score.compute(d)
                 << "\")],\n";
        }
      );
      stream << "])\n"
             << "plt.figure()\n"
             << "plt.plot(data[:,0], data[:,1])\n"
             << "plt.figure()\n"
             << "sel = np.abs(data[:,0]) < 2.0\n"
             << "plt.plot(data[sel][:,0], data[sel][:,1])\n"
             << "plt.show()\n";
      stream.flush();
      proc.closeWriteChannel();

      // OpenCV event loop
      while (true) {
        int key = cv::waitKey(0);
        if (key == 27 || key == -1)
          break;
      }
      proc.waitForFinished(-1);
    }
  }

  return 0;
}
