#include "CameraLoader.h"
#include "GraphCut.h"

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
  parser.setApplicationDescription("Model Generator");
  parser.addHelpOption();
  parser.addVersionOption();
  parser.addPositionalArgument("bundle", "Input bundle file");

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
  //recon::VoxelList vlist = graph_cut(model, cameras);
  //recon::save_ply("voxels.ply", model, vlist);

  //QImage img = recon::ncc_image(model, cameras, 30, 35, 36);
  //img.save("ncc-35-36.png");

#if true
  QList<QPointF> data;
  int vx = 55, vy = 30, vz = 58;
  int cam_i = 34, cam_j = 36;
  //
  //
  data = depth_curve(model, cameras, vx, vy, vz, cam_i, cam_j);
  printf("# X Y\n");
  for (QPointF p : data){
    printf("%.10g %.10g\n", p.x(), p.y());
  }
#endif

#if 0
  recon::point3 model_center = model.real_box.center();
  for (int i = 0; i < cameras.size(); ++i) {
    for (int j = i+1; j < cameras.size(); ++j) {
      float dp = (float)dot(normalize(cameras[i].center()-model_center), normalize(cameras[j].center()-model_center));
      if (dp >= 0.95f) {
        printf("ncc for %d & %d\n", i, j);
        QImage img = recon::ncc_image(model, cameras, 30, i, j);
        img.save(QString("ncc-%1-%2-%3.png").arg(i).arg(j).arg(dp));
      }
    }
  }
#endif

#if 0
  for (int i = 0; i < cameras.size(); ++i) {
    printf("process vote-%d\n", i);
    QImage img = recon::vote_image(model, cameras, 30, i);
    img.save(QString("vote-%1.png").arg(i));
  }
#endif

  return 0;
}
