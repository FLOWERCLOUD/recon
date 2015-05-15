#ifndef VISUALHULL_H
#define VISUALHULL_H

#include <iostream>
#include <vector>
#include <cmath>
#include "util.hpp"
#include "voxel.hpp"


using namespace std;

class VisualHull{
public:
	static vector<vector<double> > findbbox(vector<Camera> &cameras);
	static vector<vector<double> > project(Camera &camera, Voxel &voxels);
	static void carve(Camera &camera, Voxel &voxels);
};

#endif