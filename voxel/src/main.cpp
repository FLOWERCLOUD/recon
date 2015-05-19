#include "CameraData.h"
#include "CameraLoader.h"
#include "VoxelData.h"

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

  voxel::CameraLoader loader;

  // Load Bundle
  if (loader.load_from_nvm(bundlePath)) {
    int n = loader.cameras().size();
    std::cout << "# of cameras = " << n << "\n";
    for (int i = 0; i < n; ++i) {
      //std::cout << "image = " << images[i].toStdString() << "\n";
      std::cout << "cam[" << i << "] {\n"
                << "  image = " << loader.image_paths()[i].toStdString() << "\n"
                << "  aspect = " << loader.cameras()[i].aspect_ratio << "\n"
                << "}\n";
    }
    voxel::AABB aabb = loader.feature_bounding();
    std::cout << "feature AABB = ("
              << aabb.minpos[0] << ", " << aabb.minpos[1] << ", " << aabb.minpos[2]
              << ") to ("
              << aabb.maxpos[0] << ", " << aabb.maxpos[1] << ", " << aabb.maxpos[2]
              << ")\n";
  } else {
    std::cout << "failed to open " << bundlePath.toStdString() << std::endl;
  }

  // Voxel Data
  voxel::VoxelData voxels;
  voxels.width = 128;
  voxels.stride = sizeof(uint32_t);
  voxels.data = malloc(sizeof(uint32_t) * voxels.width * voxels.width * voxels.width);

  // Determine bounding box
  for (int i = 0, n = loader.cameras().size(); i < n; ++i) {
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
