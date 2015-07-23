// Find the points where the divergence of a vector field is a local manima
// --- Input: 1. normalized 3D vector field
//
//               dFx     dFy     dFz
// divergence = ----- + ----- + -----
//               dx      dy      dz
//
// --- Output: highest ...% divergence point list
// --- Author: Nicu D. Cornea, Vizlab, Rutgers University
// --- Date: Wed Aug 20 17:53:56 EDT 2003
//

#include "localMinDiv.h"


#define TRACE

#define SEARCH_GRID		1
#define CELL_SIZE		1.00 / SEARCH_GRID
#define vdist                   CELL_SIZE / 2.0

#define MAX_NUM_HDPTS	50000

/*
typedef struct {
	int* Points;
	int numPoints;
} HDGroup;

inline bool PointIsCloseToGroup(int pt, int grp, HDGroup *Groups, 
VoxelPositionDouble **HDPts);
*/

inline double GetDiv(double x, double y, double z, 
	      int L, int M, int N, Vector* ForceField);


bool GetLocalMinDivPoints(
	Vector* ForceField, 	      // [in] vector field
	int L, int M, int N,	      // [in] size of vector field (X, Y and Z)
	unsigned char *flags,	      // [in] flags array
	float perc,		      // [in] percentage of high div. points 
	                              //         to be returned (top <perc> %)
	VoxelPositionDouble **HDPts,  // [out] high divergence point list
	int *numHDPts,		      // [out] number of points in the list
	bool inOut                    // [in] flag specifying if we should look
	                              //    outside the object too (if true).
	                              // DEFAULT: false
) {

#ifdef TRACE
  printf("TRACE: Starting GetLocalMinDivPoints function. Cellsize = %lf\n", CELL_SIZE);
#endif

  (*HDPts) = NULL;
  (*numHDPts) = 0;

  if(perc == 0) {
    return true;
  }
  
  long idx, slsz;
  int i,j,k, ii, jj, kk, s;
  double x, y, z;
  long cntz, cntnz;
  
  slsz = L*M;		// slice size
  double adiv[MAX_NUM_HDPTS];	// divergence array
  
  long nb[26];
  // 26 neighbors
  nb[0] = -1;
  nb[1] = -1-L;
  nb[2] = -1+L;
  nb[3] = -1-slsz;
  nb[4] = -1-L-slsz;
  nb[5] = -1+L-slsz;
  nb[6] = -1+slsz;
  nb[7] = -1-L+slsz;
  nb[8] = -1+L+slsz;
  nb[9] = -L;
  nb[10] = +L;
  nb[11] = -slsz;
  nb[12] = -L-slsz;
  nb[13] = +L-slsz;
  nb[14] = +slsz;
  nb[15] = -L+slsz;
  nb[16] = +L+slsz;
  nb[17] = +1;
  nb[18] = +1-L;
  nb[19] = +1+L;
  nb[20] = +1-slsz;
  nb[21] = +1-L-slsz;
  nb[22] = +1+L-slsz;
  nb[23] = +1+slsz;
  nb[24] = +1-L+slsz;
  nb[25] = +1+L+slsz;

  
    
  if(((*HDPts) = new VoxelPositionDouble[MAX_NUM_HDPTS]) == NULL) {
    printf("GetHighDivergencePoints: OOPS! - Error allocating memory for the output array. Abort.\n");
    exit(1);
  }

  
  // calculate divergence throughout the dataset
  double maxDiv = -999999.99;
  double minDiv =  999999.99;
  double div;
  
  cntz = 0;
  cntnz = 0;
  double zerodiv = 0.1;
  double ns[26];
  bool localmin = true;
  long cntlm = 0;
  bool skipit = false;

  /////////////////////////////////////

#ifdef TRACE
  printf("vdist = %lf\n", vdist);
#endif	
  
  printf("Finding high divergence points (1).\n");
  for (k = 0; k < N; k++) {
    printf("\tProcessing plane %d out of %d\r", k, N-2);
    fflush(stdout);
    for (j = 0; j < M; j++) {
      for (i = 0; i < L; i++) {
	
	idx = k*slsz + j*L +i;
	
	// if we are not looking outside the object too.
	if(!inOut) {
	  // - if this point is EXTERIOR, BOUNDARY or SURF, skip it
	  if(	(flags[idx] == EXTERIOR) ||
		(flags[idx] == BOUNDARY) ||
		(flags[idx] == SURF))
	  {
	    continue;
	  }

	  // if one of it's neighbors is EXTERIOR, BOUNDARY or SURF, skip it
	  skipit = false;
	  for(s=0; s < 26; s++) {
	    if( (((idx+nb[s]) >= 0) && ((idx+nb[s]) < (L*M*N))) &&
		((flags[idx+nb[s]] == EXTERIOR) ||
		 (flags[idx+nb[s]] == BOUNDARY) ||
		 (flags[idx+nb[s]] == SURF))
		) 
	    {
	      skipit = true;
	      break;
	    }
	  }
	  if(skipit) {
	    continue;
	  }

	}
	else {
	  // we look for high divergence points outside the object too
	  // ignore only boundary points.
	  if( (flags[idx] == BOUNDARY) ||
	      (flags[idx] == SURF))
	  {
	    continue;
	  }
	}

	for(kk=0; kk < SEARCH_GRID; kk++) {
	  for(jj=0; jj < SEARCH_GRID; jj++) {
	    for(ii=0; ii < SEARCH_GRID; ii++) {
	      x = i + (ii * CELL_SIZE);
	      y = j + (jj * CELL_SIZE);
	      z = k + (kk * CELL_SIZE);
#ifdef TRACE
	      //							printf("At point: (%lf, %lf, %lf)\n", x, y, z);
#endif		

	      div = GetDiv(x, y, z, L, M, N, ForceField);
	      //
	      // neighbors
	      //
	      ns[0] = GetDiv(x-CELL_SIZE, y, z, L, M, N, ForceField);
	      ns[1] = GetDiv(x-CELL_SIZE, y-CELL_SIZE, z, L, M, N, ForceField);
	      ns[2] = GetDiv(x-CELL_SIZE, y+CELL_SIZE, z, L, M, N, ForceField);
	      ns[3] = GetDiv(x-CELL_SIZE, y, z-CELL_SIZE, L, M, N, ForceField);
	      ns[4] = GetDiv(x-CELL_SIZE, y-CELL_SIZE, z-CELL_SIZE, L, M, N, ForceField);
	      ns[5] = GetDiv(x-CELL_SIZE, y+CELL_SIZE, z-CELL_SIZE, L, M, N, ForceField);
	      ns[6] = GetDiv(x-CELL_SIZE, y, z+CELL_SIZE, L, M, N, ForceField);
	      ns[7] = GetDiv(x-CELL_SIZE, y-CELL_SIZE, z+CELL_SIZE, L, M, N, ForceField);
	      ns[8] = GetDiv(x-CELL_SIZE, y+CELL_SIZE, z+CELL_SIZE, L, M, N, ForceField);

	      ns[9] = GetDiv(x, y-CELL_SIZE, z, L, M, N, ForceField);
	      ns[10] = GetDiv(x, y+CELL_SIZE, z, L, M, N, ForceField);
	      ns[11] = GetDiv(x, y, z-CELL_SIZE, L, M, N, ForceField);
	      ns[12] = GetDiv(x, y-CELL_SIZE, z-CELL_SIZE, L, M, N, ForceField);
	      ns[13] = GetDiv(x, y+CELL_SIZE, z-CELL_SIZE, L, M, N, ForceField);
	      ns[14] = GetDiv(x, y, z+CELL_SIZE, L, M, N, ForceField);
	      ns[15] = GetDiv(x, y-CELL_SIZE, z+CELL_SIZE, L, M, N, ForceField);
	      ns[16] = GetDiv(x, y+CELL_SIZE, z+CELL_SIZE, L, M, N, ForceField);

	      ns[17] = GetDiv(x+CELL_SIZE, y, z, L, M, N, ForceField);
	      ns[18] = GetDiv(x+CELL_SIZE, y-CELL_SIZE, z, L, M, N, ForceField);
	      ns[19] = GetDiv(x+CELL_SIZE, y+CELL_SIZE, z, L, M, N, ForceField);
	      ns[20] = GetDiv(x+CELL_SIZE, y, z-CELL_SIZE, L, M, N, ForceField);
	      ns[21] = GetDiv(x+CELL_SIZE, y-CELL_SIZE, z-CELL_SIZE, L, M, N, ForceField);
	      ns[22] = GetDiv(x+CELL_SIZE, y+CELL_SIZE, z-CELL_SIZE, L, M, N, ForceField);
	      ns[23] = GetDiv(x+CELL_SIZE, y, z+CELL_SIZE, L, M, N, ForceField);
	      ns[24] = GetDiv(x+CELL_SIZE, y-CELL_SIZE, z+CELL_SIZE, L, M, N, ForceField);
	      ns[25] = GetDiv(x+CELL_SIZE, y+CELL_SIZE, z+CELL_SIZE, L, M, N, ForceField);


	      //
	      // check if local minimum
	      //
	      localmin = true;
	      for(s = 0; s < 26; s++) {
		if(div >= ns[s]) {
		  localmin = false;
		  break;
		}
	      }
	      if(localmin) {
		cntlm++;

		// add the point to the HD list
		(*HDPts)[(*numHDPts)].x = x;
		(*HDPts)[(*numHDPts)].y = y;
		(*HDPts)[(*numHDPts)].z = z;
		
		adiv[(*numHDPts)] = div;
		
		(*numHDPts) = (*numHDPts) + 1;
		if((*numHDPts) >= MAX_NUM_HDPTS) {
		  printf("OOPS! Too many high divergence points detected. \
										Reached maximum of %d. Abort\n", MAX_NUM_HDPTS);
		  exit(1);
		}
		/*
		printf("x, y, z: %lf, %lf, %lf. Div = %lf\nNeighbors:\n", x, y, z, div);
		for(s=0; s < 26; s++) {
		  printf(" %lf", ns[s]);
		}
		return false;
		*/
	      }

	      if((div > -zerodiv) && (div < zerodiv)) {
		cntz++;
	      }
	      else {
		cntnz++;
	      }
	      
#ifdef TRACE
	      /*
		printf("Forces:\n");
		for(s = 0; s < 6; s++) {
		printf("%lf  %lf  %lf\n", v[s].xd, v[s].yd, v[s].zd);
		}
		printf("Div = %lf\n", div);
	      */
#endif							
	      
	      if(div > maxDiv) {
		maxDiv = div;
	      }
	      if(div < minDiv) {
		minDiv = div;
	      }div;
	    }
	  }
	}
      }
    }
  }

#ifdef _DEBUG
  printf("Divergence: max = %lf, min = %lf\n", maxDiv, minDiv);
  printf("Number of divergence local maximum points: %ld\n", cntlm);
#endif

#ifdef TRACE
  printf("Local minima: \n");
  for(i=0; i < (*numHDPts); i++) {
    printf("%f %f %f - %f\n", (*HDPts)[i].x, (*HDPts)[i].y, (*HDPts)[i].z, adiv[i]);
  }
#endif

  // sort the points by divergence and return only the first ... %

  // the number of points to be returned
  int nperc = (int) ((*numHDPts) * perc / 100.00);
  if(perc == 0.00) {
    nperc = 0;
  }

  
#ifdef TRACE
  printf("Returning the first %d points (%lf%)\n", nperc, perc);
#endif  
  //
  // sort the points on the divergence value;
  //  
  double minval, tmp;
  int minpos;
  
  for(i=0; i < (*numHDPts); i++) {
    minval = adiv[i];
    minpos = i;
    for(j=i+1; j < (*numHDPts); j++) {
      if(adiv[j] < minval) {
	minval = adiv[j];
	minpos = j;
      }
    }
    if(minpos != i) {
      // exchange points and div values
      tmp = adiv[i];
      adiv[i] = adiv[minpos];
      adiv[minpos] = tmp;
      
      tmp = (*HDPts)[i].x; 
      (*HDPts)[i].x = (*HDPts)[minpos].x; 
      (*HDPts)[minpos].x = tmp;
      
      tmp = (*HDPts)[i].y; 
      (*HDPts)[i].y = (*HDPts)[minpos].y; 
      (*HDPts)[minpos].y = tmp;
      
      tmp = (*HDPts)[i].z; 
      (*HDPts)[i].z = (*HDPts)[minpos].z; 
      (*HDPts)[minpos].z = tmp;
    }

    if(i >= nperc) {
      break;
    }
  }

#ifdef TRACE
  printf("Sorted points (only the first %d of them): \n", nperc);
  for(i=0; i < (*numHDPts); i++) {
    printf("%f %f %f - %f\n", (*HDPts)[i].x, (*HDPts)[i].y, (*HDPts)[i].z, adiv[i]);
  }
#endif
  //  
  // no clustering of the points
  //
  
  //
  // Return only the first ... points  high divergence points
  //
  
  VoxelPositionDouble* newHDPts;
  
  if((newHDPts = new VoxelPositionDouble[nperc]) == NULL) {
    printf("GetHighDivergencePoints: OOPS! - Error allocating memory for the output array. Abort.\n");
    exit(1);
  }
  
  for(i=0; i < nperc; i++) {
    newHDPts[i].x = (*HDPts)[i].x;
    newHDPts[i].y = (*HDPts)[i].y;
    newHDPts[i].z = (*HDPts)[i].z;
  }
  
  // delete the old array
  delete [] (*HDPts);
  
  // return the new array
  (*HDPts) = newHDPts;
  (*numHDPts) = nperc;
  
#ifdef TRACE
  printf("Returning points: \n");
  for(i=0; i < (*numHDPts); i++) {
    printf("%f %f %f - %f\n", (*HDPts)[i].x, (*HDPts)[i].y, (*HDPts)[i].z, adiv[i]);
  }
#endif
  
  return true;
}


inline double GetDiv(double x, double y, double z, 
	      int L, int M, int N, Vector* ForceField) {
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
  v[0] = interpolation(x + vdist, y, z, L, M, N, ForceField);
  v[1] = interpolation(x - vdist, y, z, L, M, N, ForceField);
  v[2] = interpolation(x, y + vdist, z, L, M, N, ForceField);
  v[3] = interpolation(x, y - vdist, z, L, M, N, ForceField);
  v[4] = interpolation(x, y, z + vdist, L, M, N, ForceField);
  v[5] = interpolation(x, y, z - vdist, L, M, N, ForceField);
  //
  // compute divergence
  //      
  div = 
    ((v[0].xd - v[1].xd) + (v[2].yd - v[3].yd) + (v[4].zd - v[5].zd)
    ) / (2 * vdist);
	      
#ifdef TRACE
  /*
    printf("Forces:\n");
    for(s = 0; s < 6; s++) {
    printf("%lf  %lf  %lf\n", v[s].xd, v[s].yd, v[s].zd);
    }
    printf("Div = %lf\n", div);
  */
#endif		
  return div;

}
