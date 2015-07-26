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
#include <algorithm>
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
using recon::PhotoConsistency;
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
  QCommandLineOption optVoxelY(QStringList() << "y" << "voxel-y", "Voxel Y", "voxel-y");
  parser.addOption(optVoxelY);
  QCommandLineOption optCamI(QStringList() << "i" << "cam-i", "Camera I", "cam-i");
  parser.addOption(optCamI);
  QCommandLineOption optUseXY("use-xy", "Use XY Plane");
  parser.addOption(optUseXY);
  QCommandLineOption optThreshold(QStringList() << "t" << "threshold", "Threshold of Voting", "threshold");
  parser.addOption(optThreshold);
  QCommandLineOption optDisableAutoThreshold("no-auto-threshold", "Disable Automatic Thresholding");
  parser.addOption(optDisableAutoThreshold);

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
  float voxel_y = parser.value(optVoxelY).toFloat();
  int cam_i = (parser.isSet(optCamI) ? parser.value(optCamI).toInt() : -1);

  PhotoConsistency::EnableAutoThresholding = !parser.isSet(optDisableAutoThreshold);
  if (parser.isSet(optThreshold))
    PhotoConsistency::VotingThreshold = parser.value(optThreshold).toDouble();

  QList<Camera> cameras = loader.cameras();
  VoxelModel model(level, loader.model_boundingbox());
  PhotoConsistency pcs(model, cameras);

  // Create Score Image
  cv::Mat canvas = cv::Mat::zeros(model.width, model.width, CV_32FC1);

  printf("Construct score image...\n");
  for (uint32_t i = 0, w = model.width; i < w; ++i) {
    for (uint32_t j = 0; j < w; ++j) {
      Point3 pos;
      if (parser.isSet(optUseXY)) {
        pos = model.virtual_box.lerp(j/float(model.width),
                                     i/float(model.height),
                                     voxel_y/float(model.depth));
      } else {
        pos = model.virtual_box.lerp(j/float(model.width),
                                     voxel_y/(model.height),
                                     i/float(model.depth));
      }
      printf("Computing... %.2f %%\n", float(i*w+j)/float(w*w)*100.0f);
      if (cam_i >= 0) {
        Score score(pcs.cameras, pcs.images, cam_i, pos, pcs.voxel_size);
        canvas.at<float>(i,j) = std::max(score.vote(), 0.0);
      } else {
        canvas.at<float>(i,j) = std::max(pcs.vote(pos), 0.0);
      }
    }
  }
  printf("\n");

  // Visualize
  {
    QProcess proc;
    proc.setProcessChannelMode(QProcess::ForwardedChannels);
    proc.start("python2.7");
    if (proc.waitForStarted()) {
      QTextStream stream(&proc);
      stream.setRealNumberNotation(QTextStream::ScientificNotation);
      stream.setRealNumberPrecision(15);
      stream << "import numpy as np, matplotlib.pyplot as plt\n"
             << "data = np.array([\n";
      for (uint32_t i = 0, w = model.width; i < w; ++i) {
        stream << "[";
        for (uint32_t j = 0; j < w; ++j) {
          stream << "float(\"" << canvas.at<float>(i,j) << "\")"
                 << (j==w-1 ? "\n" : ",\n");
        }
        stream << "]" << (i==w-1 ? "\n" : ",\n");
      }
      stream << "])\n"
             << "plt.figure()\n";
      if (parser.isSet(optUseXY)) {
        stream << "plt.axis([0,data.shape[1],data.shape[0],0])\n";
      } else {
        stream << "plt.axis([0,data.shape[1],0,data.shape[0]])\n";
      }
      stream << "plt.imshow(data, interpolation='nearest', cmap='jet')\n"
             << "plt.colorbar()\n"
             << "plt.show()\n";
      stream.flush();
      proc.closeWriteChannel();
      proc.waitForFinished(-1);
    }
  }

  return 0;
}
