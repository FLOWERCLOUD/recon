#include <recon/VoxelModel.h>
#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDir>
#include <QString>
#include <QtDebug>
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

  int level = parser.value(optSkelLevel).toInt();
  VoxelModel model(level, AABox(Point3::zero(), Point3(1.0f, 1.0f, 1.0f)));
  VoxelList vlist;

  QFile file(inputPath);
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    qDebug() << "Cannot open input file!";
    return 1;
  }

  QTextStream stream(&file);
  while (!stream.atEnd()) {
    QString line = stream.readLine();
    if (line.length() == 0)
      continue;
    else if (line.startsWith("#")) // comment
      continue;

    float spx, spy, spz, dt;
    int sps;
    QTextStream(&line) >> spx >> spy >> spz >> sps >> dt;

    uint32_t x, y, z;
    x = roundf(spx);
    y = roundf(spy);
    z = roundf(spz);
    vlist.append(recon::morton_encode(x,y,z));
  }

  recon::save_cubes_ply(outputPath, model, vlist);

  return 0;
}
