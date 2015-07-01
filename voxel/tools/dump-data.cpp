#include <recon/CameraLoader.h>
#include <recon/GraphCut.h>
#include <recon/numerical_method.h>
#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDir>
#include <QString>
#include <QtDebug>
#include <QImageReader>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QFile>
#include <stdlib.h>
#include <iostream>

int main(int argc, char* argv[])
{
  QCoreApplication app(argc, argv);
  QCoreApplication::setApplicationName("dump-data");
  QCoreApplication::setApplicationVersion("1.0");

  QCommandLineParser parser;
  parser.setApplicationDescription("Dump data for Python scripts");
  parser.addHelpOption();
  parser.addVersionOption();
  parser.addPositionalArgument("bundle", "Input bundle file");
  parser.addPositionalArgument("data", "Output JSON data file");

  QCommandLineOption optLevel(QStringList() << "l" << "level", "Level", "level");
  optLevel.setDefaultValue("7");
  parser.addOption(optLevel);
  parser.process(app);

  const QStringList args = parser.positionalArguments();
  if (args.count() < 2) {
    std::cout << "Bundle path and data path?\n";
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
  QJsonObject rootobj;

  // Extract cameras
  {
    QJsonArray cams;
    for (int i = 0, n = cameras.size(); i < n; ++i) {
      QJsonObject c;
      { // Center
        recon::Point3 pos = cameras[i].center();
        QJsonArray jpos = { QJsonValue((float)pos.x()),
                            QJsonValue((float)pos.y()),
                            QJsonValue((float)pos.z()) };
        c.insert("center", jpos);
      }
      { // Rotation Matrix
        recon::Mat3 mat = cameras[i].rotation();
        QJsonArray jmat = {
          QJsonArray { QJsonValue((float)mat.column0().x()),
                       QJsonValue((float)mat.column1().x()),
                       QJsonValue((float)mat.column2().x()) },
          QJsonArray { QJsonValue((float)mat.column0().y()),
                       QJsonValue((float)mat.column1().y()),
                       QJsonValue((float)mat.column2().y()) },
          QJsonArray { QJsonValue((float)mat.column0().z()),
                       QJsonValue((float)mat.column1().z()),
                       QJsonValue((float)mat.column2().z()) }
        };
        c.insert("rotation", jmat);
      }
      { // Extrinsic Matrix
        recon::Mat4 mat = cameras[i].extrinsic();
        QJsonArray jmat = {
          QJsonArray { QJsonValue((float)mat.column0().x()),
                       QJsonValue((float)mat.column1().x()),
                       QJsonValue((float)mat.column2().x()),
                       QJsonValue((float)mat.column3().x()) },
          QJsonArray { QJsonValue((float)mat.column0().y()),
                       QJsonValue((float)mat.column1().y()),
                       QJsonValue((float)mat.column2().y()),
                       QJsonValue((float)mat.column3().y()) },
          QJsonArray { QJsonValue((float)mat.column0().z()),
                       QJsonValue((float)mat.column1().z()),
                       QJsonValue((float)mat.column2().z()),
                       QJsonValue((float)mat.column3().z()) },
          QJsonArray { QJsonValue((float)mat.column0().w()),
                       QJsonValue((float)mat.column1().w()),
                       QJsonValue((float)mat.column2().w()),
                       QJsonValue((float)mat.column2().w()) }
        };
        c.insert("extrinsic", jmat);
      }
      { // Intrinsic Matrix for Image
        QImageReader reader(cameras[i].imagePath());
        QSize dim = reader.size();
        recon::Mat4 mat = cameras[i].intrinsicForImage(dim.width(), dim.height());
        QJsonArray jmat = {
          QJsonArray { QJsonValue((float)mat.column0().x()),
                       QJsonValue((float)mat.column1().x()),
                       QJsonValue((float)mat.column2().x()),
                       QJsonValue((float)mat.column3().x()) },
          QJsonArray { QJsonValue((float)mat.column0().y()),
                       QJsonValue((float)mat.column1().y()),
                       QJsonValue((float)mat.column2().y()),
                       QJsonValue((float)mat.column3().y()) },
          QJsonArray { QJsonValue((float)mat.column0().z()),
                       QJsonValue((float)mat.column1().z()),
                       QJsonValue((float)mat.column2().z()),
                       QJsonValue((float)mat.column3().z()) },
          QJsonArray { QJsonValue((float)mat.column0().w()),
                       QJsonValue((float)mat.column1().w()),
                       QJsonValue((float)mat.column2().w()),
                       QJsonValue((float)mat.column2().w()) }
        };
        c.insert("intrinsic", jmat);
      }
      c.insert("image", QJsonValue(cameras[i].imagePath()));
      c.insert("mask", QJsonValue(cameras[i].maskPath()));
      cams.append(c);
    }
    rootobj.insert("cameras", cams);
  }

  // Extract model data
  {
    QJsonObject jmodel;
    jmodel.insert("level", QJsonValue(model.level));
    jmodel.insert("real_box", QJsonArray{
      QJsonArray{ QJsonValue((float)model.real_box.minpos.x()),
                  QJsonValue((float)model.real_box.minpos.y()),
                  QJsonValue((float)model.real_box.minpos.z())},
      QJsonArray{ QJsonValue((float)model.real_box.maxpos.x()),
                  QJsonValue((float)model.real_box.maxpos.y()),
                  QJsonValue((float)model.real_box.maxpos.z())},
    });
    jmodel.insert("virtual_box", QJsonArray{
      QJsonArray{ QJsonValue((float)model.virtual_box.minpos.x()),
                  QJsonValue((float)model.virtual_box.minpos.y()),
                  QJsonValue((float)model.virtual_box.minpos.z())},
      QJsonArray{ QJsonValue((float)model.virtual_box.maxpos.x()),
                  QJsonValue((float)model.virtual_box.maxpos.y()),
                  QJsonValue((float)model.virtual_box.maxpos.z())},
    });
    jmodel.insert("width", QJsonValue((int)model.width));
    jmodel.insert("height", QJsonValue((int)model.height));
    jmodel.insert("depth", QJsonValue((int)model.depth));
    rootobj.insert("model", jmodel);
  }

  // Finally, write to the file
  QFile outfile(outputPath);
  if (outfile.open(QFile::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
    outfile.write(QJsonDocument(rootobj).toJson());
    outfile.close();
  }
  return 0;
}
