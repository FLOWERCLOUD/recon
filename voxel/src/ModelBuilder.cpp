#include "ModelBuilder.h"
#include "VoxelBlock.h"
#include <stdio.h>
#include <QImage>

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

    // debug: check result
    /*uint64_t count = 0;
    block->each_voxel([&count](uint64_t morton, VoxelData& voxel) {
      if (voxel.flag) {
        count++;
      }
    });
    printf("count = %lld\n", count);*/

    // Voxel Coloring

    // Add to octree

    blockman.release(block);
    block = blockman.generate();
  }

  return true;
}

}
