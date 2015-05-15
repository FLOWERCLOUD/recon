#include "util.hpp"

#define FOREGROUND 1
#define BACKGROUND 0
#define WHITE 255
#define GRAY 127
#define BLACK 0

GridCell::GridCell(){
	p.assign(8, vector<double>(3));
	value.assign(8, 0.f);
}

Triangle::Triangle(){
	p.assign(3, vector<double>(3));
}

void Camera::initsilhouette(){
	double centeri = (silhouette.rows-1)/2;
	double centerj = (silhouette.cols-1)/2;
	double threshold = (double)min(silhouette.rows, silhouette.cols)/40;
	for(int i = 0; i < silhouette.rows; i++){
		for(int j = 0; j < silhouette.cols; j++){
			double distance = sqrt(pow((i-centeri), 2) + pow((j-centerj), 2));
			if(distance < threshold){
				silhouette(i, j) = WHITE;
			}
			else if(silhouette(i, j) == WHITE){
				silhouette(i, j) = GRAY;
			}
		}
	}
	return;
}

void Camera::drawsilhouette(vector<bool> &segmentation){
	if(segmentation.size() != pset.group.size()){
		cerr << "ERROR/CAMERA.DRAWSILHOUETTE:" << endl;
		cerr << "	the size of segmentation and super-pixel number missmatch." << endl;
		exit(EXIT_FAILURE);
	}
	silhouette = Mat::zeros(image.rows, image.cols, CV_8U);
	for(int i = 0; i < silhouette.rows; i++){
		for(int j = 0; j < silhouette.cols; j++){
			// find which group the pixel is
			int g;
			for(g = 0; g < pset.group.size(); g++){
				if(pset.group[g] == pset.labels(i, j)) break;
			}
			if(segmentation[g] == FOREGROUND) silhouette(i, j) = WHITE;
		}
	}
	return;
}

void Camera::updatesilhouette(vector<Triangle> &triangles){	
	vector<Point> projection;
	projecttriangle(triangles, projection);

	silhouette = Mat::zeros(image.rows, image.cols, CV_8U);
	
	const Point* pointarray[1] = {};
	int pointnum[1] = {3};
	for(int i = 0; i < triangles.size(); i++){
		pointarray[0] = &projection[i*3];
		fillPoly(silhouette, pointarray, pointnum, 1, Scalar(WHITE), 8);
	}
	return;
}

void Camera::projecttriangle(vector<Triangle> &triangles, vector<Point> &projection){
	for(int i = 0; i < triangles.size(); i++){
		for(int j = 0; j < 3; j++){
			vector<double> p_world = triangles[i].p[j];
			
			vector<double> p_camera = Util::multiplyMV(R, p_world);
			p_camera = Util::addVV(p_camera, t);
		
			vector<double> p_pixel = Util::multiplyVk(p_camera, -1.f/p_camera[2]);
			p_pixel = Util::multiplyVk(p_pixel, f);

			double centerx = (image.cols-1)/2;
			double centery = (image.rows-1)/2;
			double x = p_pixel[0]+centerx;
			double y = (image.rows-1)-(p_pixel[1]+centery);

			Point p_image(x, y);
			projection.push_back(p_image);
		}
	}
}

void Camera::showinfo(){
	cerr << "position:    " << p[0] << " " << p[1] << " " << p[2] << endl;
	cerr << "translation: " << t[0] << " " << t[1] << " " << t[2] << endl;
	cerr << "direction:   " << d[0] << " " << d[1] << " " << d[2] << endl;
	cerr << "rotation:    " << R[0][0] << " " << R[0][1] << " " << R[0][2] << endl;
	cerr << "             " << R[1][0] << " " << R[1][1] << " " << R[1][2] << endl;
	cerr << "             " << R[2][0] << " " << R[2][1] << " " << R[2][2] << endl;
	cerr << endl; 
}

