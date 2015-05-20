#include "VoxelColoring.h"
#include "CameraLoader.h"
#include <QtDebug>

namespace voxel {

VoxelColoring::VoxelColoring(QString bundle_path)
{
  CameraLoader loader;

  if (loader.load_from_nvm(bundle_path)) {
    m_ImagePaths = loader.image_paths();
    m_Cameras = loader.cameras();
    m_ModelAABB = loader.feature_boundingbox();
  } else {
    qWarning() << "Cannot load bundle file " << bundle_path;
  }

  const int n = m_Cameras.size();
  for (int i = 0; i < n; ++i) {
    qDebug() << "cam[" << i << "] {\n"
             << "  image = " << m_ImagePaths[i] << "\n"
             << "  aspect = " << m_Cameras[i].aspect_ratio << "\n"
             << "}";
  }
  qDebug() << "feature AABB = ("
           << m_ModelAABB.minpos[0] << ", " << m_ModelAABB.minpos[1] << ", " << m_ModelAABB.minpos[2]
           << ") to ("
           << m_ModelAABB.maxpos[0] << ", " << m_ModelAABB.maxpos[1] << ", " << m_ModelAABB.maxpos[2]
           << ")";

  m_Images.reserve(n);
  for (int i = 0; i < n; ++i) {
    m_Images.append(QImage(m_ImagePaths[i]));
  }
}

VoxelColoring::~VoxelColoring()
{
}

bool VoxelColoring::process()
{
  int ncams = m_Cameras.size();
  if (ncams == 0)
    return false;

  m_Voxels.allocate(1024, 1024, 1024);
  m_Voxels.set_world(m_ModelAABB);

  int slicewidth = m_Voxels.width(),
      sliceheight = m_Voxels.height(),
      slicedepth = m_Voxels.depth();
  int startx = 0, starty = 0, startz = 0;

  while (slicewidth > 0 && sliceheight > 0 && slicedepth > 0) {
    int i, j, k, x, y, z;

    // XY plane
    for (k = 0, i = 0; i < slicewidth; ++i) {
      for (j = 0; j < sliceheight; ++j) {
        x = i + startx, y = j + starty, z = k + startz;
        paint_voxel(x, y, z);
      }
    }
    for (k = slicedepth-1, i = 0; i < slicewidth; ++i) {
      for (j = 0; j < sliceheight; ++j) {
        x = i + startx, y = j + starty, z = k + startz;
        paint_voxel(x, y, z);
      }
    }

    // YZ plane
    for (i = 0, j = 0; j < sliceheight; ++j) {
      for (k = 0; k < slicedepth; ++k) {
        x = i + startx, y = j + starty, z = k + startz;
        paint_voxel(x, y, z);
      }
    }
    for (i = slicewidth-1, j = 0; j < sliceheight; ++j) {
      for (k = 0; k < slicedepth; ++k) {
        x = i + startx, y = j + starty, z = k + startz;
        paint_voxel(x, y, z);
      }
    }

    // XZ plane
    for (j = 0, i = 0; i < slicewidth; ++i) {
      for (k = 0; k < slicedepth; ++k) {
        x = i + startx, y = j + starty, z = k + startz;
        paint_voxel(x, y, z);
      }
    }
    for (j = sliceheight-1, i = 0; i < slicewidth; ++i) {
      for (k = 0; k < slicedepth; ++k) {
        x = i + startx, y = j + starty, z = k + startz;
        paint_voxel(x, y, z);
      }
    }

    slicewidth -= 2;
    sliceheight -= 2;
    slicedepth -= 2;
    ++startx;
    ++starty;
    ++startz;
  }

  return true;
}

void VoxelColoring::paint_voxel(int x, int y, int z)
{
  const int n = m_Cameras.size();

  QRgb colors[n];
  for (int i = 0; i < n; ++i) {
    QImage img = m_Images[i];

    float imgpos[3];
    {
      vec4 pos = to_vec4(m_Voxels.map_to_world(x, y, z), 1.0f);
      pos = m_Cameras[i].get_extrinsic() * pos;
      vec3 xyz = to_vec3(pos);
      xyz = m_Cameras[i].get_intrinsic() * xyz;
      store_vec3(imgpos, xyz);
    }
    imgpos[0] /= imgpos[2];
    imgpos[1] /= imgpos[2];
    imgpos[0] = (imgpos[0] + 1.0f) * 0.5f;
    imgpos[1] = (-imgpos[1] + 1.0f) * 0.5f;

    int px = imgpos[0] * img.width();
    int py = imgpos[1] * img.height();

    QRgb c = (img.valid(px, py) ? img.pixel(px, py) : 0);
    colors[i] = c;
  }

  // compute correlation
  


  // paint voxel if correlation is sufficient
}

}
