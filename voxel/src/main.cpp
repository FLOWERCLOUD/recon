#include "VoxelBlock.h"
#include "CameraLoader.h"
#include "VisualHull.h"
#include "Debug.h"
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
  parser.setApplicationDescription("Model Generator");
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

  recon::CameraLoader loader;
  if (!loader.load_from_nvm(bundlePath)) {
    qDebug() << "Cannot load cameras from " << bundlePath;
    return 1;
  }

  recon::VoxelBlockGenerator block_gen(loader.feature_boundingbox());

  recon::VoxelBlock block;
  while (block_gen.generate(block)) {
    qDebug() << block.origin[0] << ", " << block.origin[1] << ", " << block.origin[2];

    block.each_voxel([](uint64_t morton, recon::VoxelBlock* block){
    });
  }

  /*voxel::VoxelColoring coloring(bundlePath);
  if (coloring.process()) {
    std::cout << "ok\n";
  } else {
    std::cout << "failed\n";
  }

  if (parser.isSet(writePNGOption)) {
    coloring.save_to_png_set(parser.value(writePNGOption));
  }*/

  return 0;
}
