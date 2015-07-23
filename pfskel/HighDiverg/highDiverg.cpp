// Interface for the high divergence module
// This function serves only as a non-changing interface to the high divergence
//    module that has it's implementation in another file
// It just calls the right implementation and returns

#include "highDiverg.h"
#include "localMinDiv.h"
#include "allDiv.h"

// #define SEARCH_GRID		1
// #define CELL_SIZE		1.00 / SEARCH_GRID

bool GetHighDivergencePoints(
	ForceVector *ForceField, 	      // [in] vector field
	int L, int M, int N,	      // [in] size of vector field (X, Y and Z)
	unsigned char *flags,	      // [in] flags array
	float perc,		      // [in] percentage of high div. points 
	                              //         to be returned (top <perc> %)
	DynamicArray<DivergencePoint>& HDPts,  // [out] high divergence 
	                              //      point list
	bool inOut,                   // [in] flag specifying if we should look
	                              //    outside the object too (if true).
	                              // DEFAULT: false
	HDSelection hdSel             // [in] specifies how the high divergence
	                              //    points are selected:
	                              //    from all points (HDS_All) or only
	                              //    from local minima (HDS_LocMin)
	                              // DEFAULT: HDS_LocMin 
) {
  double *divergence = NULL;
  bool retval = false;

  // allocate the divergence array
  //
  if((divergence = new double[L*M*N]) == NULL) {
    printf("GetHighDivergencePoints: Out of memory !\n");
    return false;
  }
  memset(divergence, 0, sizeof(double)*L*M*N);
  
  // compute divergence for all voxels
  if(ComputeDivergence(ForceField, L, M, N, flags, inOut, divergence)) {
    //
    // select seeds based on divergence
    switch(hdSel) {
    case HDS_LocMin:
      retval = 
	GetLocalMinDivPoints( divergence, L, M, N, flags, perc, 
			      HDPts, inOut);
      break;
    case HDS_All:
      retval = 
	GetAllDivPoints( divergence, L, M, N, flags, perc, 
			 HDPts, inOut);
      break;
    }
  }

  // free divergence array
  delete [] divergence;

  return retval;
}



///////////////////////////////////////////////////////////////////////////////
// computes the divergence for every voxel.
///////////////////////////////////////////////////////////////////////////////
bool ComputeDivergence(ForceVector *ForceField, // [in] vector field
		       int L, int M, int N,     // [in] size of vector field 
		                                //      (X, Y and Z)
		       unsigned char *flags,	// [in] flags array
		       bool inOut,              // [in] in/out flag
		       double *div             // [out] divergence
		       )
{
  ForceVector v[6];
  double vdist = 0.5;  // !! if you increase this above 1.0, make sure to 
                       // adjust the start and end points of the loops below
  int i, j, k;
  int idx;
  int slsz = L*M;

#ifdef TRACE
  printf("vdist = %lf\n", vdist);
#endif	

#ifdef _DEBUG  
  printf("Computing divergence...");
  fflush(stdout);
#endif

  for (k = 1; k < N-1; k++) {
    printf("\tProcessing plane %d out of %d\r", k, N-2);
    fflush(stdout);
    for (j = 1; j < M-1; j++) {
      for (i = 1; i < L-1; i++) {
	
	idx = k*slsz + j*L +i;
	
	// ignore boundary points.
	if( (flags[idx] == BOUNDARY) ||
	    (flags[idx] == SURF))
	{
	  continue;
	}

	// if we are not looking outside the object too, ignore EXTERIOR points
	if((!inOut) && (flags[idx] == EXTERIOR)) {
	  continue;
	}

	//
	// compute divergence
	//
	// interpolate force vectors arround the point  
	//
	v[0] = interpolation(i + vdist, j, k, L, M, N, ForceField, flags);
	v[1] = interpolation(i - vdist, j, k, L, M, N, ForceField, flags);
	v[2] = interpolation(i, j + vdist, k, L, M, N, ForceField, flags);
	v[3] = interpolation(i, j - vdist, k, L, M, N, ForceField, flags);
	v[4] = interpolation(i, j, k + vdist, L, M, N, ForceField, flags);
	v[5] = interpolation(i, j, k - vdist, L, M, N, ForceField, flags);

	div[idx] = 
	  ((v[0].xd - v[1].xd) + (v[2].yd - v[3].yd) + (v[4].zd - v[5].zd)) / 
	  (2 * vdist);	
      }
    }
  }

  return true;
}

