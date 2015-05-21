#include "VoxelColoring.h"

#include <QCommandLineParser>
#include <QCoreApplication>

#include <stdlib.h>
#include <iostream>

int main(int argc, char* argv[])
{
  QCoreApplication app(argc, argv);
  QCoreApplication::setApplicationName("voxel");
  QCoreApplication::setApplicationVersion("1.0");

  QCommandLineParser parser;
  parser.setApplicationDescription("Voxel Coloring");
  parser.addHelpOption();
  parser.addVersionOption();
  parser.addPositionalArgument("bundle", "Input bundle file");

  QCommandLineOption writePNGOption(QStringList() << "p" << "png", "Write PNG files", "directory");
  parser.addOption(writePNGOption);

  parser.process(app);

  const QStringList args = parser.positionalArguments();
  if (args.count() < 1) {
    std::cout << "Bundle path?\n";
    return 0;
  }

  const QString bundlePath = args.at(0);

  voxel::VoxelColoring coloring(bundlePath);
  if (coloring.process()) {
    std::cout << "ok\n";
  } else {
    std::cout << "failed\n";
  }

  if (parser.isSet(writePNGOption)) {
    coloring.save_to_png_set(parser.value(writePNGOption));
  }

  return 0;
}
