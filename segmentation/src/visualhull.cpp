#include "visualhull.hpp"

enum {x, y, z};

/******************************************************************************
 DESCRIPTION:
	Determines the bounding box (x, y and z limits) of the model 
	which is to be carved. This allows the initial voxel volume 
	to be constructed.

 ARGUMENT:
	cameras: camera definitions structure array. 
        	 (containg image, silhouette, focal length, rotation 
        	 matrix and translation)

 OUTPUT:
 	bbox: the bonding limits of 3-axis. [xmin, xmax; ymin, ymax; zmin, zmax]


 fchicken @ April,25 2015
*******************************************************************************/
vector<vector<double> > VisualHull::findbbox(vector<Camera> &cameras){
	
	// Get initial bounding box
	vector<vector<double> > bbox(3, vector<double>(2));
	bbox[x][0] = bbox[y][0] = bbox[z][0] = 100000;
	bbox[x][1] = bbox[y][1] = bbox[z][1] = -100000;
	for(int i = 0; i < cameras.size(); i++){
		if(cameras[i].p[x] < bbox[x][0]) bbox[x][0] = cameras[i].p[x];
		if(cameras[i].p[x] > bbox[x][1]) bbox[x][1] = cameras[i].p[x];
		if(cameras[i].p[y] < bbox[y][0]) bbox[y][0] = cameras[i].p[y];
		if(cameras[i].p[y] > bbox[y][1]) bbox[y][1] = cameras[i].p[y];
		if(cameras[i].p[z] < bbox[z][0]) bbox[z][0] = cameras[i].p[z];
		if(cameras[i].p[z] > bbox[z][1]) bbox[z][1] = cameras[i].p[z];
	}

	// Use viewing direction correct 3-axis limits
	double maxdiff = 0;
	for(int i = 0; i < 3; i++){
		if(bbox[i][1]-bbox[i][0] > maxdiff) maxdiff = bbox[i][1]-bbox[i][0];
	}
	double length = sqrt(2*maxdiff*maxdiff);
	for(int i = 0; i < cameras.size(); i++){
		vector<double> viewV = Util::multiplyVk(cameras[i].d, length);
		vector<double> viewP = Util::addVV(cameras[i].p, viewV);
		if(viewP[x] < bbox[x][0]) bbox[x][0] = viewP[x];
		if(viewP[x] > bbox[x][1]) bbox[x][1] = viewP[x];
		if(viewP[y] < bbox[y][0]) bbox[y][0] = viewP[y];
		if(viewP[y] > bbox[y][1]) bbox[y][1] = viewP[y];
		if(viewP[z] < bbox[z][0]) bbox[z][0] = viewP[z];
		if(viewP[z] > bbox[z][1]) bbox[z][1] = viewP[z];				
	}

	// Perform a rough space-carving to narrow down the bounding box
	Voxel voxels;
	voxels.makevoxels(bbox, 4000);
	for(int i = 0; i < cameras.size(); i++){
		carve(cameras[i], voxels);
	}

	// Make sure something is left!
	if(voxels.p.size() <= 0){
		cerr << "ERROR/FINDBBOX: nothing left after initial search." << endl;
		exit(EXIT_FAILURE);
	}

	// Update the limits after first space-carving
	bbox = voxels.currentbbox();

	return bbox;
}

/******************************************************************************
 DESCRIPTION:
	Project 3D points from world coordinate to image coordinate.

 ARGUMENT:
	camera: the camera used to project on.
	voxels: the voxels want to be project.

 OUTPUT:
	projection: the projection points in image coordinate. [x, y]


 fchicken @ April,27  2015
*******************************************************************************/
vector<vector<double> > VisualHull::project(Camera &camera, Voxel &voxels){
	vector<vector<double> > projection;
	for(int i = 0; i < voxels.p.size(); i++){
		vector<double> p_world = voxels.p[i];
		
		vector<double> p_camera = Util::multiplyMV(camera.R, p_world);
		p_camera = Util::addVV(p_camera, camera.t);
		
		vector<double> p_pixel = Util::multiplyVk(p_camera, -1.f/p_camera[z]);
		p_pixel = Util::multiplyVk(p_pixel, camera.f);

		double centerx = (camera.image.cols-1)/2;
		double centery = (camera.image.rows-1)/2;
		vector<double> p_image;
		p_image.push_back( round( p_pixel[x]+centerx ) );
		p_image.push_back( round( (camera.image.rows-1)-(p_pixel[y]+centery) ) );
		projection.push_back(p_image);
	}
	return projection;
}

template<typename T>
void erase(std::vector<T>& v, int inx)
{
	std::swap(v[inx], v.back());
	v.pop_back();
}
/******************************************************************************
 DESCRIPTION:
	Carve away [voxels] that are not inside the silhouette contained in
	[camera].

 ARGUMENT:
	camera: the camera used to do carving.
	voxels: the array of voxels coordinate.


 fchicken @ April,27 2015
*******************************************************************************/
void VisualHull::carve(Camera &camera, Voxel &voxels){
	cerr << "INFO/VISUALHULL.CARVE: " << endl;

	cerr << "	projecting points ... ";
	// Project all voxels into image
	vector<vector<double> > projection = project(camera, voxels);
	cerr << "done." << endl;

	for(int i = projection.size()-1; i >= 0; i--){
		// Clear voxels out of image
		if(projection[i][x] < 0 || projection[i][x] >= camera.image.cols 
			|| projection[i][y] < 0 || projection[i][y] >= camera.image.rows){
			voxels.value[voxels.p[i][3]] = 0;
			erase(voxels.p, i);
		}
		// Clear voxels out of silhouette
		else if(camera.silhouette(projection[i][y], projection[i][x]) != 255){
			voxels.value[voxels.p[i][3]] = 0;
			erase(voxels.p, i);			
		}
	}

	cerr << "	voxels remain number = " << voxels.p.size() << endl;
	return;
}
