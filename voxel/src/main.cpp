#include "CameraData.h"
#include "CameraLoader.h"
#include <boost/program_options.hpp>
#include <iostream>

namespace po = boost::program_options;

int main(int argc, char* argv[])
{
  po::options_description desc("Allowed options");
  desc.add_options()
    ("help", "print this message")
    ("bundle", po::value<std::string>(), "set input SFM bundle file")
  ;

  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, desc), vm);
  po::notify(vm);

  if (vm.count("help")) {
    std::cout << desc << std::endl;
    return 0;
  }

  if (!vm.count("bundle")) {
    std::cout << "input bunlde is not set\n";
    return 0;
  }

  std::vector<std::string> images;
  std::vector<voxel::CameraData> cameras;

  if (voxel::load_from_nvm(images, cameras, vm["bundle"].as<std::string>())) {
    int n = cameras.size();
    std::cout << "# of cameras = " << n << "\n";
    for (int i = 0; i < n; ++i) {
      std::cout << "image = " << images[i] << "\n";
    }
  } else {
    std::cout << "failed to open " << vm["bundle"].as<std::string>() << std::endl;
  }

  return 0;
}
