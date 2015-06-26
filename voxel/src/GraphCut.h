#pragma once

/* MAJOR

  Multi-view Stereo via Volumetric Graph-cuts and Occlusion Robust Photo-Consistency
  George Vogiatzis, Carlos H. Esteban, Philip H. S. Torr, Roberto Cipolla
  PAMI 2007

  Multi-view Stereo via Volumetric Graph-cuts
  George Vogiatzis, P.H.S. Torr, R. Cipolla
  CVPR 2005
*/

/* MINOR

  Shape from Photographs: A Multi-view Stereo Pipeline
  C. Hernández and G. Vogiatzis
  Computer Vision: Detection, Recognition and Reconstruction
  2010 Springer-Verlag

  Automatic 3D object segmentation in multiple views using volumetric graph-cuts
  N.D.F. Campbell, G. Vogiatzis, C. Hernandez and R. Cipolla
  Image and Vision Computing 2010

  Automatic Object Segmentation from Calibrated Images
  N.D.F. Cambell, G. Vogiatzis, C. Hernández and R. Cipolla
  CVMP 2011
*/

#include "Camera.h"
#include "VoxelModel.h"
#include <QList>
#include <QImage>

namespace recon {

VoxelList graph_cut(const VoxelModel& model, const QList<Camera>& cameras);
QImage photo_consistency_test(const VoxelModel& model, const QList<Camera>& cameras, int plane_y);

}
