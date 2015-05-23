#pragma once

#include <QString>

namespace recon {

struct VoxelBlock;

class OctreeBuilder {
public:
  OctreeBuilder();
  ~OctreeBuilder();

  void addBlock(const VoxelBlock* block);

private:
};

}

// http://www.forceflow.be/2012/07/24/out-of-core-construction-of-sparse-voxel-octrees/
// http://www.forceflow.be/2012/06/14/introducing-binvox2moctree/
// https://github.com/Forceflow/ooc_svo_builder
