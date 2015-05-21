#include "VoxelColoring.h"
#include "CameraLoader.h"
#include <QtDebug>
#include <QColor>
#include <QImage>
#include <QDir>
//#include <rply.h>

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

  m_Voxels.allocate(64, 64, 64);
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
  //const int n = m_Cameras.size();

  Voxel& target = m_Voxels.at(x, y, z);
  vec3 worldpos = m_Voxels.map_to_world(x, y, z);

  //vec3 imagepos = cam.world_to_image(worldpos, width, height);
  //vec3 


  // map to camera 0
  if (false) {
    const QImage& img = m_Images[0];
    const CameraData& cam = m_Cameras[0];

    vec3 pos = m_Voxels.map_to_world(x, y, z);
    pos = cam.world_to_image(pos, img.width(), img.height());

    int px = (float)get_x(pos);
    int py = (float)get_y(pos);

    if (img.valid(px, py)) {
      QRgb color = img.pixel(px, py);
      m_Voxels.at(x, y, z) = color;
    }
  }

#if 0
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
    imgpos[1] = (imgpos[1] + 1.0f) * 0.5f;

    int px = imgpos[0] * img.width();
    int py = imgpos[1] * img.height();

    QRgb c = (img.valid(px, py) ? img.pixel(px, py) : 0);
    colors[i] = c;
  }

  // compute correlation
  int tag[n], cnt[n];
  for (int i = 0; i < n; ++i) {
    tag[i] = i, cnt[i] = 0;
  }

  for (int i = 0; i < n-1; ++i) {
    QColor ci = QColor::fromRgba(colors[i]);
    if (ci.alpha() == 0) continue;
    for (int j = i+1; j < n; ++j) {
      QColor cj = QColor::fromRgba(colors[j]);
      if (cj.alpha() == 0)
        continue;
      int dr = ci.red() - cj.red();
      int dg = ci.green() - cj.green();
      int db = ci.blue() - cj.blue();
      int eu = dr * dr + dg * dg + db * db;
      if (eu < 100) {
        if (tag[j] > tag[i]) {
          tag[j] = tag[i];
        } else {
          tag[i] = tag[j];
        }
      }
    }
  }

  for (int i = 0; i < n; ++i) {
    cnt[tag[i]]++;
  }

  int maxcnt = 0, maxcnt_index = -1;
  for (int i = 0; i < n; ++i) {
    if (maxcnt < cnt[i]) {
      maxcnt = cnt[i];
      maxcnt_index = i;
    }
  }

  if (maxcnt > 3) {
    passed = true;
  }

  // paint voxel if correlation is sufficient
  if (passed) {
    /*int r = 0, g = 0, b = 0;
    for (int i = 0; i < n; ++i) {
      if (maxcnt_index == tag[i]) {
        QColor c(colors[i]);
        r += c.red();
        g += c.green();
        b += c.blue();
      }
    }
    r /= maxcnt;
    g /= maxcnt;
    b /= maxcnt;
    QColor vc(r, g, b);*/
    QColor vc = QColor(colors[maxcnt_index]);
    //QColor vc = QColor(255, 0, 0);
    m_Voxels.at(x, y, z) = vc.rgba();
    //qDebug() << "Voxel (" << x << ", " << y << ", " << z << ")"
    //         << " = " << vc;
  }
#endif
}

void VoxelColoring::save_to_png_set(const QString& basename)
{
  QDir dir(basename);
  dir.mkpath(".");

  int w, h, d;
  w = m_Voxels.width();
  h = m_Voxels.height();
  d = m_Voxels.depth();

  for (int k = 0; k < d; ++k) {
    QImage image(w, h, QImage::Format_ARGB32);

    for (int i = 0; i < w; ++i) {
      for (int j = 0; j < h; ++j) {
        QRgb c = m_Voxels.at(i, j, k);
        image.setPixel(i, j, c);
      }
    }

    image.save(dir.absoluteFilePath(QString("%1.png").arg(k)));
  }
}

/*
void VoxelColoring::save_to_ply(const QString& path)
{
  long ncubes = 0;
  int w, h, d;
  w = m_Voxels.width();
  h = m_Voxels.height();
  d = m_Voxels.depth();

  for (int i = 0; i < w; ++i) {
    for (int j = 0; j < h; ++j) {
      for (int k = 0; k < d; ++k) {
        if (QColor(m_Voxels.at(i,j,k)).alpha() > 0)
          ncubes++;
      }
    }
  }

  p_ply oply = ply_create(path.toUtf8().constData(), PLY_LITTLE_ENDIAN, NULL, 0, NULL);
  Q_CHECK_PTR(oply);

  ply_add_element(oply, "vertex", 8 * ncubes);
  ply_add_scalar_property(oply, "x", PLY_FLOAT);
  ply_add_scalar_property(oply, "y", PLY_FLOAT);
  ply_add_scalar_property(oply, "z", PLY_FLOAT);

  ply_add_element(oply, "face", 6 * 2 * 3 * ncubes);
  ply_add_list_property(oply, "vertex_indices", PLY_UINT, PLY_UINT);

  ply_write_header(oply);

  for (int i = 0; i < w; ++i) {
    for (int j = 0; j < h; ++j) {
      for (int k = 0; k < d; ++k) {
        float pos[3], size[3];
        store_vec3(pos, m_Voxels.map_to_world(i, j, k));
        store_vec3(size, m_Voxels.world_size());
        size[0] /= m_Voxels.width() * 2;
        size[1] /= m_Voxels.height() * 2;
        size[2] /= m_Voxels.depth() * 2;

        ply_write(oply, pos[0]-size[0]);
        ply_write(oply, pos[1]+size[1]);
        ply_write(oply, pos[2]-size[2]);
        ply_write(oply, pos[0]-size[0]);
        ply_write(oply, pos[1]+size[1]);
        ply_write(oply, pos[2]+size[2]);
        ply_write(oply, pos[0]+size[0]);
        ply_write(oply, pos[1]+size[1]);
        ply_write(oply, pos[2]+size[2]);
        ply_write(oply, pos[0]+size[0]);
        ply_write(oply, pos[1]+size[1]);
        ply_write(oply, pos[2]-size[2]);

        ply_write(oply, pos[0]-size[0]);
        ply_write(oply, pos[1]-size[1]);
        ply_write(oply, pos[2]-size[2]);
        ply_write(oply, pos[0]-size[0]);
        ply_write(oply, pos[1]-size[1]);
        ply_write(oply, pos[2]+size[2]);
        ply_write(oply, pos[0]+size[0]);
        ply_write(oply, pos[1]-size[1]);
        ply_write(oply, pos[2]+size[2]);
        ply_write(oply, pos[0]+size[0]);
        ply_write(oply, pos[1]-size[1]);
        ply_write(oply, pos[2]-size[2]);
      }
    }
  }

  for (int i = 0, off = 0; i < w; ++i) {
    for (int j = 0; j < h; ++j) {
      for (int k = 0; k < d; ++k) {

        ply_write(off+0);
        ply_write(off+1);
        ply_write(off+2);

        ply_write(off+0);
        ply_write(off+2);
        ply_write(off+3);

        //ply_write(off+);

        off += 8;
      }
    }
  }

  ply_close(oply);
}*/

}
