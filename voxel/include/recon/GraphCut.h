#pragma once

/* MAJOR

  Computing geodesics and minimal surfaces via graph cuts
  Y. Boykov and V. Kolmogorov
  ICCV 2003

*/

/* GRIDCUT
  http://gridcut.com/

  An experimental comparison of min-cut/max-flow algorithms for energy minimization in vision
  Y. Boykov and V Kolmogorov
  PAMI 2004

  Parallel Graph-cuts by Adaptive Bottom-up Merging
  Jiangyu Liu and Jian Sun:
  ICCV 2010

  Cache-efficient Graph Cuts on Structured Grids
  Ondřej Jamriška, Daniel Sýkora, and Alexander Hornung
  ICCV 2012

*/

#include "BuildGraph.h"

namespace recon {

VoxelList graph_cut(const VoxelGraph& graph);

}
