#include <recon/CameraLoader.h>
#include <recon/VisualHull.h>
#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDir>
#include <QString>
#include <QtDebug>
#include <QFile>
#include <QTextStream>
#include <stdlib.h>
#include <iostream>
#include <vector>

static recon::VoxelList
trim_voxels(const recon::VoxelModel& model, const recon::VoxelList& vlist)
{
  using namespace recon;

  std::vector<bool> foreground(model.morton_length, false);
  for (uint64_t m : vlist) {
      foreground[m] = true;
  }

  VoxelList result;
  result.reserve(vlist.size());

  uint32_t w = model.width, h = model.height, d = model.depth;
  for (uint64_t m : vlist) {
    uint32_t x, y, z;
    morton_decode(m, x, y, z);
    bool surface =
      (x == 0 || !foreground[morton_encode(x-1,y,z)]) ||
      (x == w-1 || !foreground[morton_encode(x+1,y,z)]) ||
      (y == 0 || !foreground[morton_encode(x,y-1,z)]) ||
      (y == h-1 || !foreground[morton_encode(x,y+1,z)]) ||
      (z == 0 || !foreground[morton_encode(x,y,z-1)]) ||
      (z == d-1 || !foreground[morton_encode(x,y,z+1)]);
    if (surface)
      result.append(m);
  }

  return result;
}

int main(int argc, char* argv[])
{
  QCoreApplication app(argc, argv);
  QCoreApplication::setApplicationName("vishull");
  QCoreApplication::setApplicationVersion("1.0");

  QCommandLineParser parser;
  parser.setApplicationDescription("Construct a visual hull");
  parser.addHelpOption();
  parser.addVersionOption();
  parser.addPositionalArgument("bundle", "Input bundle file");
  parser.addPositionalArgument("output", "Output PLY file");

  QCommandLineOption optLevel(QStringList() << "l" << "level", "Level", "level");
  optLevel.setDefaultValue("7");
  parser.addOption(optLevel);
  QCommandLineOption optExportPLY(QStringList() << "p" << "ply", "Export PLY");
  parser.addOption(optExportPLY);
  parser.process(app);

  const QStringList args = parser.positionalArguments();
  if (args.count() < 2) {
    std::cout << "Bundle path and output path?\n";
    return 0;
  }

  const QString bundlePath = args.at(0);
  const QString outputPath = args.at(1);

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

  int level = parser.value(optLevel).toInt();
  printf("level = %d\n", level);

  recon::VoxelModel model(level, loader.model_boundingbox());
  recon::VoxelList vlist = recon::visual_hull(model, cameras);
  vlist = trim_voxels(model, vlist);

  //recon::VoxelModel model2(level, recon::AABox(recon::Point3::zero(), recon::Point3(1.0,1.0,1.0)));
  recon::save_points_ply(outputPath, model, vlist);

  return 0;
}
