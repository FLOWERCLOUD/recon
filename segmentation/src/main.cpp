#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include <cassert>
#include "fileio.hpp"
#include "util.hpp"
#include "voxel.hpp"
#include "visualhull.hpp"
#include "graph.h"

#define PI 3.14159265
#define LAMBDA 1 // color coherence coefficient 
#define FOREGROUND 1
#define BACKGROUND 0
#define EPSILON 0.01
#define K 15 // GMM cluster number

using namespace std;

typedef Graph<double,double,double> GraphType;
double graphedgenum;
double graphnodenum;

vector<vector<double> > getfundamental(Camera &cam1, Camera &cam2);
vector<int> getneighbors(vector<Camera> &cameras, int target);
double getspradius(vector<Camera> &cameras);
vector<int> getspoffset(vector<Camera> &cameras);
double colorcoherence(vector<double> &c1, vector<double> &c2);
vector<Vec3b> getsamples(vector<Camera> cameras, int mode, int N);
vector<vector<Node> > construcW(vector<Camera> &cameras, EM &bgmodel, EM &fgmodel);
void updateW(vector<vector<Node> > &W, vector<Camera> &cameras, EM &bgmodel, EM &fgmodel);
GraphType *constructG(vector<vector<Node> > &W);
double getweight(vector<vector<Node> > &W, int u, int v);
vector<bool> getmincutresult(GraphType *g);
bool converge(vector<bool> label, vector<bool> lastlabel);

void writeWresult(vector<vector<Node> > &W);
vector<vector<Node> > readWresult();


