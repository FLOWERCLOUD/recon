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
  parser.addPositionalArgument("output", "Output PLY file");
  parser.process(app);

  const QStringList args = parser.positionalArguments();
  if (args.count() < 2) {
    std::cout << "Graph path and output path?\n";
    return 0;
  }

  const QString graphPath = args.at(0);
  const QString optimizePath = args.at(1);

  //for (recon::Camera cam : cameras) {
  //  qDebug() << "mask path = " << cam.maskPath();
  //}

  //recon::VoxelModel model(5, loader.model_boundingbox());
  recon::VoxelGraph graph;
  recon::VoxelList vlist = graph_cut(graph);
  //recon::save_ply("voxels.ply", model, vlist);

  return 0;
}
