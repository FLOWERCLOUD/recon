#pragma once

#include <QString>

namespace recon {

struct VoxelBlock;

struct OctreeNode {
  uint64_t morton;
  uint32_t color;
  uint16_t level;
  uint8_t child_mask;
  uint8_t reserved0;
};

class OctreeBuilder {
public:
  OctreeBuilder(const QString& octreePath);
  ~OctreeBuilder();

  void addBlock(const VoxelBlock* block);

private:


private:
  QString m_Path;
  OctreeNode* m_Buffer;
};

}

// http://www.forceflow.be/2012/07/24/out-of-core-construction-of-sparse-voxel-octrees/
// http://www.forceflow.be/2012/06/14/introducing-binvox2moctree/
// https://github.com/Forceflow/ooc_svo_builder