int main(int argc, char** argv){

	// load camera information including slic segmentation
	vector<Camera> cameras = loadcamera(argv[1], "nvm");

	// find initial visible volumn
	vector<vector<double> > vvbbox = VisualHull::findbbox(cameras);	
	Voxel visiblevolumn;
	visiblevolumn.makevoxels(vvbbox, 1000000);
	for(int i = 0; i < cameras.size(); i++){
		VisualHull::carve(cameras[i], visiblevolumn);
	}

	// shrink the bbox according to current voxels limits
	vector<vector<double> > bbox = visiblevolumn.currentbbox();

	// backproject visible volumn to each view
	visiblevolumn.isosurface(0.5);
	cerr << "INFO/MAIN.UPDATESILHOUETTE: " << endl << "	camera ";
	for(int i = 0; i < cameras.size(); i++){
		cerr << i << " ";
		cameras[i].updatesilhouette(visiblevolumn.triangles);
	}
	cerr << "	done." << endl;

	// initialize the silhouette for the algorithm
	cerr << "INFO/MAIN.INITSILHOUETTE: " << endl << "	camera ";
	for(int i = 0; i < cameras.size(); i++){
		cerr << i << " ";
		cameras[i].initsilhouette();
		char filename[1000];
		sprintf(filename, "output/updatesilhouette%.3d_iteration000.bmp", i);
		imwrite(filename, cameras[i].silhouette);
	}
	imwrite("initsilhouette.bmp", cameras[0].silhouette);
	cerr << "	done." << endl;
	
	// train background model	
	cerr << "INFO/MAIN.TRAINBACKGROUNDMODEL: " << endl;
	vector<Vec3b> bg = getsamples(cameras, BACKGROUND, 10000);
	Mat bgsamples = Mat(bg).reshape(1);
	EM bgmodel(K);
	bgmodel.train(bgsamples);
	cerr << "	done." << endl;
	imwrite("output/samplesbg000.bmp", Mat(bg).reshape(0, 100));
	
	// train foreground model
	cerr << "INFO/MAIN.TRAINFOREGROUNDMODEL: " << endl;
	vector<Vec3b> fg = getsamples(cameras, FOREGROUND, 10000);
	Mat fgsamples = Mat(fg).reshape(1);
	EM fgmodel(K);
	fgmodel.train(fgsamples);
	cerr << "	done." << endl;
	imwrite("output/samplesfg000.bmp", Mat(fg).reshape(0, 100));

	cerr << "drawing testing" << endl;
	for(int i = 0; i < cameras.size(); i++){
		Slic s = cameras[i].pset;
		for(int ii = 0; ii < s.group.size(); ii++){
			double bgprob = exp(bgmodel.predict(s.color[ii])[0]);
			double fgprob = exp(fgmodel.predict(s.color[ii])[0]);
			double bgnormal = bgprob/(bgprob+fgprob);
			double fgnormal = fgprob/(bgprob+fgprob);
			//cerr << bgnormal << " " << fgnormal << " (" << s.color[ii][0] << ", " << s.color[ii][1] << ", " << s.color[ii][2] << endl;
			assert(bgnormal >= 0 && fgnormal >= 0);
			
			double centerx = (cameras[i].image.cols-1)/2;
			double centery = (cameras[i].image.rows-1)/2;
			int x = round(s.position[ii][0]+centerx);
    		int y = round((cameras[i].image.rows-1)-(s.position[ii][1]+centery));
    		if(bgnormal > fgnormal)
    			circle(cameras[i].image, Point(x, y), 3, Scalar(255, 0, 0), -1, 8);
    		else
    			circle(cameras[i].image, Point(x, y), 3, Scalar(0, 0, 255), -1, 8);
		}
		char filename[1000];
		sprintf(filename, "output/testcolormodels%.3d.bmp", i);
		imwrite(filename, cameras[i].image);
	}

	// construct edge matrix
	//vector<vector<Node> > W = readWresult();
	vector<vector<Node> > W = construcW(cameras, bgmodel, fgmodel);
	
	vector<bool> label(graphnodenum, BACKGROUND);
	vector<bool> lastlabel(graphnodenum, FOREGROUND);
	
	int iteration = 0;
	while(true){
		iteration ++;
		cerr << "**************************************** iteration " << iteration << " ****************************************" << endl;

		// turn W matrix to graph
		GraphType *g = constructG(W);
		
		// run s-t mincut algorithm for segmentation
		cerr << "INFO/MAIN.MAXFLOW:" << endl;
		g -> maxflow();
		cerr << "	done." << endl;

		// get new silhouette according to current segmentation
		cerr << "INFO/MAIN.DRAWSILHOUETTE:" << endl << "	camera ";
		lastlabel = label;
		label = getmincutresult(g);
		vector<int> offset = getspoffset(cameras);
		for(int i = 0; i < cameras.size(); i++){
			cerr << i << " ";
			vector<bool> segmentation(label.begin()+offset[i], label.begin()+offset[i+1]);
			cameras[i].drawsilhouette(segmentation);
			char filename[1000];
			sprintf(filename, "output/drawsilhouette%.3d_iteration%.3d.bmp", i, iteration);
			imwrite(filename, cameras[i].silhouette);
		}
		cerr << "	done." << endl;

		// visualhull of new silhouette
		Voxel voxels;
		voxels.makevoxels(bbox, 10000000);
		for(int i = 0; i < cameras.size(); i++){
			VisualHull::carve(cameras[i], voxels);
		}

		// back project the visuall hull to get segmentation
		voxels.isosurface(0.5);
		char meshname[100];
		sprintf(meshname, "mesh_iteration%.3d.ply", iteration);
		write2ply(meshname, voxels.triangles);
		cerr << "INFO/MAIN.UPDATESILHOUETTE:" << endl << "	camera ";
		for(int i = 0; i < cameras.size(); i++){
			cerr << i << " ";
			cameras[i].updatesilhouette(voxels.triangles);
			char filename[1000];
			sprintf(filename, "output/updatesilhouette%.3d_iteration%.3d.bmp", i, iteration);
			imwrite(filename, cameras[i].silhouette);
		}
		cerr << "	done." << endl;

		// end the while if it's converge
		if(converge(label, lastlabel) || iteration == 10) break;

		// train foreground model
		cerr << "INFO/MAIN.TRAINFOREGROUNDMODEL:" << endl;
		fg = getsamples(cameras, FOREGROUND, 50000);
		fgsamples = Mat(fg).reshape(1);
		fgmodel.train(fgsamples);
		cerr << "	done." << endl;
		char filename[1000];
		sprintf(filename, "output/samplesfg%.3d.bmp", iteration);
		imwrite(filename, Mat(fg).reshape(0, 100));


		// update the W matrix about s and t node
		updateW(W, cameras, bgmodel, fgmodel);
		
		delete g;
	}

	return 0;
}

