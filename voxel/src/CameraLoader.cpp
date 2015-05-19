#include "CameraLoader.h"
#include "CameraData.h"

#include <fstream>
#include <boost/filesystem.hpp>
//#include <boost/gil/gil_all.hpp>
//#include <boost/gil/extension/io/jpeg_io.hpp>

namespace voxel {

bool load_from_nvm(std::vector<std::string>& images,
                   std::vector<CameraData>& cameras,
                   const std::string& path)
{
  std::ifstream stream(path);
  if (!stream.is_open())
    return false;

  // Check file type
  {
    std::string magic;
    stream >> magic;
    if (magic != "NVM_V3")
      return false;
  }

  // Camera count
  {
    int ncams;
    stream >> ncams;
    if (ncams < 1)
      return false;

    cameras.resize(ncams);
    images.resize(ncams);
  }

  boost::filesystem::path bundledir(path);
  bundledir.remove_filename();

  // Camera data
  {
    std::string imagename;
    float focal;
    float orient[4]; // XYZW
    float center[3];
    float distortion;
    int temp;

    auto it = cameras.begin(), itend = cameras.end();
    for (; it != itend; ++it) {
      stream >> imagename;
      stream >> focal;
      stream >> orient[3] >> orient[0] >> orient[1] >> orient[2];
      stream >> center[0] >> center[1] >> center[2];
      stream >> distortion;
      stream >> temp; // END of camera

      boost::filesystem::path imagepath(imagename);
      if (imagepath.is_relative())
        imagepath = bundledir / imagepath;

      images[it - cameras.begin()] = imagepath.string();

      //boost::gil::point2<ptrdiff_t> dim = boost::gil::jpeg_read_dimensions(imagepath.string());
      //float aspect = ((dim.x > 0 && dim.y > 0) ? (float)dim.x / (float)dim.y : 1.0f);
      float aspect = 1.0f;

      CameraData& cam = *it;
      cam.focal_length = focal;
      cam.aspect_ratio = aspect;
      cam.radial_distortion[0] = distortion;
      cam.radial_distortion[1] = 0.0f;
      memcpy(cam.center, center, sizeof(float)*3);
      memcpy(cam.orientation, orient, sizeof(float)*4);

      cam.update_extrinsic();
      cam.update_intrinsic();
    }
  }

  return true;
}

}
