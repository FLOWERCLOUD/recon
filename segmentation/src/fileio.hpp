#ifndef FILEIO_H
#define FILEIO_H

#include <iostream>
#include <cstdio>
#include <vector>
#include "util.hpp"
#include "voxel.hpp"
#include "slic.hpp"

using namespace std;

/******************************************************************************
 DESCRIPTION:
	Loads information from visualSFM output file and returns a structure 
	array containing camera definitions. Each camera contains the image,
	silhouette, and camera parameters.

 ARGUMENT:
	dirname: the directory name which contains image list and visualSFM
			 output file(.nvm).

 RETURN:
	cameras: camera definitions structure array.


 fchicken @ May,4 2015
*******************************************************************************/
vector<Camera> loadcameranvm(string dirname){
	cerr << "INFO/FILEIO.LOADCAMERANVM: " << endl;

	// load sparse.nvm
	string filename = dirname + "/sparse.nvm";
	FILE *fp_nvm = fopen(filename.c_str(), "r");
	if(fp_nvm == NULL){
		cerr << "ERROR/FILEIO.LOADCAMERANVM: can't open " + filename << endl;
		exit(EXIT_FAILURE);
	}
	
	bool readsegmentfromtxt = true;
	FILE *fp_segment = fopen("segmentresult.txt", "r");
	if(fp_segment == NULL){
		readsegmentfromtxt = false;	
	}

	char buffer[100];
	fgets(buffer, 100, fp_nvm);
	fgets(buffer, 100, fp_nvm);

	int camnum;
	fscanf(fp_nvm, "%d", &camnum);

	vector<Camera> cameras;
	for(int c = 0; c < camnum; c++){
		Camera camera;
		
		char imgname[1000];
		fscanf(fp_nvm, "%s%lf", imgname, &camera.f);

		vector<double> q(4, 0.f);
		fscanf(fp_nvm, "%lf%lf%lf%lf", &q[0], &q[1], &q[2], &q[3]);
		camera.R = Util::quaternion2R(q);

		for(int i = 0; i < 3; i++){
			fscanf(fp_nvm, "%lf", &camera.p[i]);
		}
		fscanf(fp_nvm, "%lf%*d", &camera.k1);

		// calculate camera viewing direction = R'*[0 0 -1]
		vector<double> tmpV(3, 0.f); tmpV[2] = -1.f;
		vector<vector<double> > tmpR = Util::transposeM(camera.R);
		camera.d = Util::multiplyMV(tmpR, tmpV);

		// calculate camera translation = -R*p
		tmpR = Util::multiplyMk(camera.R, -1.f);
		camera.t = Util::multiplyMV(tmpR, camera.p);

		filename = dirname + "/" + imgname;
		camera.image = imread(filename, CV_LOAD_IMAGE_COLOR);
		camera.silhouette = Mat::ones(camera.image.rows, camera.image.cols, CV_8U)*255; 
		if(readsegmentfromtxt){
			camera.pset.readsegmentresult(fp_segment);
			//if(c == 0){
			//	camera.pset.drawsegmentresult(camera.image);
			//	camera.pset.drawspresult(camera.image);
			//	imwrite("segmentfromtxt.jpg", camera.image);
			//}
		}
		else{
			camera.pset.segment(camera.image, 2000);
			camera.pset.writesegmentresult("segmentresult.txt");
			//if(c == 0){
			//	camera.pset.drawsegmentresult(camera.image);
			//	camera.pset.drawspresult(camera.image);
			//	imwrite("segment.jpg", camera.image);
			//}
		}
		cameras.push_back(camera);
	}

	fclose(fp_nvm);
	if(readsegmentfromtxt) fclose(fp_segment);

	return cameras;
}


