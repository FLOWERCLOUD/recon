#include "slic.hpp"

/******************************************************************************
 DESCRIPTION:
	Use slic super-pixel segmentation algorithm to segment [image]. 

 ARGUMENT:
	cameras: cameras in the scene.
	voxels: voxels in the scene.


 fchicken @ April,27 2015
*******************************************************************************/
void Slic::segment(Mat_<Vec3b> &image, int N){
	cerr << "INFO/SLIC.SEGMENT: " << endl;
	segmentnum = N;
	Mat_<Vec3b> imagelab(image.rows, image.cols);
	cvtColor(image, imagelab, CV_RGB2Lab);

	// Convert image to one-dimensional array.
    float* imgarr = new float[image.rows*image.cols*3];
    for (int i = 0; i < image.rows; ++i) {
        for (int j = 0; j < image.cols; ++j) {
            imgarr[j + image.cols*i + image.cols*image.rows*0] = imagelab(i, j)[0];
            imgarr[j + image.cols*i + image.cols*image.rows*1] = imagelab(i, j)[1];
            imgarr[j + image.cols*i + image.cols*image.rows*2] = imagelab(i, j)[2];
        }
    }

    // The algorithm will store the final segmentation in a one-dimensional array.
    vl_uint32* segmentation = new vl_uint32[image.rows*image.cols];
    vl_size height = image.rows;
    vl_size width = image.cols;
    vl_size channels = image.channels();
    vl_size region = floor(sqrt(image.rows*image.cols/N));        
    float regularizer = (double)region;
    vl_size minregion = 10;
    vl_slic_segment(segmentation, imgarr, width, height, channels, region, regularizer, minregion);
    cerr << "	region = " << region << endl;
    cerr << "	regularization = " << regularizer << endl;

    // Convert segmentation
    labels = Mat::zeros(image.rows, image.cols, CV_32S);
    for(int i = 0; i < image.rows; i++){
        for(int j = 0; j < image.cols; j++){
        	labels(i, j) = (int)segmentation[ j+image.cols*i ];
        }
    }
    getspinfo(image);

    delete [] imgarr;
    delete [] segmentation;
    
    return;
}

/******************************************************************************
 DESCRIPTION:
	Calculate how many groups are there and the mean position and color of 
	each group.


 fchicken @ April,27 2015
*******************************************************************************/
void Slic::getspinfo(Mat_<Vec3b> &image){
	cerr << "INFO/SLIC.GETSPINFO: " << endl;

	// Count how many segments are there and what the labels are
	group.reserve(segmentnum);    
	for(int i = 0; i < image.rows; i++){
    	for(int j = 0; j < image.cols; j++){
    		for(int g = 0; g < group.size(); g++){
    			if(labels(i, j) == group[g]) break;
    			if(g == group.size()-1) group.push_back(labels(i, j));
    		}
    		if((int)group.size() == 0) group.push_back(labels(i, j));
    	}
    }
    cerr << "	number of groups = " << group.size() << endl;

	double psum[(int)group.size()][2];
	double csum[(int)group.size()][3];
	int number[(int)group.size()];

	// initialization
	for(int g = 0; g < group.size(); g++){
		psum[g][0] = 0.f;
		psum[g][1] = 0.f;
		csum[g][0] = 0.f;
		csum[g][1] = 0.f;
		csum[g][2] = 0.f;
		number[g] = 0;
	}

	// sum
	double centerx = (image.cols-1)/2;
	double centery = (image.rows-1)/2;
    for(int i = 0; i < image.rows; i++){
    	for(int j = 0; j < image.cols; j++){
    		// find which group this pixel is
    		int g;
    		for(g = 0; g < group.size(); g++){
    			if(labels(i, j) == group[g]) break;
    		}

    		psum[g][0] += (j-centerx); // x-axis
    		psum[g][1] += ((image.rows-1-i)-centery); // y-axis
    		csum[g][0] += image(i, j)[0];
    		csum[g][1] += image(i, j)[1];
    		csum[g][2] += image(i, j)[2];
    		number[g] += 1;
    	}
    }

    // average
    position.reserve(segmentnum*3);
    color.reserve(segmentnum*3);
    for(int g = 0; g < group.size(); g++){
		double pmean[3] = {psum[g][0]/number[g], 
						   psum[g][1]/number[g],
						   1.f};
		vector<double> p(pmean, pmean + sizeof(pmean)/sizeof(double));    
		position.push_back(p);

		double cmean[3] = {csum[g][0]/number[g],
						   csum[g][1]/number[g],
						   csum[g][2]/number[g]};
		vector<double> c(cmean, cmean + sizeof(cmean)/sizeof(double));
		color.push_back(c);
	}

	cerr << "	done." << endl;
	return;
}


