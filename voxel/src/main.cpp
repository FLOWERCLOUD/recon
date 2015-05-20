#include "VoxelColoring.h"

#include <QCommandLineParser>
#include <QCoreApplication>

#include <stdlib.h>
#include <mutex>
#include <iostream>

static QCommandLineParser& cmdparse();

int main(int argc, char* argv[])
{
  QCoreApplication app(argc, argv);
  QCoreApplication::setApplicationName("voxel");
  QCoreApplication::setApplicationVersion("1.0");

  QCommandLineParser& parser = cmdparse();
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

  return 0;
}

static void _init_cmdparse(QCommandLineParser& parser);

static QCommandLineParser& cmdparse()
{
  static QCommandLineParser parser;
  static std::once_flag flag;

  std::call_once(flag, &_init_cmdparse, std::ref<QCommandLineParser>(parser));

  return parser;
}

static void _init_cmdparse(QCommandLineParser& parser)
{
  parser.setApplicationDescription("Voxel Coloring");
  parser.addHelpOption();
  parser.addVersionOption();
  parser.addPositionalArgument("bundle", "Input bundle file");
}