/******************************************************************************
 DESCRIPTION:
	Get the fundamental matrix from [cam1] to [cam2].
	x2'* F * x1 = 0

 ARGUMENT:
	cam1: the source camera.
	cam2: the target camera.

 OUTPUT:
	result: the fundamental matrix.
*******************************************************************************/
vector<vector<double> > getfundamental(Camera &cam1, Camera &cam2){

	// project e2 = P2 * C1
	vector<double> e2 = Util::multiplyMV(cam2.R, cam1.p);
	e2 = Util::addVV(e2, cam2.t);
	e2[0] = e2[0] * cam2.f;
	e2[1] = e2[1] * cam2.f;

	// build e2x
	vector<vector<double> > e2x(3, vector<double>(3));
	e2x[0][0] =  0.f;    e2x[0][1] = -e2[2];  e2x[0][2] =  e2[1];
	e2x[1][0] =  e2[2];  e2x[1][1] =  0.f;    e2x[1][2] = -e2[0];
	e2x[2][0] = -e2[1];  e2x[2][1] =  e2[0];  e2x[2][2] =  0.f;

	// calculate P1^-1
	Mat_<double> P1(3, 4);
	Mat_<double> P2(3, 4);
	Mat_<double> pinvP1(4, 3);
	for(int i = 0; i < 3; i++){
		for(int j = 0; j < 3; j++){
			P1(i, j) = cam1.R[i][j];
			P2(i, j) = cam2.R[i][j];
		}
		P1(i, 3) = cam1.t[i];
		P2(i, 3) = cam2.t[i];
	}
	for(int i = 0; i < 2; i++){
		for(int j = 0; j < 4; j++){
			P1(i, j) = cam1.f * P1(i, j);
			P2(i, j) = cam2.f * P2(i, j);
		}
	}
	pinvP1 = P1.inv(DECOMP_SVD);

	// calculate P2 * P1^-1
	Mat_<double> tmpM = P2 * pinvP1;
	vector<vector<double> > tmp(3, vector<double>(3));
	for(int i = 0; i < 3; i++){
		for(int j = 0; j < 3; j++){
			tmp[i][j] = tmpM(i, j);
		}
	}

	// calculate F = e2x * P2 * P1^-1
	vector<vector<double> > result = Util::multiplyMM(e2x, tmp);
	return result;
}

/******************************************************************************
 DESCRIPTION:
	Decide which camera is current camera's neighbor. 
	value 0 means nothing, 1 means it's neighbor, 2 means it's current camera.

 ARGUMENT:
	cameras: the camera array.
	target: current camera index.

 OUTPUT:
	neighbor: the neighbor indicate array.
*******************************************************************************/
vector<int> getneighbors(vector<Camera> &cameras, int target){
	vector<int> neighbor(cameras.size(), 0);
	Camera c = cameras[target];
	for(int i = 0; i < cameras.size(); i++){
		if(i == target){
			neighbor[i] = 2;
			continue;
		}
		double tmp = Util::dotVV(c.d, cameras[i].d);
		double angle = acos(tmp) * 180.f / PI;
		if(angle < 22.5){
			neighbor[i] = 1;
		}
	}
	return neighbor;
}

/******************************************************************************
 DESCRIPTION:
	Calculate the mean radius of super pixels.

 ARGUMENT:
	cameras: the camera array.

 RETURN:
	the mean radius.
*******************************************************************************/
double getspradius(vector<Camera> &cameras){
	double spnum = 0;
	for(int i = 0; i < cameras.size(); i++){
		spnum += cameras[i].pset.group.size();
	}

	// average number of super-pixels in an image
	spnum = spnum / (double)cameras.size();

	// define radius = sqrt(super-pixels area) / 2
	double imagesize = cameras[0].image.rows * cameras[0].image.cols;
	double radius = sqrt(imagesize/spnum)/2;
	
	return radius;
}

/******************************************************************************
 DESCRIPTION:
	Accumulate super pixel number according to [cameras].

 ARGUMENT:
	cameras: the camera array.

 RETURN:
	offset array.
*******************************************************************************/
vector<int> getspoffset(vector<Camera> &cameras){
	vector<int> offset(1, 0);
	for(int i = 0; i < cameras.size(); i++){
		offset.push_back(offset[i] + cameras[i].pset.group.size());
	}
	return offset;
}

/******************************************************************************
 DESCRIPTION:
	Calculate the color coherence of [c1] and [c2] .

 ARGUMENT:
	c1: the color.
	c2: the color.

 RETURN:
	the color coherence score.
*******************************************************************************/
double colorcoherence(vector<double> &c1, vector<double> &c2){
	vector<double> mc2 = Util::multiplyVk(c2, -1);
	vector<double> diff = Util::addVV(c1, mc2);
	return exp(-LAMBDA*Util::dotVV(diff, diff));
}

/******************************************************************************
 DESCRIPTION:
	Get [N] background or foreground samples from silhouette image in [cameras].

 ARGUMENT:
	cameras: camera array which contain silhouette images.

 RETURN:
	sample point array.
*******************************************************************************/
vector<Vec3b> getsamples(vector<Camera> cameras, int mode, int N){
	int n = ceil((double)N/cameras.size());
	srand(time(NULL));

	vector<Vec3b> samples;
	int rows = cameras[0].silhouette.rows;
	int cols = cameras[0].silhouette.cols;
	for(int c = 0; c < cameras.size(); c++){
		int nsample = 0;
		while(nsample != n){
			int i = rand()%rows;
			int j = rand()%cols;
			// if mode = BACKGROUND(0), we should find silhouette = 0
			//    mode = FOREGROUND(1), we should find silhouette = 255
			if(cameras[c].silhouette(i, j) == mode*255){
				samples.push_back(cameras[c].image(i, j));
				nsample++;
				if(samples.size() == N) break;
			}
		}
	}
	return samples;
}