/******************************************************************************
 DESCRIPTION:
	Draw a specific segment on [image] with [color].


 fchicken @ May,9 2015
*******************************************************************************/
void Slic::drawsegment(Mat_<Vec3b> &image, int target, vector<double> color){
	int targetlabel = group[target];
	for(int i = 0; i < image.rows; i++){
		for(int j = 0; j < image.cols; j++){
			if(labels(i, j) == targetlabel){
				image(i, j) = Vec3b(color[0], color[1], color[2]); 
			}
		}
	}
}

/******************************************************************************
 DESCRIPTION:
	Draw the segment boundary on [image].


 fchicken @ May,9 2015
*******************************************************************************/
void Slic::drawsegmentresult(Mat_<Vec3b> &image){
	// draw boundary
    int label, labeltop, labelbottom, labelleft, labelright;
    for (int i = 0; i < image.rows; i++){
        for (int j = 0; j < image.cols; j++){
            label = labels(i, j);
            labeltop = labelbottom = labelleft = labelright = label;
            if (i > 0) labeltop = labels(i-1, j);
            if (i < image.rows-1) labelbottom = labels(i+1, j);
            if (j > 0) labelleft = labels(i, j-1);
            if (j < image.cols-1) labelright = labels(i, j+1);
            if (label!=labeltop || label!=labelbottom || label!=labelleft || label!=labelright) {
                image(i, j)[0] = 0;
                image(i, j)[1] = 0;
                image(i, j)[2] = 255;
            }
        }
    }
}

/******************************************************************************
 DESCRIPTION:
	Draw the mean position and color of each group, and save as a file named
	[filename].


 fchicken @ April,27 2015
*******************************************************************************/
void Slic::drawspresult(Mat_<Vec3b> &image){
	// draw mean point and color
    double centerx = (image.cols-1)/2;
	double centery = (image.rows-1)/2;
    for(int i = 0; i < group.size(); i++){
    	int x = round(position[i][0]+centerx);
    	int y = round((image.rows-1)-(position[i][1]+centery));
    	int b = round(color[i][0]);
    	int g = round(color[i][1]);
    	int r = round(color[i][2]);

    	circle(image, Point(x, y), 4, Scalar(255, 0, 0), -1, 8);
    	circle(image, Point(x, y), 3, Scalar(b, g, r), -1, 8);
    }
}
/******************************************************************************
 DESCRIPTION:
	Write the labels, mean position and color of each group, and save as a file named
	[filename].


 fchicken @ April,27 2015
*******************************************************************************/
void Slic::writesegmentresult(string filename){
	FILE *fp = fopen(filename.c_str(), "a");
	if(fp == NULL){
		cerr << "ERROR/SLIC.WRITESEGMENTRESULT: can't open " + filename << endl;
		exit(EXIT_FAILURE);
	}
	fprintf(fp, "%d %d\n", labels.rows, labels.cols);
	for(int i = 0; i < labels.rows; i++){
		for(int j = 0; j < labels.cols; j++){
			fprintf(fp, "%d ", labels(i, j));
		}
		fprintf(fp, "\n");
	}
	fprintf(fp, "%d\n", (int)group.size());
	for(int i = 0; i < group.size(); i++){
		fprintf(fp, "%d ", group[i]);
		fprintf(fp, "%lf %lf %lf ", position[i][0], position[i][1], position[i][2]);
		fprintf(fp, "%lf %lf %lf\n", color[i][0], color[i][1], color[i][2]);
	}
	fclose(fp);
	return;
}


/******************************************************************************
 DESCRIPTION:
	Read the labels, mean position and color of each group from [fp].


 fchicken @ April,27 2015
*******************************************************************************/
void Slic::readsegmentresult(FILE *fp){
	int rows, cols;
	fscanf(fp, "%d%d", &rows, &cols);
	labels = Mat::zeros(rows, cols, CV_32S);
	for(int i = 0; i < labels.rows; i++){
		for(int j = 0; j < labels.cols; j++){
			fscanf(fp, "%d", &labels(i, j));
		}
	}
	int groupnum;
	fscanf(fp, "%d", &groupnum);
	for(int i = 0; i < groupnum; i++){
		int g;
		double pmean[3], cmean[3]; 
		
		fscanf(fp, "%d", &g);
		group.push_back(g);

		fscanf(fp, "%lf%lf%lf", &pmean[0], &pmean[1], &pmean[2]);
		vector<double> p(pmean, pmean + sizeof(pmean)/sizeof(double));    
		position.push_back(p);

		fscanf(fp, "%lf%lf%lf", &cmean[0], &cmean[1], &cmean[2]);
		vector<double> c(cmean, cmean + sizeof(cmean)/sizeof(double));    
		color.push_back(c);
	}
	return;
}


