#include <recon/GraphCut.h>

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
  parser.addPositionalArgument("graph", "Input graph file");
  parser.addPositionalArgument("output", "Output voxel list file");
  parser.process(app);

  const QStringList args = parser.positionalArguments();
  if (args.count() < 2) {
    std::cout << "Graph path and output path?\n";
    return 0;
  }

  const QString graphPath = args.at(0);
  const QString outputPath = args.at(1);

  recon::VoxelGraph graph;
  // TODO: load graph from file

  recon::VoxelList vlist = graph_cut(graph);
  recon::VoxelModel model(graph.level, recon::AABox(recon::Point3::zero(), recon::Point3(1.0,1.0,1.0)));
  recon::save_ply(outputPath, model, vlist);

  return 0;
}
