#include <recon/CameraLoader.h>
#include <recon/VoxelModel.h>
#include "../src/VoxelScore1.h"
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
  QCoreApplication::setApplicationName("vote_test");
  QCoreApplication::setApplicationVersion("1.0");

  QCommandLineParser parser;
  parser.setApplicationDescription("Test Voting");
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

  QList<Camera> cameras = loader.cameras();
  VoxelModel model(level, loader.model_boundingbox());
  recon::PhotoConsistency<Score> pcs(model, cameras);

  // Transform Points
  //float voxel_h = (float)model.virtual_box.extent().x() / model.width;
  Point3 voxel_pos = model.virtual_box.lerp(voxel_x/(model.width),
                                            voxel_y/(model.height),
                                            voxel_z/(model.depth));
  //Vec3 voxel_dir = normalize(cameras[cam_i].center() - voxel_pos) * 1.4f * voxel_h;
  //Vec3 vpos_i = Vec3::proj(transform(txfm_i, voxel_pos));

  // Visualize Vote Curve
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
      for (int i = 0; i < cameras.size(); ++i) {
        stream << "[" << i << ", float(\""
               << Score(pcs.cameras, pcs.images, i, voxel_pos, pcs.voxel_size).compute(0.0f)
               << "\")],\n";
      }
      stream << "])\n"
             << "plt.figure()\n"
             << "plt.plot(data[:,0], data[:,1],'-o')\n"
             << "plt.show()\n";
      stream.flush();
      proc.closeWriteChannel();
      proc.waitForFinished(-1);
    }
  }

  return 0;
}
