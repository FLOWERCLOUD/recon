#include "VisualHull.h"
#include <QImage>

namespace recon {

VoxelList visual_hull(const VoxelModel& model, const QList<Camera>& cameras)
{
  QList<uint64_t> voxels[2];
  voxels[0].reserve(model.morton_length);

  for (uint64_t m = 0, ms = model.morton_length; m < ms; ++m) {
    voxels[0].append(m);
  }

  int current_voxel_list = 0;
  for (int cam_i = 0, cam_n = cameras.size(); cam_i < cam_n; ++cam_i) {
    Camera cam = cameras[cam_i];
    QImage mask = QImage(cam.maskPath());

    Mat4 extrinsic = cam.extrinsic();
    Mat4 intrinsic = cam.intrinsicForImage(mask.width(), mask.height());
    Mat4 transform = intrinsic * extrinsic;

    QList<uint64_t>& old_voxels = voxels[current_voxel_list];
    QList<uint64_t>& new_voxels = voxels[(current_voxel_list + 1) % 2];
    new_voxels.clear();
    new_voxels.reserve(old_voxels.size());

    for (uint64_t morton : old_voxels) {
      AABox vbox = model.element_box(morton);
      Vec3 pos = (Vec3)vbox.center();
      pos = Vec3::proj(transform * Vec4(pos, 1.0f));

      QPoint pt2d = QPoint((float)pos.x(), (float)pos.y());
      if (mask.valid(pt2d)) {
        if (qGray(mask.pixel(pt2d)) > 100) {
          new_voxels.append(morton);
        }
      }
    }

    current_voxel_list = (current_voxel_list + 1) % 2;
  }

  return voxels[current_voxel_list];
}

}
