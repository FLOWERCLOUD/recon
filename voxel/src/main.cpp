#include "CameraLoader.h"
#include "VoxelModel.h"
#include "morton_code.h"
#include <trimesh2/TriMesh.h>

#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDir>
#include <QString>
#include <QtDebug>

#include <QImage>

#include <stdlib.h>
#include <iostream>

static void visual_hull(recon::VoxelModel& model, const QList<recon::Camera>& cameras);
static void plane_sweep(recon::VoxelModel& model, const QList<recon::Camera>& cameras);
static void save_model(const QString& path, const recon::VoxelModel& model);

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

  //for (recon::Camera cam : cameras) {
  //  qDebug() << "mask path = " << cam.maskPath();
  //}

  recon::VoxelModel model(7, loader.model_boundingbox());
  visual_hull(model, cameras);
  plane_sweep(model, cameras);
  save_model("model.ply", model);

  return 0;
}

static void visual_hull(recon::VoxelModel& model, const QList<recon::Camera>& cameras)
{
  using recon::AABox;
  using recon::VoxelData;
  using recon::Camera;
  using recon::mat4;
  using recon::vec3;
  using recon::vec4;
  using recon::morton_decode;

  // Initialize
  for (uint64_t m = 0; m < model.size(); ++m) {
    uint32_t val = 0;
    val |= VoxelData::VISUALHULL_FLAG;
    model[m].flag = val;
  }

  // Visual Hull
  for (Camera cam : cameras) {
    QImage mask = QImage(cam.maskPath());

    mat4 extrinsic = cam.extrinsic();
    mat4 intrinsic = cam.intrinsicForImage(mask.width(), mask.height());
    mat4 transform = intrinsic * extrinsic;

    for (uint64_t m = 0; m < model.size(); ++m) {
      VoxelData& voxel = model[m];
      AABox vbox = model.boundingbox(m);
      vec3 pt = vbox.center();
      pt = proj_vec3(transform * vec4(pt, 1.0f));

      QPoint pt2d = QPoint((float)pt.x(), (float)pt.y());
      if (mask.valid(pt2d)) {
        if (qGray(mask.pixel(pt2d)) < 10) {
          voxel.flag &= ~VoxelData::VISUALHULL_FLAG; // mark invisible
          // mark neighborhood
          uint32_t x, y, z;
          morton_decode(m, x, y, z);
          VoxelData* neighbor;

          if ((neighbor = model.get(x+1, y, z))) {
            neighbor->flag |= VoxelData::SURFACE_FLAG;
          }
          if ((neighbor = model.get(x-1, y, z))) {
            neighbor->flag |= VoxelData::SURFACE_FLAG;
          }
          if ((neighbor = model.get(x, y+1, z))) {
            neighbor->flag |= VoxelData::SURFACE_FLAG;
          }
          if ((neighbor = model.get(x, y-1, z))) {
            neighbor->flag |= VoxelData::SURFACE_FLAG;
          }
          if ((neighbor = model.get(x, y, z+1))) {
            neighbor->flag |= VoxelData::SURFACE_FLAG;
          }
          if ((neighbor = model.get(x, y, z-1))) {
            neighbor->flag |= VoxelData::SURFACE_FLAG;
          }
        }
      } else {
        voxel.flag &= ~VoxelData::VISUALHULL_FLAG;
      }
    }
  }

  // Finalize
  for (uint64_t m = 0; m < model.size(); ++m) {
    if ((model[m].flag & VoxelData::VISUALHULL_FLAG) == 0) {
      model[m].flag = 0;
    }
  }
}

static void plane_sweep(recon::VoxelModel& model, const QList<recon::Camera>& cameras)
{
  using recon::Camera;
  using recon::VoxelData;
  using recon::vec3;

  // from up to down
  // NOTE: consider only one direction sweep -y->+y, -x->+x

  // select cameras look at positive X direction
  QList<Camera> pxcams;
  pxcams.reserve(cameras.size());
  for (Camera cam : cameras) {
    vec3 dir = cam.direction();
    if ((float)dir.x() > 0.1f) {
      pxcams.append(cam);
    }
  }

  for (uint32_t y = 0; y < 128; ++y) {
    uint8_t flag[128] = {0};
    for (uint32_t x = 0; x < 128; ++x) {
      for (uint32_t z = 0; z < 128; ++z) {
        if (flag[z])
          continue;
        VoxelData* voxel = model.get(x, y, z);
        if (voxel->flag & VoxelData::SURFACE_FLAG) {
          QList<uint32_t> pixels;
          QList<uint32_t> pixbounds;

          for (Camera cam: pxcams) {
            // project 8 corners of voxel onto the image, compute the bounding rectangle
            // https://gist.github.com/davll/86633cf34567e6852820#file-voxelcolor-cpp-L168
          }

          // TODO: check photo consistency


        }
      }
    }
  }
  // TODO
  /*
    for plane from -y to y
      create queue from all surface voxel on the plane
      while queue not empty
        voxel = dequeue(queue)
        if voxel is not photo-consistent
          voxel.flag = 0
          mark neighbors as surface voxels
          enqueue(queue, newly marked surface voxels)
        endif
      endwhile
    endfor

    function check-photo-consistency(voxel, cameras)

    endfunction
  */
}

static void save_model(const QString& path, const recon::VoxelModel& model)
{
  using recon::AABox;
  using recon::VoxelData;

  uint64_t count = 0;
  for (uint64_t m = 0; m < model.size(); ++m) {
    if ((model[m].flag & VoxelData::SURFACE_FLAG) != 0)
      count++;
  }

  trimesh::TriMesh mesh;
  mesh.vertices.reserve(8 * count);
  mesh.faces.reserve(6 * 2 * count);

  uint64_t vid = 0;
  for (uint64_t m = 0; m < model.size(); ++m) {
    VoxelData v = model[m];
    if ((v.flag & VoxelData::SURFACE_FLAG) == 0)
      continue;

    AABox vbox = model.boundingbox(m);
    float x0, y0, z0, x1, y1, z1;

    x0 = (float)vbox.minpos.x();
    y0 = (float)vbox.minpos.y();
    z0 = (float)vbox.minpos.z();
    x1 = (float)vbox.maxpos.x();
    y1 = (float)vbox.maxpos.y();
    z1 = (float)vbox.maxpos.z();

    trimesh::point pt[] = {
      { x0, y0, z0 },
      { x1, y0, z0 },
      { x0, y1, z0 },
      { x1, y1, z0 },
      { x0, y0, z1 },
      { x1, y0, z1 },
      { x0, y1, z1 },
      { x1, y1, z1 }
    };
    trimesh::TriMesh::Face face[] = {
      { vid+0, vid+2, vid+1 },
      { vid+1, vid+2, vid+3 },
      { vid+0, vid+6, vid+2 },
      { vid+0, vid+4, vid+6 },
      { vid+0, vid+5, vid+4 },
      { vid+0, vid+1, vid+5 },
      { vid+1, vid+3, vid+5 },
      { vid+3, vid+7, vid+5 },
      { vid+3, vid+2, vid+6 },
      { vid+3, vid+6, vid+7 },
      { vid+4, vid+5, vid+7 },
      { vid+4, vid+7, vid+6 }
    };
    vid += 8;

    for (int i = 0; i < 8; ++i) {
      mesh.vertices.push_back(pt[i]);
    }
    for (int i = 0; i < 12; ++i)
      mesh.faces.push_back(face[i]);
  }

  mesh.need_tstrips();
  mesh.write(path.toUtf8().constData());
}
