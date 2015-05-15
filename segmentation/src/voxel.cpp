#include "voxel.hpp"

enum {x, y, z};

/******************************************************************************
 DESCRIPTION:
	Create a grid of [N] voxels in the bounding box.

 ARGUMENT:
 	box: the bounding box limits. [xmin xmax; ymin, ymax; zmin, zmax]
 	N: number of voxels you want to create.

 fchicken @ April,25 2015
*******************************************************************************/
void Voxel::makevoxels(vector<vector<double> > &box, int N){
	cerr << "INFO/VOXEL.MAKEVOXELS: " << endl;
	
	bbox = box;
	double volume = (bbox[x][1]-bbox[x][0])
				   *(bbox[y][1]-bbox[y][0])
				   *(bbox[z][1]-bbox[z][0]);
	resolution = pow(volume/N, 1.f/3);
	for(double xx = bbox[x][0]; xx <= bbox[x][1]; xx += resolution){
		for(double yy = bbox[y][0]; yy <= bbox[y][1]; yy += resolution){
			for(double zz = bbox[z][0]; zz <= bbox[z][1]; zz += resolution){
				double v[] = {xx, yy, zz, p.size()};
				vector<double> voxel(v, v + sizeof(v)/sizeof(double));
				p.push_back(voxel);
				value.push_back(1);
			}
		}
	}
	cerr << "	resolution = " << resolution << endl;
	cerr << "	number of voxels = " << p.size() << endl;
	return;
}

/******************************************************************************
 DESCRIPTION:
	Get the isosurface with the value of [isovalue], which is representing by
	tiangles.

 ARGUMENT:
 	isovalue: the value where the surface should lie on.

 fchicken @ April,29 2015
*******************************************************************************/
void Voxel::isosurface(double isovalue){
	cerr << "INFO/VOXEL.ISOSURFACE: " << endl;
	
	// clean the triangles first
	triangles.clear();
	
	// record the number of points in each direction
	int nx, ny, nz;

	// re-voxelize current data
	vector< vector<double> > _p;
	nx = 0;
	for(double xx = bbox[x][0]; xx <= bbox[x][1]; xx += resolution){
		ny = 0;
		for(double yy = bbox[y][0]; yy <= bbox[y][1]; yy += resolution){
			nz = 0;
			for(double zz = bbox[z][0]; zz <= bbox[z][1]; zz += resolution){
				double v[] = {xx, yy, zz};
				vector<double> voxel(v, v + sizeof(v)/sizeof(double));
				_p.push_back(voxel);
				nz++;
			}
			ny++;
		}
		nx++;
	}

	// process every grid to get triangle mesh
	for(int gx = 0; gx < nx-1; gx++){
		for(int gy = 0; gy < ny-1; gy++){
			for(int gz = 0; gz < nz-1; gz++){
				int index[8] = {  gz + gy*nz + gx*ny*nz 			//(0, 0, 0)
				 				, gz + gy*nz + (gx+1)*ny*nz 		//(1, 0, 0)
								, gz + (gy+1)*nz + (gx+1)*ny*nz 	//(1, 1, 0)
								, gz + (gy+1)*nz + gx*ny*nz 		//(0, 1, 0)
								, (gz+1) + gy*nz + gx*ny*nz 		//(0, 0, 1)
								, (gz+1) + gy*nz + (gx+1)*ny*nz 	//(1, 0, 1)
								, (gz+1) + (gy+1)*nz + (gx+1)*ny*nz //(1, 1, 1)
								, (gz+1) + (gy+1)*nz + gx*ny*nz}; 	//(0, 1, 1)

				GridCell grid;
				for(int i = 0; i < 8; i++){
					grid.p[i] = _p[ index[i] ];
					grid.value[i] = value[ index[i] ];
				}	

				// append new triangles to the end
				vector<Triangle> tmp_triangles = MarchingCube::polygonise(grid, 0.5);
				triangles.insert(triangles.end(), tmp_triangles.begin(), tmp_triangles.end());
			}
		}
	}

	cerr << "	" << triangles.size() << " triangles created." << endl;
	return;
}

/******************************************************************************
 DESCRIPTION:
	Return current bbox according to remaining voxels.


 fchicken @ May,8 2015
*******************************************************************************/
vector<vector<double> > Voxel::currentbbox(){
	vector<vector<double> > box(3, vector<double>(2));
	box[x][0] = box[y][0] = box[z][0] = 100000;
	box[x][1] = box[y][1] = box[z][1] = -100000;
	for(int i = 0; i < p.size(); i++){
		if(p[i][x] < box[x][0]) box[x][0] = p[i][x];
		if(p[i][x] > box[x][1]) box[x][1] = p[i][x];
		if(p[i][y] < box[y][0]) box[y][0] = p[i][y];
		if(p[i][y] > box[y][1]) box[y][1] = p[i][y];
		if(p[i][z] < box[z][0]) box[z][0] = p[i][z];
		if(p[i][z] > box[z][1]) box[z][1] = p[i][z];
	}
	for(int i = 0; i < 3; i++){
		box[i][0] = box[i][0] - 2*resolution;
		box[i][1] = box[i][1] + 2*resolution;
	}
	return box;
}



