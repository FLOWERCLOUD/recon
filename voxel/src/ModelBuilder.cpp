#include "ModelBuilder.h"
#include "VoxelBlock.h"
#include <stdio.h>
#include <QImage>
#include <trimesh2/TriMesh.h>
#include <trimesh2/TriMesh_algo.h>

namespace recon {

ModelBuilder::ModelBuilder(const AABox& box,
                           const QList<Camera>& cameras,
                           const QString& octreePath,
                           const QString& meshPath)
: m_ModelBox(box)
, m_Cameras(cameras)
, m_OctreePath(octreePath)
, m_MeshPath(meshPath)
{
}

ModelBuilder::~ModelBuilder()
{
}

bool ModelBuilder::execute()
{
  VoxelBlockManager blockman(m_ModelBox, 1, 64);

  for (VoxelBlock* block = blockman.generate(); block;) {
    //printf("block (%u, %u, %u)\n", block->origin[0], block->origin[1], block->origin[2]);

    // Initialize voxels
    block->each_voxel([block](uint64_t morton, VoxelData& voxel) {
      voxel.flag = 0;
      voxel.color = 0;
    });

    // Visual Hull
    bool first = true;
    for (Camera cam : m_Cameras) {
      QImage mask = QImage(cam.maskPath());
      mat4 extrinsic = cam.extrinsic();
      mat4 intrinsic = cam.intrinsicForImage(mask.width(), mask.height());
      mat4 transform = intrinsic * extrinsic;

      if (first) {
        block->each_voxel([block, transform, mask](uint64_t morton, VoxelData& voxel) {
          AABox vbox = block->voxelbox(morton);
          vec3 pt = vbox.get_center();
          pt = proj_vec3(transform * vec4(pt, 1.0f));

          QPoint pt2d = QPoint((float)pt.x(), (float)pt.y());
          if (mask.valid(pt2d)) {
            if (qGray(mask.pixel(pt2d)) > 100) {
              voxel.flag = 1; // mark visible
            }
          }
        });
        first = false;
      } else {
        block->each_voxel([block, transform, mask](uint64_t morton, VoxelData& voxel) {
          AABox vbox = block->voxelbox(morton);
          vec3 pt = vbox.get_center();
          pt = proj_vec3(transform * vec4(pt, 1.0f));

          QPoint pt2d = QPoint((float)pt.x(), (float)pt.y());
          if (mask.valid(pt2d)) {
            if (qGray(mask.pixel(pt2d)) < 10) {
              voxel.flag = 0; // mark invisible
            }
          }
        });
      }
    }

    // Voxel Coloring

    // debug: export voxels to mesh
    {
      uint64_t count = 0;
      block->each_voxel([&count](uint64_t morton, VoxelData& voxel) {
        if (voxel.flag) count++;
      });

      trimesh::TriMesh mesh;
      mesh.vertices.reserve(8 * count);
      mesh.faces.reserve(6 * 2 * count);

      uint64_t vid = 0;
      block->each_voxel([&vid, &mesh, block](uint64_t morton, VoxelData& voxel) {
        if (voxel.flag) {
          AABox vbox = block->voxelbox(morton);
          float x0 = vbox.minpos[0], y0 = vbox.minpos[1], z0 = vbox.minpos[2],
                x1 = vbox.maxpos[0], y1 = vbox.maxpos[1], z1 = vbox.maxpos[2];
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
            { vid+3, vid+5, vid+7 },
            { vid+3, vid+2, vid+6 },
            { vid+3, vid+6, vid+7 },
            { vid+4, vid+5, vid+7 },
            { vid+4, vid+7, vid+6 }
          };
          vid += 8;

          for (int i = 0; i < 8; ++i)
            mesh.vertices.push_back(pt[i]);
          for (int i = 0; i < 12; ++i)
            mesh.faces.push_back(face[i]);
        }
      });

      mesh.need_tstrips();
      mesh.write(m_MeshPath.toUtf8().constData());
    }

    // Add to octree

    blockman.release(block);
    block = blockman.generate();
  }

  return true;
}

}
