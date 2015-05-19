#pragma once

#ifndef _VOXEL_CAMERALOADER_H_
#define _VOXEL_CAMERALOADER_H_

#include "CameraData.h"

#include <QList>
#include <QString>

namespace voxel {

typedef QList<CameraData> CameraList;

bool load_from_nvm(QStringList& images,
                   CameraList& cameras,
                   const QString& path);

}

#endif /* end of include guard: _VOXEL_CAMERALOADER_H_ */