Camera::Camera(){
	R.assign(3, vector<double>(3));
	t.assign(3, 0.f);
	p.assign(3, 0.f);
	d.assign(3, 0.f);
}

vector<double> Util::multiplyVk(vector<double> &V, double k){
	int dimension = (int)V.size();

	vector<double> result(dimension, 0.f);
	for(int i = 0; i < dimension; i++){
			result[i] = k*V[i];
	}
	return result;
}

vector<double> Util::multiplyMV(vector<vector<double> > &M, vector<double> &V){
	int dimension1 = (int)M.size();
	int dimension2 = (int)M[0].size();

	vector<double> result(dimension1, 0.f);
	for(int i = 0; i < dimension1; i++){
		for(int j = 0; j < dimension2; j++){
			result[i] += M[i][j]*V[j];
		}
	}
	return result;
}

vector<double> Util::addVV(vector<double> &V1, vector<double> &V2){
	int dimension = (int)V1.size();

	vector<double> result(dimension, 0.f);
	for(int i = 0; i < dimension; i++){
		result[i] = V1[i] + V2[i];
	}
	return result;
}

vector<vector<double> > Util::multiplyMk(vector<vector<double> > &M, double k){
	int dimension1 = (int)M.size();
	int dimension2 = (int)M[0].size();

	vector<vector<double> > result(dimension1, vector<double>(dimension2));
	for(int i = 0; i < dimension1; i++){
		for(int j = 0; j < dimension2; j++){
			result[i][j] = k*M[i][j];
		}
	}
	return result;
}

vector<vector<double> > Util::multiplyMM(vector<vector<double> > &M1, vector<vector<double> > &M2){
	int dimension1 = (int)M1.size();
	int dimension2 = (int)M1[0].size();
	int dimension3 = (int)M2[0].size();

	vector<vector<double> > result(dimension1, vector<double>(dimension3));
	for(int i = 0; i < dimension1; i++){
		for(int j = 0; j < dimension3; j++){
			result[i][j] = 0.f;
			for(int k = 0; k < dimension2; k++){
				result[i][j] += (M1[i][k] * M2[k][j]);
			}
		}
	}
	return result;
}

vector<vector<double> > Util::transposeM(vector<vector<double> > &M){
	int dimension1 = (int)M.size();
	int dimension2 = (int)M[0].size();

	vector<vector<double> > result(dimension1, vector<double>(dimension2));
	for(int i = 0; i < dimension1; i++){
		for(int j = 0; j < dimension2; j++){
			result[i][j] = M[j][i];
		}
	}
	return result;
}

vector<vector<double> > Util::quaternion2R(vector<double> &q){
	vector<vector<double> > result(3, vector<double>(3));
	result[0][0] = 1.f - 2*q[2]*q[2] - 2*q[3]*q[3];
	result[0][1] = 2.f * (q[1]*q[2] - q[3]*q[0]);
	result[0][2] = 2.f * (q[1]*q[3] + q[2]*q[0]);
	result[1][0] = -(2.f * (q[1]*q[2] + q[3]*q[0]));
	result[1][1] = -(1.f - 2*q[1]*q[1] - 2*q[3]*q[3]);
	result[1][2] = -(2.f * (q[2]*q[3] - q[1]*q[0]));
	result[2][0] = -(2.f * (q[1]*q[3] - q[2]*q[0]));
	result[2][1] = -(2.f * (q[2]*q[3] + q[1]*q[0]));
	result[2][2] = -(1.f - 2*q[1]*q[1] - 2*q[2]*q[2]);
	return result;
}

double Util::dotVV(vector<double> &V1, vector<double> &V2){
	double result = 0;
	for(int i = 0; i < V1.size(); i++){
		result += V1[i]*V2[i];
	}
	return result;
}

double Util::norm(vector<double> &V){
	double result = 0;
	for(int i = 0; i < V.size(); i++){
		result += V[i]*V[i];
	}
	return sqrt(result);
}