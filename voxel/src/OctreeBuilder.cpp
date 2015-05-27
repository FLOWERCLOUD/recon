#include "OctreeBuilder.h"

namespace recon {

OctreeBuilder::OctreeBuilder(const QString& octreePath)
: m_Path(octreePath)
{
}

OctreeBuilder::~OctreeBuilder()
{
}

void OctreeBuilder::addBlock(const VoxelBlock* block)
{
}

}
