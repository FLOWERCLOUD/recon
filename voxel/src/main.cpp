#include "CameraData.h"
#include "CameraLoader.h"

#include <QCommandLineParser>
#include <QCoreApplication>

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

  QStringList images;
  voxel::CameraList cameras;

  if (voxel::load_from_nvm(images, cameras, bundlePath)) {
    int n = cameras.size();
    std::cout << "# of cameras = " << n << "\n";
    for (int i = 0; i < n; ++i) {
      std::cout << "image = " << images[i].toStdString() << "\n";
    }
  } else {
    std::cout << "failed to open " << bundlePath.toStdString() << std::endl;
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
  parser.setApplicationDescription("VisualHull + VoxelColoring");
  parser.addHelpOption();
  parser.addVersionOption();
  parser.addPositionalArgument("bundle", "Input bundle file");
}
