#include "CameraLoader.h"
#include "VoxelModel.h"
#include "morton_code.h"
#include <trimesh2/TriMesh.h>

#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDir>
#include <QString>
#include <QtDebug>

#include <QImage>

#include <stdlib.h>
#include <iostream>

static void visual_hull(recon::VoxelModel& model, const QList<recon::Camera>& cameras);
static void plane_sweep(recon::VoxelModel& model, const QList<recon::Camera>& cameras);
static void save_model(const QString& path, const recon::VoxelModel& model);
static bool check_photo_consistency(const QList<uint32_t>& pixels, const QList<uint32_t>& pixbounds, uint32_t& vcolor);

int main(int argc, char* argv[])
{
  QCoreApplication app(argc, argv);
  QCoreApplication::setApplicationName("voxel");
  QCoreApplication::setApplicationVersion("1.0");

  QCommandLineParser parser;
  parser.setApplicationDescription("Model Generator");
  parser.addHelpOption();
  parser.addVersionOption();
  parser.addPositionalArgument("bundle", "Input bundle file");

  parser.process(app);

  const QStringList args = parser.positionalArguments();
  if (args.count() < 1) {
    std::cout << "Bundle path?\n";
    return 0;
  }

  const QString bundlePath = args.at(0);

  recon::CameraLoader loader;
  if (!loader.load_from_nvm(bundlePath)) {
    qDebug() << "Cannot load cameras from " << bundlePath;
    return 1;
  }

  QList<recon::Camera> cameras = loader.cameras();

  QStringList mask_paths;
  mask_paths.reserve(cameras.size());
  for (recon::Camera cam : cameras) {
    QString path =  cam.imagePath();
    QString rootname = path.section(QDir::separator(), 0, -3, QString::SectionIncludeLeadingSep);
    QString filename = path.section(QDir::separator(), -1);
    QString mpath = rootname + QString(QDir::separator()) + "masks" + QString(QDir::separator()) + filename;
    cam.setMaskPath(mpath);
  }

  //for (recon::Camera cam : cameras) {
  //  qDebug() << "mask path = " << cam.maskPath();
  //}

  recon::VoxelModel model(7, loader.model_boundingbox());
  visual_hull(model, cameras);
  plane_sweep(model, cameras);
  save_model("model.ply", model);

  return 0;
}

static void visual_hull(recon::VoxelModel& model, const QList<recon::Camera>& cameras)
{
  using recon::AABox;
  using recon::VoxelData;
  using recon::Camera;
  using recon::mat4;
  using recon::vec3;
  using recon::vec4;
  using recon::morton_decode;

  qDebug() << "visual_hull: initialize";

  // Initialize
  for (uint64_t m = 0; m < model.size(); ++m) {
    model[m].flag = VoxelData::VISIBLE_FLAG;
  }

  // Visual Hull
  for (Camera cam : cameras) {
    qDebug() << "visual_hull: camera iteration";
    QImage mask = QImage(cam.maskPath());

    mat4 extrinsic = cam.extrinsic();
    mat4 intrinsic = cam.intrinsicForImage(mask.width(), mask.height());
    mat4 transform = intrinsic * extrinsic;

    for (uint64_t m = 0; m < model.size(); ++m) {
      VoxelData& voxel = model[m];
      if ((voxel.flag & VoxelData::VISIBLE_FLAG) == 0) {
        continue;
      }

      AABox vbox = model.boundingbox(m);
      vec3 pt = vbox.center();
      pt = proj_vec3(transform * vec4(pt, 1.0f));

      QPoint pt2d = QPoint((float)pt.x(), (float)pt.y());
      if (mask.valid(pt2d)) {
        if (qGray(mask.pixel(pt2d)) < 10) {
          voxel.flag = 0; // mark invisible
          // mark neighborhood
          uint32_t x, y, z;
          morton_decode(m, x, y, z);
          VoxelData* neighbor;

          if ((neighbor = model.get(x+1, y, z))) {
            neighbor->flag |= VoxelData::SURFACE_FLAG;
          }
          if ((neighbor = model.get(x-1, y, z))) {
            neighbor->flag |= VoxelData::SURFACE_FLAG;
          }
          if ((neighbor = model.get(x, y+1, z))) {
            neighbor->flag |= VoxelData::SURFACE_FLAG;
          }
          if ((neighbor = model.get(x, y-1, z))) {
            neighbor->flag |= VoxelData::SURFACE_FLAG;
          }
          if ((neighbor = model.get(x, y, z+1))) {
            neighbor->flag |= VoxelData::SURFACE_FLAG;
          }
          if ((neighbor = model.get(x, y, z-1))) {
            neighbor->flag |= VoxelData::SURFACE_FLAG;
          }
        }
      } else {
        // outside image
        voxel.flag = 0;
      }
    }
  }

  qDebug() << "visual_hull: finalize";

  // Finalize
  for (uint64_t m = 0; m < model.size(); ++m) {
    if ((model[m].flag & VoxelData::VISIBLE_FLAG) == 0) {
      model[m].flag = 0;
    }
  }
}