///////////////////////////////////////////////////////////////////////////////
// Save HD points to file
///////////////////////////////////////////////////////////////////////////////
bool SaveHDPoints(char *fileName, DynamicArray<DivergencePoint>& HDPts) {
  FILE *fout;
  int i;
  if ((fout = fopen(fileName,"w")) == NULL) {
    printf("SaveHDPoints: Cannot open %s for writing\n", fileName);
    return false;
  }
  
  for(i=0; i < HDPts.GetNrElem(); i++) {
    // printf("writing point (%f, %f, %f) to output file.\n", 
    //	HDPts[i].x, HDPts[i].y, HDPts[i].z);
    fprintf(fout,"%lf %lf %lf %lf\n", 
	    HDPts[i].position.x, HDPts[i].position.y, HDPts[i].position.z,
	    HDPts[i].div);
  }
  fclose(fout);
  return true;
}

///////////////////////////////////////////////////////////////////////////////
// Read HD points from file
///////////////////////////////////////////////////////////////////////////////
bool ReadHDPoints(char *fileName, DynamicArray<DivergencePoint>& HDPts) {
  FILE *fin;
  int read;
  DivergencePoint pt;
  //  int unused;
  
  if ((fin = fopen(fileName, "r")) == NULL)  {
    printf("ReadHDPoints: Error: couldn't open file: %s\n", fileName);
    return false;
  }
  
  while(!feof(fin)) {
    read =  fscanf(fin, "%lf %lf %lf %lf\n", 
		   &(pt.position.x), &(pt.position.y), &(pt.position.z), 
		   &(pt.div));
    if(read!= 4) {
      if(read != EOF) {
	printf("ReadHDPoints: Error reading input file.\n");
	return false;
      }
      else {
	// printf("Reached end of file.\n");
	break;
      }
    }
    HDPts.Append(pt);
  }
  
  fclose(fin);
  return true;
}




/*
// compute divergence for a position
inline double GetDiv(double x, double y, double z, 
		     int L, int M, int N, ForceVector *ForceField, 
		     unsigned char *flags) {
  ForceVector v[6];
  double div = 0;

  //
  // check the coordinates
  //
  if( (x < (0.00 - vdist)) || (x > (L-1-vdist)) ||
      (y < (0.00 - vdist)) || (y > (M-1-vdist)) ||
      (z < (0.00 - vdist)) || (z > (N-1-vdist)))
  {
    return div;
  }
  //
  // interpolate force vectors arround the point  
  //
  v[0] = interpolation(x + vdist, y, z, L, M, N, ForceField, flags);
  v[1] = interpolation(x - vdist, y, z, L, M, N, ForceField, flags);
  v[2] = interpolation(x, y + vdist, z, L, M, N, ForceField, flags);
  v[3] = interpolation(x, y - vdist, z, L, M, N, ForceField, flags);
  v[4] = interpolation(x, y, z + vdist, L, M, N, ForceField, flags);
  v[5] = interpolation(x, y, z - vdist, L, M, N, ForceField, flags);
  //
  // compute divergence
  //      
  div = 
    ((v[0].xd - v[1].xd) + (v[2].yd - v[3].yd) + (v[4].zd - v[5].zd)
    ) / (2 * vdist);
	      
#ifdef TRACE
  //printf("Forces:\n");
  //for(s = 0; s < 6; s++) {
  //printf("%lf  %lf  %lf\n", v[s].xd, v[s].yd, v[s].zd);
  //}
  //printf("Div = %lf\n", div);
#endif		

  return div;
}
*/
