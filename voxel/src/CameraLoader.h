#pragma once

#ifndef _VOXEL_CAMERALOADER_H_
#define _VOXEL_CAMERALOADER_H_

#include "CameraData.h"

#include <string>
#include <fstream>
#include <vector>

namespace voxel {

bool load_from_nvm(std::vector<std::string>& images,
                   std::vector<CameraData>& cameras,
                   const std::string& path);

}

#endif /* end of include guard: _VOXEL_CAMERALOADER_H_ */