static void plane_sweep(recon::VoxelModel& model, const QList<recon::Camera>& cameras)
{
  using recon::Camera;
  using recon::VoxelData;
  using recon::vec3;
  using recon::mat4;
  using recon::vec4;
  using recon::AABox;
  using recon::Ray;

  int cam_n = cameras.size();

  QList<QImage> images, masks;
  images.reserve(cam_n);
  masks.reserve(cam_n);
  for (Camera cam : cameras) {
    images.append(QImage(cam.imagePath()));
    masks.append(QImage(cam.maskPath()));
  }

  for (int iterations = 5; iterations; --iterations) {
    for (uint64_t morton = 0, morton_count = model.size(); morton < morton_count; ++morton) {
      VoxelData& voxel = model[morton];
      if ((voxel.flag & VoxelData::VISIBLE_FLAG) == 0)
        continue;
      if ((voxel.flag & VoxelData::SURFACE_FLAG) == 0)
        continue;
      if ((voxel.flag & VoxelData::PHOTO_CONSISTENT_FLAG))
        continue;

      QList<uint32_t> pixels, pixbounds;

      for (int cam_id = 0; cam_id < cam_n; ++cam_id) {
        Camera cam = cameras[cam_id];
        QImage image = images[cam_id];
        QImage mask = masks[cam_id];

        if (!model.check_visibility(cam.center(), morton)) {
          continue;
        }

        // project 8 corners of voxel onto the image, compute the bounding rectangle
        vec3 minpt, maxpt;
        {
          int width = image.width(), height = image.height();
          mat4 proj = cam.intrinsicForImage(width, height) * cam.extrinsic();
          AABox vbox = model.boundingbox(morton);
          vec3 corners[8] = {
            vbox.corner0(),
            vbox.corner1(),
            vbox.corner2(),
            vbox.corner3(),
            vbox.corner4(),
            vbox.corner5(),
            vbox.corner6(),
            vbox.corner7()
          };
          maxpt = minpt = proj_vec3(proj * vec4(corners[0], 1.0f));
          for (int i = 1; i < 8; ++i) {
            vec3 pt = proj_vec3(proj * vec4(corners[i], 1.0f));
            minpt = min(minpt, pt);
            maxpt = max(maxpt, pt);
          }
          //clamp(minpt, vec3::zero(), vec3((float)width, (float)height, 0.0f));
          //clamp(maxpt, vec3::zero(), vec3((float)width, (float)height, 0.0f));
        }

        // feed pixels
        for (int px = (float)minpt.x(); px <= (float)maxpt.x(); ++px) {
          for (int py = (float)minpt.y(); py <= (float)maxpt.y(); ++py) {
            if (image.valid(px, py))
              //if (qGray(mask.pixel(px, py) > 100))
                pixels.append(image.pixel(px, py));
          }
        }

        // update pixbounds
        if (pixbounds.empty()) {
          if (!pixels.empty())
            pixbounds.append(pixels.size());
        } else if (pixels.size() > pixbounds.size()) {
          pixbounds.append(pixels.size());
        }

        // compute color
        if (false) {
          vec3 color = vec3::zero();
          float count = 0.0f;
          for (int px = (float)minpt.x(); px < (float)maxpt.x(); ++px) {
            for (int py = (float)minpt.y(); py < (float)maxpt.y(); ++py) {
              QRgb c = image.pixel(px, py);
              color = color + vec3(qRed(c), qGreen(c), qBlue(c));
              count += 1.0f;
            }
          }
          color = color / count;
          uint32_t ucolor = qRgb((float)color.x(), (float)color.y(), (float)color.z());
          voxel.color = ucolor;
        }
      }
      //
      uint32_t vcolor;
      if (check_photo_consistency(pixels, pixbounds, vcolor)) {
        voxel.color = vcolor;
        voxel.flag |= VoxelData::PHOTO_CONSISTENT_FLAG;
      } else {
        voxel.flag = 0;
        voxel.color = 0;
        uint32_t x, y, z;
        recon::morton_decode(morton, x, y, z);
        VoxelData* neighbor;

        if ((neighbor = model.get(x+1, y, z))) {
          neighbor->flag |= VoxelData::SURFACE_FLAG;
        }
        if ((neighbor = model.get(x-1, y, z))) {
          neighbor->flag |= VoxelData::SURFACE_FLAG;
        }
        if ((neighbor = model.get(x, y+1, z))) {
          neighbor->flag |= VoxelData::SURFACE_FLAG;
        }
        if ((neighbor = model.get(x, y-1, z))) {
          neighbor->flag |= VoxelData::SURFACE_FLAG;
        }
        if ((neighbor = model.get(x, y, z+1))) {
          neighbor->flag |= VoxelData::SURFACE_FLAG;
        }
        if ((neighbor = model.get(x, y, z-1))) {
          neighbor->flag |= VoxelData::SURFACE_FLAG;
        }
      }
    }
  }

  /*
  TODO
  update_flag = true
  while update_flag == true do
    update_flag = false
    for each voxel do
      continue if voxel is not surface voxel
      for each camera do
        continue if voxel is already computed with the camera
        check photo consistency
        if not consistent then
          carve the voxel (its flag becomes zero)
          mark neighboring voxels as surface voxels
          update_flag = true
        endif
      end
    end
  done
  */
  // https://gist.github.com/davll/86633cf34567e6852820#file-voxelcolor-cpp-L168

  // Finalize
  for (uint64_t m = 0; m < model.size(); ++m) {
    if ((model[m].flag & VoxelData::VISIBLE_FLAG) == 0) {
      model[m].flag = 0;
    }
  }
}

