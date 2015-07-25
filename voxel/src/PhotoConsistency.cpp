#include "VoxelScore1.h"
#include "PhotoConsistency.h"

namespace recon {

PhotoConsistency::
PhotoConsistency(const VoxelModel& model, const QList<Camera>& cams)
: voxel_size((float)model.virtual_box.extent().x() / model.width)
, cameras(cams)
{
  images.reserve(cams.size());
  for (int i = 0; i < cams.size(); ++i) {
    QImage img = QImage(cameras[i].imagePath());
    //if (img.width() > 640)
    //  img = img.scaledToWidth(640, Qt::SmoothTransformation);
    if (img.width() > 960)
      img = img.scaledToWidth(img.width()/2, Qt::SmoothTransformation);
    images.append(img);
  }
}

double PhotoConsistency::vote(Point3 x) const
{
  double sum = 0.0;
  for (int i = 0, n = cameras.size(); i < n; ++i) {
    VoxelScore1 score(cameras, images, i, x, voxel_size);

    // NOTE: thresholding to eliminate outliers
    double v = score.vote();
    sum += (v >= VotingThreshold ? v : 0.0);
    //sum = fmax(sum, v);
  }
  return sum;
}

double PhotoConsistency::VotingThreshold = 0.5;

}
