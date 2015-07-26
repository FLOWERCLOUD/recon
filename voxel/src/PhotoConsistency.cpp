#include "VoxelScore1.h"
#include "PhotoConsistency.h"
#include <algorithm>
#include <math.h>

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

static double otsu_threshold(const QList<double>& _votes)
{
  // copy
  QList<double> votes = _votes;
  // sort votes
  std::sort(votes.begin(), votes.end());
  // convert to integral votes
  for (int i = 0, n = votes.size(); i < n; ++i)
    votes[i] = votes.at(i) / (double)n;
  for (int i = 1, n = votes.size(); i < n; ++i)
    votes[i] = votes.at(i-1) + votes.at(i);
  // Otsu Method
  // find argmax{ inter-class variance }
  double answer_t = 0.0;
  double max_var = 0.0;
  for (int i = 1, n = votes.size(); i < n; ++i) {
    // split into { 0 ... i-1 }, { i ... n-1 }
    // compute weights of two classes
    double w1 = (double)i / (double)n;
    double w2 = 1.0 - w1;
    // compute means of two classes
    double u1 = votes.at(i-1) / w1;
    double u2 = (votes.at(n-1) - votes.at(i-1)) / w2;
    double ud = u1 - u2;
    // compute inter-class variance
    double sb = w1 * w2 * ud * ud;
    // check if maxima
    if (max_var < sb) {
      answer_t = (votes.at(i) + votes.at(i-1)) * 0.5;
      max_var = sb;
    }
  }
  return answer_t;
}

double PhotoConsistency::vote(Point3 x) const
{
  QList<double> votes;
  votes.reserve(cameras.size());
  for (int i = 0, n = cameras.size(); i < n; ++i) {
    VoxelScore1 score(cameras, images, i, x, voxel_size);
    votes.append(score.vote());
  }
  double threshold = 0.0;
  if (EnableAutoThresholding) {
    threshold = otsu_threshold(votes);
    threshold = fmax(threshold, VotingThreshold);
  } else {
    threshold = VotingThreshold;
  }

  double sum = 0.0;
  for (int i = 0, n = votes.size(); i < n; ++i) {
    double v = votes.at(i);
    //sum += v;
    sum += (v >= threshold ? v : 0.0);
    //sum = fmax(sum, v);
  }
  return sum;
}

bool PhotoConsistency::EnableAutoThresholding = true;
double PhotoConsistency::VotingThreshold = 0.0;

}