static bool check_photo_consistency(const QList<uint32_t>& pixels, const QList<uint32_t>& pixbounds, uint32_t& color)
{
  const double adaptiveThreshold1 = 10.0;
  const double adaptiveThreshold2 = 6.0;

  if(pixels.size() == 0) {
    //printf("WTF\n");
    return false;
  }

  int64_t colRsquared = 0, colR = 0;
  int64_t colGsquared = 0, colG = 0;
  int64_t colBsquared = 0, colB = 0;
  int64_t current_colRsquared = 0, current_colR = 0;
  int64_t current_colGsquared = 0, current_colG = 0;
  int64_t current_colBsquared = 0, current_colB = 0;

  double averageStdDev = 0.0;         // accumulator for the average standard
                                      // deviation over all images
  int K = 0, current_K = 0;
  unsigned int i = 0;
  auto imageTracker = pixbounds.begin();
  for(auto iter = pixels.begin(); iter != pixels.end(); iter++, i++) {
    //dbg << i << " " << *imageTracker << "\n";
    if(i >= *imageTracker) {     // advance to the next image
      colRsquared += current_colRsquared;
      colGsquared += current_colGsquared;
      colBsquared += current_colBsquared;
      colR += current_colR;
      colG += current_colG;
      colB += current_colB;
      K += current_K;

      // calculate the standard deviation for this single image
      if(current_K != 0) {
        double variance =
          (static_cast<double>(current_colRsquared + current_colGsquared + current_colBsquared) / current_K) -
          (static_cast<double>(current_colR * current_colR + current_colG * current_colG + current_colB * current_colB) / (current_K * current_K));

        averageStdDev += sqrt(variance);
      }

      current_colRsquared = 0;
      current_colGsquared = 0;
      current_colBsquared = 0;
      current_colR = 0;
      current_colG = 0;
      current_colB = 0;
      current_K = 0;
      imageTracker++;
      while(i >= *imageTracker) {
        if(imageTracker == pixbounds.end())
          throw "That is impossible: ran out of images";
        imageTracker++;
      }
    }

    uint32_t pixel = *iter;

    current_colRsquared += qRed(pixel) * qRed(pixel);
    current_colGsquared += qGreen(pixel) * qGreen(pixel);
    current_colBsquared += qBlue(pixel) * qBlue(pixel);
    current_colR += qRed(pixel);
    current_colG += qGreen(pixel);
    current_colB += qBlue(pixel);
    current_K++;
  }

  { // add stats from last image
    colRsquared += current_colRsquared;
    colGsquared += current_colGsquared;
    colBsquared += current_colBsquared;
    colR += current_colR;
    colG += current_colG;
    colB += current_colB;
    K += current_K;

    // calculate the standard deviation for this single image
    if(current_K != 0) {
      double variance =
        (static_cast<double>(current_colRsquared + current_colGsquared + current_colBsquared) / current_K) -
        (static_cast<double>(current_colR * current_colR + current_colG * current_colG + current_colB * current_colB) / (current_K * current_K));

      averageStdDev += sqrt(variance);
    }
  }

  // divide by number of images to get the true average
  averageStdDev = averageStdDev / pixbounds.size();

  color = qRgb(colR / K, colG / K, colB / K);

  // note: this is only safe because we are now using 64-bit integers; that way
  // there won't be any overflows. With 32-bit integers, the following statement
  // can overflow with ~11000 white pixels (~32000 / 3)
  double var = (static_cast<double>(colRsquared + colGsquared + colBsquared) / K) -
               (static_cast<double>(colR * colR + colG * colG + colB * colB) / (K * K));
  double stddev = sqrt(var);

  //printf("%lf %lf\n", stddev, adaptiveThreshold1 + averageStdDev * adaptiveThreshold2);

  if(stddev < adaptiveThreshold1 + averageStdDev * adaptiveThreshold2) {
    return true;
  }
  return false;
}

