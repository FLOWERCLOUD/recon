#include "VoxelModel.h"
#include "morton_code.h"
#include <QtGlobal>

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

    point3 start = real_box.minpos;
    vec3 extent = vec3(vsiz, vsiz, vsiz);
    virtual_box = AABox(start, start + extent);
  }
}

}
#include <trimesh2/TriMesh.h>
namespace recon {

void save_ply(const QString& path, const VoxelModel& model, const VoxelList& vlist)
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

}
