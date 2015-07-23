#include <recon/VoxelModel.h>
#include <recon/Skeleton.h>
#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDir>
#include <QString>
#include <QtDebug>
#include <QFile>
#include <QTextStream>
#include <stdlib.h>
#include <iostream>

int main(int argc, char* argv[])
{
  QCoreApplication app(argc, argv);
  QCoreApplication::setApplicationName("skeleton");
  QCoreApplication::setApplicationVersion("1.0");

  QCommandLineParser parser;
  parser.setApplicationDescription("Import Skeleton");
  parser.addHelpOption();
  parser.addVersionOption();
  parser.addPositionalArgument("skel", "Input skeleton file");
  parser.addPositionalArgument("output", "Output file");
  QCommandLineOption optSkelLevel("skel-level", "Level of skeleton file", "skel-level");
  parser.addOption(optSkelLevel);
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
  using recon::SkeletonField;

  int level = parser.value(optSkelLevel).toInt();
  VoxelModel model(level, AABox(Point3::zero(), Point3(1.0f, 1.0f, 1.0f)));
  VoxelList vlist;

  SkeletonField skel(level);
  if (!skel.load(inputPath))
    return 1;

  //for (uint64_t m = 0; m < model.morton_length; ++m) {
  //  if (skel.skeleton[(uint)m])
  //    vlist.append(m);
  //}
  vlist = skel.skeleton;

  recon::save_cubes_ply(outputPath, model, vlist);

  return 0;
}
