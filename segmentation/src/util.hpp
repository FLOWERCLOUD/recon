#ifndef UTIL_H
#define UTIL_H

#include <opencv2/opencv.hpp>
#include <cmath>
#include "slic.hpp"

using namespace cv;

class GridCell{
public:
	vector< vector<double> > p; // size = 8x3 (indicate the position of 8 corners in a cell)
	vector<double> value;		// size = 8   (indicate the value of 8 corners in a cell)
	GridCell();
};

class Triangle{
public:
	vector< vector<double> > p; // size = 3x3 (indicate the position of 3 points) 
	Triangle();
};

class Node{
public:	
	int dest;
	double weight;

	Node(int d, double w){
		dest = d;
		weight = w;
	}
};

class Camera{
public:
	double f;		// focal length
	double k1, k2;	// radial distortion coeffs
	vector< vector<double> > R;	// 3x3 matrix representing camera rotation
	vector<double> t;			// 3x1 vector representing camera translation
	vector<double> p;        	// 3x1 vector representing camera position
	vector<double> d;	        // 3x1 vector representing camera viewing direction
	Mat_<Vec3b> image;
	Mat_<uchar> silhouette;
	Slic pset;

	void initsilhouette();
	void drawsilhouette(vector<bool> &segmentation);
	void updatesilhouette(vector<Triangle> &triangles);
	void projecttriangle(vector<Triangle> &triangles, vector<Point> &projection);
	void showinfo();
	Camera();
};

class Util{
public:
	static vector<double> multiplyVk(vector<double> &V, double k);
	static vector<double> multiplyMV(vector<vector<double> > &M, vector<double> &V);
	static vector<double> addVV(vector<double> &V1, vector<double> &V2);
	
	static vector<vector<double> > multiplyMk(vector<vector<double> > &M, double k);
	static vector<vector<double> > multiplyMM(vector<vector<double> > &M1, vector<vector<double> > &M2);
	static vector<vector<double> > transposeM(vector<vector<double> > &M);
	static vector<vector<double> > quaternion2R(vector<double> &q);

	static double dotVV(vector<double> &V1, vector<double> &V2);
	static double norm(vector<double> &V);
};

#endif
