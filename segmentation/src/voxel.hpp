#ifndef VOXEL_H
#define VOXEL_H

#include <iostream>
#include <vector>
#include <cmath>
#include "util.hpp"
#include "marchingcube.hpp"

using namespace std;

class Voxel{
public:
	vector<vector<double> > p;  // (x, y, z, index of value)
	vector<double> value;		// background = 0, object = 1
	double resolution;			// voxel resolution

	vector<vector<double> > bbox;	// initial bounding box used to make this voxel array
	vector<Triangle> triangles; 	// triangle list of the mesh

	void makevoxels(vector<vector<double> > &box, int N);
	void isosurface(double isovalue);
	vector<vector<double> > currentbbox();
};

#endif