#include <recon/GraphCut.h>

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

int main(int argc, char* argv[])
{
  QCoreApplication app(argc, argv);
  QCoreApplication::setApplicationName("voxel");
  QCoreApplication::setApplicationVersion("1.0");

  QCommandLineParser parser;
  parser.setApplicationDescription("Model Generator");
  parser.addHelpOption();
  parser.addVersionOption();
  parser.addPositionalArgument("graph", "Input graph file");
  parser.addPositionalArgument("output", "Output voxel list file");

  QCommandLineOption optLambda(QStringList() << "l" << "lambda", "Lambda", "lambda");
  optLambda.setDefaultValue("0.5");
  parser.addOption(optLambda);
  QCommandLineOption optMju(QStringList() << "m" << "mju", "Mju", "mju");
  optMju.setDefaultValue("2.0");
  parser.addOption(optMju);

  parser.process(app);

  const QStringList args = parser.positionalArguments();
  if (args.count() < 2) {
    std::cout << "Graph path and output path?\n";
    return 0;
  }

  const QString graphPath = args.at(0);
  const QString outputPath = args.at(1);

  using recon::Point3;
  using recon::AABox;
  using recon::VoxelGraph;
  using recon::VoxelModel;
  using recon::VoxelList;

  VoxelGraph graph;
  if (!recon::load_graph(graph, graphPath)) {
    return 1;
  }

  double lambda = parser.value(optLambda).toDouble();
  double mju = parser.value(optMju).toDouble();

  VoxelList vlist = graph_cut(graph, lambda, mju);
  VoxelModel model(graph.level, AABox(Point3::load(graph.voxel_minpos),
                                      Point3::load(graph.voxel_maxpos)));
  recon::save_points_ply(outputPath, model, vlist);
  //recon::save_voxels(model, vlist, outputPath);

  return 0;
}
