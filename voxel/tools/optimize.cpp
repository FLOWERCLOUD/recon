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

static void decode_xyz(const QString& s, int& x, int& y, int& z)
{
  QStringRef s2 = s.midRef(1, s.size()-2);
  auto v = s2.split(',');
  x = v[0].toInt();
  y = v[1].toInt();
  z = v[2].toInt();
}

static bool load_graph(recon::VoxelGraph& graph, const QString& path)
{
  using namespace recon;

  QFile file(path);
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    qDebug() << "Cannot open graph file!";
    return false;
  }

  QTextStream stream(&file);

  stream >> graph.level
         >> graph.width
         >> graph.voxel_size
         >> graph.voxel_minpos[0]
         >> graph.voxel_minpos[1]
         >> graph.voxel_minpos[2]
         >> graph.voxel_maxpos[0]
         >> graph.voxel_maxpos[1]
         >> graph.voxel_maxpos[2];

  uint64_t length = graph.width * graph.width * graph.width;
  graph.foreground.resize(length, false);
  graph.x_edges.resize(length, 0.0);
  graph.y_edges.resize(length, 0.0);
  graph.z_edges.resize(length, 0.0);

  QString xyz, dir;
  for (uint64_t i = 0; i < length; ++i) {
    int x, y, z;
    int flag;
    stream >> xyz >> flag;
    decode_xyz(xyz, x, y, z);
    //printf("%d %d %d %d\n", x, y, z, flag);
    graph.foreground[morton_encode(x, y, z)] = flag;
  }

  for (uint64_t i = 0; i < length*3; ++i) {
    int x, y, z;
    double w;
    stream >> xyz >> w >> dir;
    decode_xyz(xyz, x, y, z);
    if (dir == "+x") {
      graph.x_edges[morton_encode(x, y, z)] = w;
    } else if (dir == "+y") {
      graph.y_edges[morton_encode(x, y, z)] = w;
    } else if (dir == "+z") {
      graph.z_edges[morton_encode(x, y, z)] = w;
    }
  }

  return true;
}

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
  if (!load_graph(graph, graphPath)) {
    return 1;
  }

  double lambda = parser.value(optLambda).toDouble();
  double mju = parser.value(optMju).toDouble();

  VoxelList vlist = graph_cut(graph, lambda, mju);
  VoxelModel model(graph.level, AABox(Point3::load(graph.voxel_minpos),
                                      Point3::load(graph.voxel_maxpos)));
  recon::save_points_ply(outputPath, model, vlist);

  return 0;
}