/******************************************************************************
 DESCRIPTION:
	Construct the W matrix of the graph.

 ARGUMENT:
	cameras: the camera array. 

 RETURN:
	the W matrix. 
	W[i] = Node(j, score) means there is an edge from i to j with weight = score.
*******************************************************************************/
vector<vector<Node> > construcW(vector<Camera> &cameras, EM &bgmodel, EM &fgmodel){
	cerr << "INFO/MAIN.CONSTRUCTW:" << endl;

	vector<int> offset = getspoffset(cameras);	
	graphedgenum = 0;
	graphnodenum = offset[cameras.size()];
	vector<vector<Node> > W( graphnodenum+2 );

	cerr << "	construct W - st node" << endl;
	int source = W.size()-2;
	int target = W.size()-1;
	for(int i = 0; i < cameras.size(); i++){
		Slic s = cameras[i].pset;
		for(int ii = 0; ii < s.color.size(); ii++){

			double bgprob = exp(bgmodel.predict(s.color[ii])[0]);
			double fgprob = exp(fgmodel.predict(s.color[ii])[0]);
			double bgnormal = bgprob/(bgprob+fgprob);
			double fgnormal = fgprob/(bgprob+fgprob);

			int current = offset[i] + ii;
			Node ns(current, fgnormal);
			Node nt(current, bgnormal);
			W[source].push_back(ns);
			W[target].push_back(nt);
		}
	}

	cerr << "	construct W - intra image" << endl << "	camera ";
	// construct W matrix - intra image
	double radius = getspradius(cameras);
	for(int i = 0; i < cameras.size(); i++){
		cerr << i << " ";
		Slic s = cameras[i].pset;
		for(int ii = 0; ii < s.position.size(); ii++){
			for(int jj = 0; jj < s.position.size(); jj++){
				if(ii == jj) continue;
				vector<double> mii = Util::multiplyVk(s.position[ii], -1);
				vector<double> diff = Util::addVV(s.position[jj], mii);
				double distance = sqrt(Util::dotVV(diff, diff));
				if(distance < 2.2*radius){
					vector<double> c1 = s.color[ii];
					vector<double> c2 = s.color[jj];
					double w = colorcoherence(c1, c2);
					if(w < EPSILON) continue;
					
					graphedgenum ++;
					Node n(offset[i]+jj, w);
					W[offset[i]+ii].push_back(n);
				} 
			}
		}
	}
	cerr << endl;
	graphedgenum /= 2;

	cerr << "	construct W - inter image" << endl << "	camera ";
	// construct W matrix - inter image
	for(int i = 0; i < cameras.size(); i++){
		cerr << i << "(";
		vector<int> neighbor = getneighbors(cameras, i);
		int neighbornum = 0;
		for(int j = 0; j < neighbor.size(); j++){
		
			if(neighbor[j] != 1) continue;
			neighbornum++;
			Slic s1 = cameras[i].pset;
			Slic s2 = cameras[j].pset;
			
			vector<vector<double> > F = getfundamental(cameras[i], cameras[j]);
			for(int ii = 0; ii < s1.position.size(); ii++){
				
				vector<double> x1 = s1.position[ii];
				vector<double> c1 = s1.color[ii];
				vector<double> l = Util::multiplyMV(F, x1);

				for(int jj = 0; jj < s2.position.size(); jj++){
					vector<double> x2 = s2.position[jj];
					vector<double> c2 = s2.color[jj];
					double distance = abs(Util::dotVV(x2, l)) / sqrt(l[0]*l[0] + l[1]*l[1]);
					if(distance < radius){
						double w = colorcoherence(c1, c2);
						if(w < EPSILON) continue;
						
						graphedgenum ++;
						Node n(offset[j]+jj, w);
						W[offset[i]+ii].push_back(n);
					} 
				}	
			}

		}
		cerr << neighbornum << ") ";
	}
	cerr << endl;
	cerr << "	node # = " << graphnodenum << endl;
	cerr << "	edge # = " << graphedgenum << endl;
	cerr << "	done." << endl;
	writeWresult(W);
	return W;
}

