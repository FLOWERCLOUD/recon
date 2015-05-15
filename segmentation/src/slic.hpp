#ifndef SLIC_H
#define SLIC_H

#include <opencv2/opencv.hpp>
#include <vl/generic.h>
#include <vl/slic.h>
#include <iostream>
#include <cstdio>


using namespace cv;
using namespace std;

class Slic{
public:
	Mat_<int> labels;			// segmentation labels
	vector<int> group;			// group list
	vector< vector<double> > position;	// mean position of each group (image center at (0, 0, 1))
	vector< vector<double> > color;		// mean color of each group (bgr)
	int segmentnum;

	void segment(Mat_<Vec3b> &image, int N);
	void getspinfo(Mat_<Vec3b> &image);
	void drawsegment(Mat_<Vec3b> &image, int target, vector<double> color);
	void drawsegmentresult(Mat_<Vec3b> &image);
	void drawspresult(Mat_<Vec3b> &image);
	void writesegmentresult(string filename);
	void readsegmentresult(FILE *fp);
};

#endif