/******************************************************************************
 DESCRIPTION:
	Loads information from bundler output file and returns a structure 
	array containing camera definitions. Each camera contains the image,
	silhouette, and camera parameters.

 ARGUMENT:
	dirname: the directory name which contains image list and bundler
			 output file(.out).

 RETURN:
	cameras: camera definitions structure array.


 fchicken @ April,25 2015
*******************************************************************************/
vector<Camera> loadcamerabundler(string dirname){
	cerr << "INFO/FILEIO.LOADCAMERABUNDLER: " << endl;

	// load bundler.out
	string filename = dirname + "/bundle.out";
	FILE *fp_bundler = fopen(filename.c_str(), "r");
	if(fp_bundler == NULL){
		cerr << "ERROR/FILEIO.LOADCAMERABUNDLER: can't open " + filename << endl;
		exit(EXIT_FAILURE);
	}

	// load list.txt
	filename = dirname + "/list.txt";
	FILE *fp_list = fopen(filename.c_str(), "r");
	if(fp_list == NULL){
		cerr << "ERROR/FILEIO.LOADCAMERABUNDLER: can't open " + filename << endl;
		exit(EXIT_FAILURE);		
	}
	
	bool readsegmentfromtxt = true;
	FILE *fp_segment = fopen("segmentresult.txt", "r");
	if(fp_segment == NULL){
		readsegmentfromtxt = false;	
	}

	char buffer[100];
	fgets(buffer, 100, fp_bundler);

	int camnum, pointnum;
	fscanf(fp_bundler, "%d%d", &camnum, &pointnum);

	vector<Camera> cameras;
	for(int c = 0; c < camnum; c++){
		Camera camera;
		fscanf(fp_bundler, "%lf%lf%lf", &camera.f, &camera.k1, &camera.k2);
		for(int i = 0; i < 3; i++){
			for(int j = 0; j < 3; j++){
				fscanf(fp_bundler, "%lf", &camera.R[i][j]);
			}
		}
		for(int i = 0; i < 3; i++){
			fscanf(fp_bundler, "%lf", &camera.t[i]);
		}

		// calculate camera viewing direction = R'*[0 0 -1]
		vector<double> tmpV(3, 0.f); tmpV[2] = -1.f;
		vector<vector<double> > tmpR = Util::transposeM(camera.R);
		camera.d = Util::multiplyMV(tmpR, tmpV);

		// calculate camera position = -R'*t
		tmpR = Util::multiplyMk(tmpR, -1);
		camera.p = Util::multiplyMV(tmpR, camera.t);

		char imgname[1000];
		fscanf(fp_list, "%s", imgname);
		filename = dirname + "/" + imgname;
		camera.image = imread(filename, CV_LOAD_IMAGE_COLOR);
		camera.silhouette = Mat::ones(camera.image.rows, camera.image.cols, CV_8U)*255; 
		if(readsegmentfromtxt){
			camera.pset.readsegmentresult(fp_segment);
		}
		else{
			camera.pset.segment(camera.image, 2000);
			camera.pset.writesegmentresult("segmentresult.txt");
		}
		cameras.push_back(camera);
	}
	fclose(fp_bundler);
	fclose(fp_list);
	if(readsegmentfromtxt) fclose(fp_segment);

	return cameras;
}

vector<Camera> loadcamera(string dirname, string mode){
	if(mode.compare("bundler") == 0){
		return loadcamerabundler(dirname);
	}
	else if(mode.compare("nvm") == 0){
		return loadcameranvm(dirname);
	}
	else{
		cerr << "ERROR/FILEIO.LOADCAMERA: mode doesn't exist" << endl;
		exit(EXIT_FAILURE);
	}
}

/******************************************************************************
 DESCRIPTION:
	Write out current [cameras] and [voxels] info to visualize in matlab.

 ARGUMENT:
	cameras: cameras in the scene.
	voxels: voxels in the scene.


 fchicken @ April,25 2015
*******************************************************************************/
void write2visualize(vector<Camera> &cameras, vector<int> &neighbor, Voxel &voxels){
	FILE *fp = fopen("visualize.txt", "w");

	fprintf(fp, "%d %d\n", (int)cameras.size(), (int)voxels.p.size());
	for(int c = 0; c < cameras.size(); c++){
		for(int i = 0; i < 3; i++){
			for(int j = 0; j < 3; j++){
				fprintf(fp, "%lf ", cameras[c].R[i][j]);
			}
			fprintf(fp, "\n");
		}
		for(int i = 0; i < 3; i++){
			fprintf(fp, "%lf ", cameras[c].t[i]);
		}
		fprintf(fp, "\n");
		fprintf(fp, "%d\n", neighbor[c]);
	}
	fprintf(fp, "%lf\n", voxels.resolution);
	for(int v = 0; v < voxels.p.size(); v++){
		for(int i = 0; i < 3; i++){
			fprintf(fp, "%lf ", voxels.p[v][i]);
		}
		fprintf(fp, "\n");
	}
	fclose(fp);
	return;
}

void write2ply(string filename, vector<Triangle> &triangles){
	cerr << "INFO/FILEIO.WRITE2PLY: ";

	FILE *fp = fopen(filename.c_str(), "w");
	fprintf(fp, "ply\n");
	fprintf(fp, "format ascii 1.0\n");
	fprintf(fp, "element vertex %d\n", (int)triangles.size()*3);
	fprintf(fp, "property float x\n");
	fprintf(fp, "property float y\n");
	fprintf(fp, "property float z\n");
	fprintf(fp, "element face %d\n", (int)triangles.size());
	fprintf(fp, "property list uchar int vertex_index\n");
	fprintf(fp, "end_header\n");
	for(int t = 0; t < triangles.size(); t++){
		for(int i = 0; i < 3; i++){
			fprintf(fp, "%lf %lf %lf\n", triangles[t].p[i][0], triangles[t].p[i][1], triangles[t].p[i][2]); 
		}
	}
	for(int t = 0; t < triangles.size(); t++){
		fprintf(fp, "3 %d %d %d\n", t*3, t*3+1, t*3+2);
	}

	cerr << "	done." << endl;
	fclose(fp);
	return;
}

#endif