#include <recon/CameraLoader.h>
#include <recon/BuildGraph.h>
#include "../src/PhotoConsistency.h"
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
  QCoreApplication::setApplicationName("build-graph");
  QCoreApplication::setApplicationVersion("1.0");

  QCommandLineParser parser;
  parser.setApplicationDescription("Build a graph to optimize");
  parser.addHelpOption();
  parser.addVersionOption();
  parser.addPositionalArgument("bundle", "Input bundle file");
  parser.addPositionalArgument("data", "Output text file");

  QCommandLineOption optLevel(QStringList() << "l" << "level", "Level", "level");
  optLevel.setDefaultValue("7");
  parser.addOption(optLevel);

  QCommandLineOption optThreshold(QStringList() << "t" << "threshold", "Threshold of Voting", "threshold");
  parser.addOption(optThreshold);
  QCommandLineOption optDisableAutoThreshold("no-auto-threshold", "Disable Automatic Thresholding");
  parser.addOption(optDisableAutoThreshold);

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

  using recon::PhotoConsistency;
  PhotoConsistency::EnableAutoThresholding = !parser.isSet(optDisableAutoThreshold);
  if (parser.isSet(optThreshold))
    PhotoConsistency::VotingThreshold = parser.value(optThreshold).toDouble();

  recon::VoxelModel model(level, loader.model_boundingbox());
  recon::VoxelGraph graph;
  recon::build_graph(graph, model, cameras);
  recon::save_graph(graph, outputPath);
  return 0;
}
