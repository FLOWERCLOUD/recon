#pragma once

#include "Camera.h"
#include "VoxelModel.h"

namespace recon {

VoxelList visual_hull(const VoxelModel& model, const QList<Camera>& cameras);

}
