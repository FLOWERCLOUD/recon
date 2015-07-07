#include <recon/CameraLoader.h>
#include <recon/BuildGraph.h>
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
  recon::VoxelGraph graph;
  recon::build_graph(graph, model, cameras);

  QFile outfile(outputPath);
  if (!outfile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
    qDebug() << "Cannot open output file: " << outputPath;
    return 1;
  }

  QTextStream stream(&outfile);
  stream.setRealNumberNotation(QTextStream::ScientificNotation);
  stream.setRealNumberPrecision(15);

  stream << graph.level << "\n"
         << graph.width << "\n"
         << graph.voxel_size << "\n"
         << graph.voxel_minpos[0] << " "
         << graph.voxel_minpos[1] << " "
         << graph.voxel_minpos[2] << "\n"
         << graph.voxel_maxpos[0] << " "
         << graph.voxel_maxpos[1] << " "
         << graph.voxel_maxpos[2] << "\n";

  for (uint64_t m = 0; m < model.morton_length; ++m) {
    uint32_t x, y, z;
    recon::morton_decode(m, x, y, z);
    stream << "(" << x << "," << y << "," << z << ") "
           << graph.foreground[m] << "\n";
  }
  for (uint64_t m = 0; m < model.morton_length; ++m) {
    uint32_t x, y, z;
    recon::morton_decode(m, x, y, z);
    stream << "(" << x << "," << y << "," << z << ") "
           << graph.x_edges[m] << " +x\n";
    stream << "(" << x << "," << y << "," << z << ") "
           << graph.y_edges[m] << " +y\n";
    stream << "(" << x << "," << y << "," << z << ") "
           << graph.z_edges[m] << " +z\n";
  }

  outfile.close();
  return 0;
}
