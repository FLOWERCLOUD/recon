#include "VoxelModel.h"
#include "morton_code.h"
#include <QtGlobal>
#include <QColor>
#include <QTextStream>
#include <QFile>
#include <QtDebug>

namespace recon {

VoxelModel::VoxelModel(uint16_t lv, AABox model_box)
: level(lv)
, real_box(model_box)
, virtual_box()
, width(0x1 << lv)
, height(0x1 << lv)
, depth(0x1 << lv)
, morton_length(0x1ull << lv*3)
{
  if (level > 20) {
    qFatal("%s:%d: level is too high (level = %d)", __FILE__, __LINE__, level);
  }

  {
    float siz[3], vsiz;
    real_box.extent().store(siz);
    vsiz = fmaxf(siz[0], fmaxf(siz[1], siz[2]));

    Point3 start = real_box.minpos;
    Vec3 extent = Vec3(vsiz, vsiz, vsiz);
    virtual_box = AABox(start, start + extent);
  }
}

bool load_voxels(VoxelModel& model, VoxelList& vlist, const QString& path)
{
  QFile file(path);
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    qDebug() << "Cannot open input file!";
    return false;
  }

  QTextStream stream(&file);

  int level, width;
  float minpos[3], maxpos[3];
  stream >> level >> width
         >> minpos[0] >> minpos[1] >> minpos[2]
         >> maxpos[0] >> maxpos[1] >> maxpos[2];

  model = VoxelModel(
    level, AABox(Point3::load(minpos), Point3::load(maxpos))
  );

  uint64_t nvoxels;
  stream >> nvoxels;
  vlist.clear();
  vlist.reserve(nvoxels);

  for (uint64_t i = 0; i < nvoxels; ++i) {
    uint32_t x, y, z;
    stream >> x >> y >> z;
    vlist.append(morton_encode(x,y,z));
  }

  return true;
}

bool save_voxels(const VoxelModel& model, const VoxelList& vlist, const QString& path)
{
  QFile file(path);
  if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
    qDebug() << "Cannot open output file: " << path;
    return false;
  }

  QTextStream stream(&file);
  stream.setRealNumberNotation(QTextStream::ScientificNotation);
  stream.setRealNumberPrecision(15);

  stream << model.level << "\n";
  stream << model.width << "\n";
  stream << (float)model.virtual_box.minpos.x() << " "
         << (float)model.virtual_box.minpos.y() << " "
         << (float)model.virtual_box.minpos.z() << "\n"
         << (float)model.virtual_box.maxpos.x() << " "
         << (float)model.virtual_box.maxpos.y() << " "
         << (float)model.virtual_box.maxpos.z() << "\n";

  stream << vlist.size() << "\n";
  for (uint64_t m: vlist) {
    uint32_t x, y, z;
    morton_decode(m, x, y, z);
    stream << x << " " << y << " " << z << "\n";
  }

  stream.flush();
  file.close();
  return true;
}

}
#include <trimesh2/TriMesh.h>
namespace recon {

void save_cubes_ply(const QString& path, const VoxelModel& model, const VoxelList& vlist)
{
  uint64_t count = vlist.count();

  trimesh::TriMesh mesh;
  mesh.vertices.reserve(8 * count);
  mesh.faces.reserve(6 * 2 * count);

  uint64_t vid = 0;
  for (uint64_t m : vlist) {
    AABox vbox = model.element_box(m);
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

void save_points_ply(const QString& path, const VoxelModel& model, const VoxelList& vlist)
{
  uint64_t count = vlist.count();

  trimesh::TriMesh mesh;
  mesh.vertices.reserve(count);

  for (uint64_t m : vlist) {
    AABox vbox = model.element_box(m);
    trimesh::point pt = {
      (float)vbox.center().x(),
      (float)vbox.center().y(),
      (float)vbox.center().z()
    };
    mesh.vertices.push_back(pt);
  }
  mesh.write(path.toUtf8().constData());
}

bool save_raw(const VoxelModel& model, const VoxelList& vlist, const QString& path)
{
  size_t size = model.width * model.height * model.depth;
  QByteArray data = QByteArray(size, 0);

  for (uint64_t m : vlist)
    data[(uint)m] = 255;

  QFile file(path);
  if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
    qDebug() << "Cannot open output file: " << path;
    return false;
  }
  file.write(data);
  file.close();
  return true;
}

}
