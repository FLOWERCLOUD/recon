#include <recon/VoxelModel.h>
#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDir>
#include <QString>
#include <QtDebug>
#include <QImage>
#include <QFile>
#include <QTextStream>
#include <stdlib.h>
#include <iostream>

struct VoxelData {
  int disjoint;
};

int main(int argc, char* argv[])
{
  QCoreApplication app(argc, argv);
  QCoreApplication::setApplicationName("repair-voxel");
  QCoreApplication::setApplicationVersion("1.0");

  QCommandLineParser parser;
  parser.setApplicationDescription("Repair voxels");
  parser.addHelpOption();
  parser.addVersionOption();
  parser.addPositionalArgument("voxels", "Input voxel list file");
  parser.addPositionalArgument("output", "Output voxel list file");
  //QCommandLineOption optLambda(QStringList() << "l" << "lambda", "Lambda", "lambda");
  //optLambda.setDefaultValue("0.5");
  //parser.addOption(optLambda);
  parser.process(app);

  const QStringList args = parser.positionalArguments();
  if (args.count() < 2) {
    std::cout << "Graph path and output path?\n";
    return 0;
  }

  const QString inputPath = args.at(0);
  const QString outputPath = args.at(1);

  using recon::Point3;
  using recon::AABox;
  using recon::VoxelModel;
  using recon::VoxelList;

  VoxelModel model(0, AABox());
  VoxelList vlist;
  if (!recon::load_voxels(model, vlist, inputPath)) {
    return 1;
  }

  //recon::save_points_ply(outputPath, model, vlist);
  recon::save_cubes_ply(outputPath, model, vlist);
  //recon::save_voxels(model, vlist, outputPath);

  return 0;
}