static void save_model(const QString& path, const recon::VoxelModel& model)
{
  using recon::AABox;
  using recon::VoxelData;

  uint64_t count = 0;
  for (uint64_t m = 0; m < model.size(); ++m) {
    uint32_t f = model[m].flag;
    if ((f & VoxelData::VISIBLE_FLAG) && (f & VoxelData::SURFACE_FLAG))
      count++;
  }

  trimesh::TriMesh mesh;
  mesh.vertices.reserve(8 * count);
  mesh.colors.reserve(8 * count);
  mesh.faces.reserve(6 * 2 * count);

  uint64_t vid = 0;
  for (uint64_t m = 0; m < model.size(); ++m) {
    VoxelData v = model[m];
    uint32_t flag = model[m].flag;
    if (!(flag & VoxelData::VISIBLE_FLAG) || !(flag & VoxelData::SURFACE_FLAG))
      continue;

    AABox vbox = model.boundingbox(m);
    float x0, y0, z0, x1, y1, z1;

    x0 = (float)vbox.minpos.x();
    y0 = (float)vbox.minpos.y();
    z0 = (float)vbox.minpos.z();
    x1 = (float)vbox.maxpos.x();
    y1 = (float)vbox.maxpos.y();
    z1 = (float)vbox.maxpos.z();

    trimesh::point pt[] = {
      { x0, y0, z0 },
      { x1, y0, z0 },
      { x0, y1, z0 },
      { x1, y1, z0 },
      { x0, y0, z1 },
      { x1, y0, z1 },
      { x0, y1, z1 },
      { x1, y1, z1 }
    };
    trimesh::TriMesh::Face face[] = {
      { vid+0, vid+2, vid+1 },
      { vid+1, vid+2, vid+3 },
      { vid+0, vid+6, vid+2 },
      { vid+0, vid+4, vid+6 },
      { vid+0, vid+5, vid+4 },
      { vid+0, vid+1, vid+5 },
      { vid+1, vid+3, vid+5 },
      { vid+3, vid+7, vid+5 },
      { vid+3, vid+2, vid+6 },
      { vid+3, vid+6, vid+7 },
      { vid+4, vid+5, vid+7 },
      { vid+4, vid+7, vid+6 }
    };
    vid += 8;

    trimesh::Color vcolor(qRed(v.color), qGreen(v.color), qBlue(v.color));

    for (int i = 0; i < 8; ++i) {
      mesh.vertices.push_back(pt[i]);
      mesh.colors.push_back(vcolor);
    }
    for (int i = 0; i < 12; ++i)
      mesh.faces.push_back(face[i]);
  }

  mesh.need_tstrips();
  mesh.write(path.toUtf8().constData());
}
