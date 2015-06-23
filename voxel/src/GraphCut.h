#pragma once

/*
  Multi-view Stereo via Volumetric Graph-cuts and Occlusion Robust Photo-Consistency
  George Vogiatzis, Carlos H. Esteban, Philip H. S. Torr, Roberto Cipolla
  PAMI 2007

  Shape from Photographs: A Multi-view Stereo Pipeline
  C. Hernández and G. Vogiatzis
  Computer Vision: Detection, Recognition and Reconstruction
  2010 Springer-Verlag

  Multi-view Stereo via Volumetric Graph-cuts
  George Vogiatzis, P.H.S. Torr, R. Cipolla
  CVPR 2005
*/

// N.D.F. Cambell, G. Vogiatzis, C. Hernández and R. Cipolla, CVMP 2011. Automatic Object Segmentation from Calibrated Images

#include "Camera.h"
#include "VoxelModel.h"
#include <QList>

namespace recon {

VoxelList graph_cut(const VoxelModel& model, const QList<Camera>& cameras);

}
