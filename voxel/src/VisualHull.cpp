#include "VisualHull.h"
#include "VoxelBlock.h"
#include <QImage>

namespace recon {

void visualhull(VisualHullParams& params)
{
  int ncams = params.cameras.size();
  VoxelBlock* block = params.block;

  for (int i = 0; i < ncams; ++i) {
    const CameraData& cam = params.cameras[i];
    QImage mask = QImage(params.mask_paths[i]);

    block->each_voxel([&cam, &mask](uint64_t morton, recon::VoxelBlock* block) {
      AABB vbox = block->voxelbox(morton);
      VoxelData& vdata = block->voxeldata(morton);

      vec3 pt = vbox.get_center();
      vec3 pixpos = cam.world_to_image(pt, mask.width(), mask.height());
      int px = (float)pixpos.x(), py = (float)pixpos.y();

      if (mask.valid(px, py)) {
        if (qGray(mask.pixel(px, py)) > 10) {
          vdata.color = qRgb(255, 255, 255);
        }
      }
    });
  }
}

}