/******************************************************************************
 DESCRIPTION:
	Update the link of s and t node of the W matrix.

 ARGUMENT:
 	W: the W matrix.
 	cameras: the camera array.
	bgmodel: the trained background model.
	fgmodel: the trained foreground model. 
*******************************************************************************/
void updateW(vector<vector<Node> > &W, vector<Camera> &cameras, EM &bgmodel, EM &fgmodel){
	cerr << "INFO/MAIN.UPDATEW:" << endl;
	
	int source = W.size()-2;
	int target = W.size()-1;
	
	vector<int> offset = getspoffset(cameras);	
	for(int i = 0; i < cameras.size(); i++){
		Slic s = cameras[i].pset;
		for(int ii = 0; ii < s.color.size(); ii++){
			
			double bgprob = exp(bgmodel.predict(s.color[ii])[0]);
			double fgprob = exp(fgmodel.predict(s.color[ii])[0]);
			double bgnormal = bgprob/(bgprob+fgprob);
			double fgnormal = fgprob/(bgprob+fgprob);

			int current = offset[i] + ii;
			W[source][current].weight = fgnormal;
			W[target][current].weight = bgnormal;
		}
	}
	cerr << "	done." << endl;
}

GraphType *constructG(vector<vector<Node> > &W){
	cerr << "INFO/MAIN.CONSTRUCTG:" << endl;
	cerr << "	construct G - tlink" << endl;
	GraphType *g = new GraphType(graphnodenum, graphedgenum);
	g -> add_node(graphnodenum);

	int source = W.size()-2;
	int target = W.size()-1;
	for(int i = 0; i < graphnodenum; i++){
		int node = i;
		double sweight = W[source][i].weight;
		double tweight = W[target][i].weight;
		g -> add_tweights(node, sweight, tweight);
	}
	cerr << "	construct G - nlink" << endl;
	for(int i = 0; i < graphnodenum; i++){
		for(int j = 0; j < W[i].size(); j++){
			int node1 = i;
			int node2 = W[i][j].dest;
			double w12 = getweight(W, node1, node2);
			double w21 = getweight(W, node2, node1);
			g -> add_edge(node1, node2, w12, w21);
		}
	}
	cerr << "	done.";
	return g;
}

double getweight(vector<vector<Node> > &W, int u, int v){
	for(int i = 0; i < W[u].size(); i++){
		if(W[u][i].dest == v){
			return W[u][i].weight;
		}
	}
	return 0.f;
}

vector<bool> getmincutresult(GraphType *g){
	int nodenum = g -> get_node_num();
	vector<bool> label(nodenum);
	for(int i = 0; i < label.size(); i++){
		label[i] = !(g->what_segment(i));
	}
	return label;
}

/******************************************************************************
 DESCRIPTION:
	Deside whether the labeling question is converge or not. If the result remains
	no change a lot since last time, consider it's converge.

 ARGUMENT:
 	label: current labeling result.
 	lastlabel: last labeling result.
*******************************************************************************/
bool converge(vector<bool> label, vector<bool> lastlabel){
	cerr << "INFO/MAIN.CONVERGE:" << endl;
	int threshold = (int)label.size()*0.001;
	int diffnum = 0;
	for(int i = 0; i < label.size(); i++){
		if(label[i] != lastlabel[i]) diffnum++;
	}
	cerr << "	diffnum = " << diffnum << endl;
	cerr << "	threshold = " << threshold << endl;
	return (diffnum < threshold);
}

void writeWresult(vector<vector<Node> > &W){
	FILE *fp = fopen("W.txt", "w");
	fprintf(fp, "%d\n", (int)W.size());
	for(int i = 0; i < W.size(); i++){
		fprintf(fp, "%d\n", (int)W[i].size());
		for(int j = 0; j < W[i].size(); j++){
			fprintf(fp, "%d %lf ", W[i][j].dest, W[i][j].weight);
		}
		fprintf(fp, "\n");
	}
	fclose(fp);
}

vector<vector<Node> > readWresult(){
	cerr << "INFO/MAIN.READWRESULT:" << endl;
	FILE *fp = fopen("W.txt", "r");
	int wsize;
	fscanf(fp, "%d", &wsize);
	vector<vector<Node> > W(wsize);
	for(int i = 0; i < wsize; i++){
		int wisize;
		fscanf(fp, "%d", &wisize);
		for(int j = 0; j < wisize; j++){
			int dest;
			double weight;
			fscanf(fp, "%d%lf", &dest, &weight);
			Node n(dest, weight);
			W[i].push_back(n);
		}
	}
	fclose(fp);
	cerr << "	done." << endl;
	return W;
}

