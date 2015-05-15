#ifndef MARCHINGCUBE_H
#define MARCHINGCUBE_H

#include <vector>
#include "util.hpp"

using namespace std;

class MarchingCube{
public:
	static vector<Triangle> polygonise(GridCell &grid, double isolevel);
	static vector<double> vertexinterp(double isolevel, vector<double> &p1, vector<double> &p2, double v1, double v2);
};

#endif