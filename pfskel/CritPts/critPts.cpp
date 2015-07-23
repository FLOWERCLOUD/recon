// Find critical points of a vector field
// --- Input: 1. normalized 3D vector field
//
// --- Output: critical point list
// --- Author: Nicu D. Cornea, Vizlab, Rutgers University
// --- Date: Fri Aug  1 13:14:18 EDT 2003
//

#include "critPts.h"


// #define TRACE

//
// maximum number of iterations for the Newton method
//
#define MAX_NR_ITERATIONS  20     

//
// defined the number of divisions for each coordinate axis when splitting a 
// cell. Exmple: if 2, we split the cell in 8. 2 seems to be the best value
#define CELL_SUBDIVISION_FACTOR   2

// #define NR_OF_SIGN_CHANGES	3 - not used - assumed 3

#define DBG_CELL_X    133
#define DBG_CELL_Y    180
#define DBG_CELL_Z    95



// #define SIGN(nr)		(((nr) < 0.00) ? -1 : 1)
// #define SIGN(nr)		(((nr) < 0.00) ? -1 : (((nr) > 0.00) ? 1 : 0))
#define SIGN(nr)		(IS_ZERO(nr) ? 0 : ((nr) < 0.00 ? -1 : 1))



bool ConstructJacobian(TNT::Array2D<double> *Jac, 
		       VoxelPositionDouble *pos, double vdist, 
		       int L, int M, int N, ForceVector *ForceField,
		       unsigned char *flags,
		       bool *foundAZeroNeighbor, ForceVector *zeroPos);

bool FindCritPointNewton(double x, double y, double z, double cellsize,
			 int sX, int sY, int sZ, ForceVector *ForceField,
			 unsigned char *flags,
			 VoxelPositionDouble *critPt);

bool FindCriticalPointInFloatCell(double x, double y, double z, 
				  double cellsize,
				  int sX, int sY, int sZ, 
				  ForceVector *ForceField, 
				  unsigned char *flags,
				  int maxRecursionDepth, int recursionDepth,
				  bool useNewton, 
				  VoxelPositionDouble *critPt);

inline bool ChangeInSign(ForceVector *forces, int numForces);


bool GetCriticalPoints(
	ForceVector *ForceField,    // [in] vector field
	int L, int M, int N,        // [in] volume size
	unsigned char *flags,       // [in] volume flags
	DynamicArray<CriticalPoint> &CritPts, // [out] detected critical points
	//CriticalPoint **CritPts,  
	//int *numCritPts,          // [out] critical points count
	bool inOut /*= false*/,     // [in] flag specifying if we know the 
                                    //    inside of the object from the outside
                                    //    false - can distinguish the inside
	                            //    true - cannot distinguish the inside
	                            //    DEFAULF: false
	bool notCloseToSurf /*=false*/, // [in] do not search for critical 
	                            // points
	                            //    close to the surface (on the surface
                                    //    or 1 voxel away) 
	int maxRecursionDepth /*=20*/, // [in] specified maximum depth of 
	                            //    recursion when dividing a candidate 
	                            //    cell
	bool useNewton /*= true*/   // [in] specified whether Newton's method
	                            // should be used to localize the critical 
	                            // points more accurately
) {

#ifdef TRACE
  printf("TRACE: Starting GetCriticalPoints function... flags = %p\n", flags);
#endif

  // ForceVector *Normforce;
  
  long idx, slsz;
  int i,j,k, ii;
  int offset;
  
  VoxelPositionDouble critPt;
  CriticalPoint tmpCP;
  
  slsz = L*M;		// slice size
  
  // find critical points
  
  int inds[8];
  bool skipThisPoint = false;

#ifdef _DEBUG
  if(notCloseToSurf) {
    printf("\
** Critical points on the boundary \n\
      or a voxel away from the boundary are ignored **\n"); 
  }
  if(inOut) {
    printf("** Inside and Outside. **\n"); 
  }
  else {
    printf("** Inside ONLY. **\n"); 
  }
#endif

/////////////////////////////////////

  for (k = 1; k < N-1; k++) {
    printf("\tProcessing plane %d out of %d\r", k, N-1);
    fflush(stdout);
    
    for (j = 1; j < M-1; j++) {
      for (i = 1; i < L-1; i++) {
	
	idx = k*slsz + j*L +i;

	// these are the neighbors that will be touched by the interpolation
	inds[0] = idx;
	inds[1] = idx + 1;
	inds[2] = idx + L;
	inds[3] = idx + L + 1;
	inds[4] = idx + slsz;
	inds[5] = idx + slsz + 1;
	inds[6] = idx + slsz + L;
	inds[7] = idx + slsz + L + 1;

	skipThisPoint = false;

	// we should skip the cells that have a vertex outside the object
	// if we don't, we will end up finding critical points outside the 
	// object !!
	// This is a resolution problem - increase the resolution and we should
	// be able to identify all critical points
	//
	//
	// if we know the interior vs. exterior, ignore the outside
	//
	if(!inOut) {
	  for(ii=0; ii < 8; ii++) {
	    if(flags[inds[ii]] == EXTERIOR) {
	      skipThisPoint = true;
	      break;
	    }
	  }
	}
	

	/* Jul 20, 2006 - will not ignore critical points close to surface 
	//   anymore - the force following algorithm may get stuck if
	//   we simply ignore critical points
	// 
	// 
	// if notCloseToSurf is true, also ignore this cell if any of the 
	// neighbors is on the surface
	//
	
	// Dec 15 - let's ignore them anyway because they could be generated
	// by noise on the surface
	// and we don't want extra segments to pop up 
	// when noise is added to the object
	*/

	if(!skipThisPoint && notCloseToSurf) {
	  // if the current point or any of the neighbors touched by the 
	  // interpolation is SURF or BOUNDARY, skip 
	  for(ii=0; ii < 8; ii++) {
	    if((flags[inds[ii]] == SURF) || 
	       (flags[inds[ii]] == BOUNDARY)) 
	    {
	      skipThisPoint = true;
	      break;
	    }
	  }
	}
	

		  
	if(skipThisPoint)  {

#ifdef TRACE
	  if((i == DBG_CELL_X) && (j == DBG_CELL_Y) && (k == DBG_CELL_Z)) {
	    printf("\n** At (%d, %d, %d): SKIPPED.\n", 
		   DBG_CELL_X, DBG_CELL_Y, DBG_CELL_Z);
	  }
#endif

	  continue;
	}

	//
	// not skipped
	//
	
#ifdef TRACE	
	if(k == DBG_CELL_Z) {
	  printf("inspecting cell: %d %d %d.\r", i, j, k);
	  fflush(stdout);
	}
#endif

	  
	if(FindCriticalPointInIntCell(i, j, k, inds, L, M, N, 
				      ForceField, flags, inOut, 
				      maxRecursionDepth, useNewton, 
				      &critPt)) 
	  {
	  // add to array of critical points
	  tmpCP.position.x = critPt.x;
	  tmpCP.position.y = critPt.y;
	  tmpCP.position.z = critPt.z;
	  tmpCP.type = CPT_UNKNOWN;
	  	  
	  if(!CritPts.Append(tmpCP)) {
	    printf("GetCriticalPoints: insert critical point operation failed !\n");
	    return false;
	  }
	  
	  
#ifdef TRACE
	  if((i == DBG_CELL_X) && (j == DBG_CELL_Y) && (k == DBG_CELL_Z)) {
	    printf("\n** At (%d, %d, %d): found (%.16lf, %.16lf, %.16lf)\n", 
		   DBG_CELL_X, DBG_CELL_Y, DBG_CELL_Z,
		   critPt.x, critPt.y, critPt.z);
	  }
#endif
	  }
	else {
#ifdef TRACE
	  if((i == DBG_CELL_X) && (j == DBG_CELL_Y) && (k == DBG_CELL_Z)) {
	    printf("\n** At (%d, %d, %d): NOT found.\n", 
		   DBG_CELL_X, DBG_CELL_Y, DBG_CELL_Z);
	  }
#endif
	}
      }
    }
  }

#ifdef _DEBUG
  PrintElapsedTime("\n\tCP-2: Locating critical points.");
  //printf("\t** %d critical points reported.\n", (*numCritPts));
  printf("\t** %d critical points reported.\n", CritPts.GetNrElem());
  printf("Removing duplicates...\n");
#endif

  // go through the list of critical points and remove duplicates
  for(i=0; i < CritPts.GetNrElem(); i++) {
    for(j=i+1; j < CritPts.GetNrElem(); j++) {
      if((IS_ZERO(CritPts[i].position.x - CritPts[j].position.x)) &&
	 (IS_ZERO(CritPts[i].position.y - CritPts[j].position.y)) &&
	 (IS_ZERO(CritPts[i].position.z - CritPts[j].position.z)))
      {
	// found a duplicate
	/*
	  
	printf("duplicate found: (%d and %d)\n\
	(%.16lf, %.16lf, %.16lf)\n\
	(%.16lf, %.16lf, %.16lf)\n", 
	i, j, 
	(*CritPts)[i].position.x,
	(*CritPts)[i].position.y, 
	(*CritPts)[i].position.z,  
	(*CritPts)[j].position.x,
	(*CritPts)[j].position.y,
	(*CritPts)[j].position.z);
	
	*/
	
	CritPts[i].position.x = -1;
	CritPts[i].position.y = -1;
	CritPts[i].position.z = -1;
	
      }
    }
  }
  
  // compact the array
  
  i = 0;
  offset = 0;
  
  while((i+offset) < CritPts.GetNrElem()) {
    if(offset > 0) {
      CritPts[i].position.x = CritPts[i+offset].position.x;
      CritPts[i].position.y = CritPts[i+offset].position.y;
      CritPts[i].position.z = CritPts[i+offset].position.z;
    }
    
    if(CritPts[i].position.x == -1) {
      offset++;      
    }
    else {
      i++;
    }
  }
  // remove the last <offset> critical points
  for(i=0; i < offset; i++) {
    if(!CritPts.RemoveLastElem()) return false;
  }
  

#ifdef _DEBUG
  PrintElapsedTime("\n\tCP-3: Removing duplicate critical points.");
  printf("\t** Number of critical points is: %d\n", CritPts.GetNrElem());
#endif

#ifdef TRACE
  // print the critical points
  printf("Critical points found:\n");
  for(i = 0; i < CritPts.GetNrElem(); i++) {
    printf("%.11f %.11f %.11f\n", 
	   CritPts[i].position.x, 
	   CritPts[i].position.y, 
	   CritPts[i].position.z);
  }
#endif
  
  // clasify the critical points as: atracting/repelling nodes or saddles
  
  for(i=0; i < CritPts.GetNrElem(); i++) {
    if(!ClassifyCriticalPoint(CritPts[i], ForceField, flags, L, M, N)) {
      printf("WARNING: A critical point was not classified !\n");
    }
  }


#ifdef _DEBUG
  PrintElapsedTime("\tCP-4: Characterizing critical points.");
#endif

  printf("Returning %d critical points.\n", CritPts.GetNrElem());
  
  return true;
}




// Sign change notes:
//
// if one component is 0 in all vectors, it should be considered as a 
//   change in sign
//   or else, we might miss some critical points for which we have only
//   NR_OF_SIGN_CHANGES - 1 changes in sign and the last component is 0
//   all over the cell. Example (2D):
//     *->--<-*
//     *->--<-*
// In the above example, the arrows indicate the the vectors at the 4 
//    corners of a 2D cell. The X component changes sign, but the Y 
//    component is 0 in all 4 corners, and thus we might miss this 
//    critical point if we require exactly 2 sign changes. Considering
//    0 everywhere as a change in sign, will include this critical point.
//


inline bool ChangeInSign(ForceVector* forces, int numForces) {
  int i;
  // int count;
  unsigned char xpc, xnc, xzc, ypc, ync, yzc, zpc, znc, zzc;
  // SEE notes on sign change above
  //
  // change in sign for at least one of the vector components leads to a 
  //    sort of medial surface
  //
  // change in sign for at least NR_OF_SIGN_CHANGES vector components
  //
  // we are looking for at least one positive and at least one negative value
  // or, all 0's
  //
  if(numForces <= 0) return false;

  // count = 0;
  // check the components of these vectors.    

  // X
  xpc = 0; xnc = 0; xzc = 0;
  for(i = 0; i < numForces; i++) {
    switch(SIGN(forces[i].xd)) {
      case 0:
	xzc++;
	break;
      case 1:
	xpc++;
	break;
      case -1:
	xnc++;
	break;
    }
  }
  if((xpc == numForces) || (xnc == numForces)) {
    // not interesting - all X components have the same sign
    return false;
  }
  

  // Y
  ypc = 0; ync = 0; yzc = 0;
  for(i = 0; i < numForces; i++) {
    switch(SIGN(forces[i].yd)) {
      case 0:
	yzc++;	
	break;
      case 1:
	ypc++;	
	break;
      case -1:
	ync++;	
	break;
    }
  }
  if((ypc == numForces) || (ync == numForces)) {
    // not interesting - all Y components have the same sign
    return false;
  }
    
  // Z
  zpc = 0; znc = 0; zzc = 0;
  for(i = 0; i < numForces; i++) {
    switch(SIGN(forces[i].zd)) {
      case 0:
	zzc++;	
	break;
      case 1:
	zpc++;	
	break;
      case -1:
	znc++;	
	break;
    }
  }
  if((zpc == numForces) || (znc == numForces)) {
    // not interesting - all Z components have the same sign
    return false;
  }
  
#ifdef TRACE
  /*
  printf("\n\
ChangeInSign:  NrPoz  NrNeg  NrZer\n\
           X:    %d      %d      %d \n\
           Y:    %d      %d      %d \n\
           Z:    %d      %d      %d \n", 
	 xpc, xnc, xzc, ypc, ync, yzc, zpc, znc, zzc);
  */
#endif

  // if we are here, we found different signs in all 3 components
  return true;
}



bool FindCriticalPointInIntCell(
	int x, int y, int z, int* inds,
	int sX, int sY, int sZ, 
	ForceVector *ForceField, unsigned char *flags, bool inOut,
	int maxRecursionDepth, bool useNewton,
	VoxelPositionDouble *critPt)
{

  int kk, jj, ii;
  ForceVector cv[8];
  int nrCP;
  
#ifdef TRACE
  if((x == DBG_CELL_X) && (y == DBG_CELL_Y) && (z == DBG_CELL_Z)) {
    printf("Testing cell: %d, %d, %d\n", x, y, z);
    printf("\tForces:\n");
    for(ii=0; ii < 8; ii++) {
      printf("\t\t(%f,\t%f,\t%f)\n", 
	     ForceField[inds[ii]].xd, ForceField[inds[ii]].yd, 
	     ForceField[inds[ii]].zd);
    }
  }
#endif

  // if the force in any of the vertices of this cell is exactly 0
  // we have two possibilities (knowing that all vertices are inside)
  // 1. the voxel is actually a critical point and we are so lucky that it is
  //    located at integer coordinates (highly unlikely :)
  // 2. the force was never calculated for this voxel. This can happen for 
  //    surface voxels in potential fields. If the region around this voxel is 
  //    so thin that it has no interior neighbor, the force cannot be evaluated
  //    (remember that the force at surface voxels is not computed using the 
  //    potential field formula but by averaging the forces computed at 
  //    sub-surface voxels. 
  //    For normal diffusion fields, forces cannot be 0 on the surface
  //    For other types of fields ???
  //
  // The implementation only allows for one critical point to be reported back
  // However, it may be possible that more than one vertex of this cell is 0
  // and interior. The others may be reported when inspecting other cells 
  // but they may not be if the other cells containing them have an exterior 
  // vertex. This is very unlikely and I believe it should not happen. But just
  // to check, I am including a counter to count how many critical points we 
  // discover in the vertices. Since I can only return one, we are doomed 
  // anyway and we can only hope that the other will be reported from other 
  // cells; so I will print a warning and still return true even if more than
  // one vertex with force = 0 is detected.
  // 
  nrCP = 0; // counts how many vertices are interior and have a 0 force
  for(ii = 0; ii < 8; ii++) {
    if(	(IS_ZERO(ForceField[inds[ii]].xd)) &&
	(IS_ZERO(ForceField[inds[ii]].yd)) &&
	(IS_ZERO(ForceField[inds[ii]].zd)))
    {
      // the force is exactly 0.

      // if we know what is interior and exterior
      if(!inOut) {
	// if the voxel is exterior just ignore this cell 
	// -this should not happen
	if(flags[inds[ii]] == EXTERIOR) {
	  return false;
	}
	// if the voxel is not an interior voxel (it means it's either 
	// SURF or BOUNDARY) skip the cell
	if(flags[inds[ii]] != INTERIOR) {
	  return false;
	}

	// at this point, the vertex is INTERIOR and the force is 0
	// we should report it as a critical point
	// if the point is reported from multiple cells, duplicates will be
	// removed just before returning the critical points to the caller
	// However, it may not be reported multiple times if the other cells it
	// belongs to have an exterior vertex.
	
	//
	// compute coordinates from index value
	critPt->z = (int) (inds[ii] / (sX * sY));
	critPt->y = (int) ((inds[ii] - (critPt->z * (sX * sY))) / sX);
	critPt->x = (int) (inds[ii] - (critPt->z * (sX * sY)) - 
			     (critPt->y * sX));

#ifdef TRACE
	if((x == DBG_CELL_X) && (y == DBG_CELL_Y) && (z == DBG_CELL_Z)) {
	  printf("\n** In cell (%d, %d, %d): a vertex vector(%d) = 0. Critical point found at: (%.16lf, %.16lf, %.16lf)\n", 
		 DBG_CELL_X, DBG_CELL_Y, DBG_CELL_Z, ii, 
		 critPt->x, critPt->y, critPt->z);
	}      
#endif
	nrCP++;
	// return true;  // return later, after checking all vertices
      }
      else {
	// we do not know what is interior and what is exterior
	// report only the (0,0,0) vertex of the cell as a critical point
	// the rest of the the vertices (if there are others with force = 0) 
	// will be reported when inspecting the cell that has them as 
	// (0,0,0) vertices
	if(ii == 0) {
	  critPt->x = x;
	  critPt->y = y;
	  critPt->z = z;
	  return true;
	}
      }
    }
  }

  if(nrCP == 1) {
    // a single vertex with force = 0 was detected
    return true;
  }
  else {
    if(nrCP > 1) {
      // we have more than 1 vertex with force = 0
      printf("\
WARNING: In cell (%d, %d, %d): \n\
  %d vertices where the force is (0,0,0) were detected. The implementation \n\
  allows a single critical point to be reported ! \n\
  (You should hope the others are reported from other cells :)).\n",
	     x, y, z, nrCP);
      return true;
    }
  }

  //  
  // if we are here, no vertex has force = 0
  //

  // the cell is a candidate cell if there is a change of sign
  //	in one of the vector components among all eight vertices of the cell

  // copy cell vectors to the cv array. Vectors corresponding to outside 
  // voxels will not be copied
  jj = 0;
  for(ii = 0; ii < 8; ii++) {
    // if in/out is known and this voxel is outside, it is not copied
    if((!inOut) && (flags[inds[ii]] == EXTERIOR)) {
      // this is not copied
      // this should not happen
    }
    else {
      cv[jj] = ForceField[inds[ii]];
      jj++;
    }
  }
  // here jj gives the number of forces copied
  if(ChangeInSign(cv, jj)) {
    // it is a candidate cell
    // divide the cell in 8 subcells
    // for each of those 8 subcells do the candidate test again
    // and try to find the critical point in one of the candidate subcells
    // printf("\tCandidate cell\n");
    
#ifdef TRACE
    if((x == DBG_CELL_X) && (y == DBG_CELL_Y) && (z == DBG_CELL_Z)) {
      printf("Cell (%d, %d, %d) is a candidate cell.\n", x, y, z);
    }
    /*
      printf("\tForces:\n");
      for(ii=0; ii < 8; ii++) {
      printf("\t\t(%f,\t%f,\t%f)\n", 
      ForceField[inds[ii]].xd, ForceField[inds[ii]].yd, 
      ForceField[inds[ii]].zd);
      }
    */
#endif    

#ifdef TRACE
    /*
      if(((x == DBG_CELL_X) && (y == DBG_CELL_Y) && (z == DBG_CELL_Z)) 
      //|| 
      //((x >= 0) && (x <= 7))
    ) 
    {
      printf("\n** (%d, %d, %d): is a candidate cell.\n", 
	     x, y, z);
      
      printf("\tForces:\n");
      for(ii=0; ii < 8; ii++) {
	printf("\t\t(%.16lf,\t%.16lf,\t%.16lf)\n", 
	       ForceField[inds[ii]].xd, ForceField[inds[ii]].yd, 
	       ForceField[inds[ii]].zd);
      }
    }
    */
#endif

    //
    // divide the cell in ... sub-cells and look for critical points in each
    //
    int nrCPFound = 0;
    for(kk=0; kk < CELL_SUBDIVISION_FACTOR; kk++) {
      for(jj=0; jj < CELL_SUBDIVISION_FACTOR; jj++) {
	for(ii=0; ii < CELL_SUBDIVISION_FACTOR; ii++) {
	  if(
	     FindCriticalPointInFloatCell(
	       x + ((double)ii/(double)CELL_SUBDIVISION_FACTOR), 
	       y + ((double)jj/(double)CELL_SUBDIVISION_FACTOR), 
	       z + ((double)kk/(double)CELL_SUBDIVISION_FACTOR),
	       1.00 / (double)CELL_SUBDIVISION_FACTOR,
	       sX, sY, sZ, ForceField, flags, 
	       maxRecursionDepth, 1, useNewton, critPt))
	  {
	    // crtPt is already set
	    nrCPFound++;
	    //printf("here\n");
	    return true;
	  }
	}
      }
    }
    
    if(nrCPFound >= 1) {
      printf("OOPS: Found %d critical points in a single INT cell!\n", 
	     nrCPFound);
    }
  }
  else {
    // no change in sign
#ifdef TRACE
    /*
    if((x == DBG_CELL_X) && (y == DBG_CELL_Y) && (z == DBG_CELL_Z)) {
      printf("\n** (%d, %d, %d): is NOT a candidate cell.\n", 
	     x, y, z);
      printf("\tForces:\n");
      for(ii=0; ii < 8; ii++) {
	printf("\t\t(%.16lf,\t%.16lf,\t%.16lf)\n", 
	       ForceField[inds[ii]].xd, ForceField[inds[ii]].yd, 
	       ForceField[inds[ii]].zd);
      }
    }
    */
#endif
  }

#ifdef TRACE
  if((x == DBG_CELL_X) && (y == DBG_CELL_Y) && (z == DBG_CELL_Z)) {
    printf("Critical point not found in INT cell (%d, %d, %d). Returning false.\n",
	   x, y, z);
  }
#endif

  return false;
}


bool FindCriticalPointInFloatCell(
	double x, double y, double z, double cellsize,
	int sX, int sY, int sZ, 
	ForceVector *ForceField, unsigned char *flags, 
	int maxRecursionDepth, int recursionDepth, bool useNewton, 
	VoxelPositionDouble *critPt)
{
  int kk, jj, ii, i, idx;
  ForceVector cv[8];
  int ix, iy, iz;
  
#ifdef TRACE
  /*	
  printf("testing subcell: %f, %f, %f. Cell size: %.10f\n", x, y, z, cellsize);
  */
#endif

  // interpolate vector values at each of the 8 vertices of the cell
  cv[0] = interpolation(x, 		y, 		z,
			sX, sY, sZ, ForceField, flags);
  cv[1] = interpolation(x+cellsize, 	y, 		z,
			sX, sY, sZ, ForceField, flags);
  cv[2] = interpolation(x,		y+cellsize,	z,
			sX, sY, sZ, ForceField, flags);
  cv[3] = interpolation(x+cellsize,	y+cellsize,	z,
			sX, sY, sZ, ForceField, flags);
  cv[4] = interpolation(x,		y,		z+cellsize,
			sX, sY, sZ, ForceField, flags);
  cv[5] = interpolation(x+cellsize,	y,		z+cellsize,
			sX, sY, sZ, ForceField, flags);
  cv[6] = interpolation(x,		y+cellsize,	z+cellsize,	
			sX, sY, sZ, ForceField, flags);
  cv[7] = interpolation(x+cellsize,	y+cellsize,	z+cellsize,	
			sX, sY, sZ, ForceField, flags);

#ifdef TRACE	
  /*
  printf("\t\tForces:\n");
  for(int ii=0; ii < 8; ii++) {
    printf("\t\t\t(%.10f,\t%.10f,\t%.10f)\n", cv[ii].xd, cv[ii].yd, cv[ii].zd);
  }
  */
#endif

#ifdef TRACE
  if(((int(x) == DBG_CELL_X) && (int(y) == DBG_CELL_Y) && 
      (int(z) == DBG_CELL_Z))) 
  {
    printf("\n** Testing cell (%.16lf, %.16lf, %.16lf, size: %.16lf): .\n", 
	   x, y, z, cellsize);
    
    printf("\tForces:\n");
    for(ii=0; ii < 8; ii++) {
      printf("\t\t(%.16lf,\t%.16lf,\t%.16lf)\n", 
	     cv[ii].xd, cv[ii].yd, 
	     cv[ii].zd);
    }
  }
#endif


  
  // if first vertex vector (coresponding to (x, y, z)) is (0, 0, 0)
  //	then return (x, y, z) as the critical point.
  if(	(IS_ZERO(cv[0].xd)) &&
	(IS_ZERO(cv[0].yd)) &&
	(IS_ZERO(cv[0].zd)))
  {
    printf("\tFirst vertex vector = 0. Found a critical point (%lf, %lf, %lf).\n",
	   x, y, z);
    
    critPt->x = x;
    critPt->y = y;
    critPt->z = z;
    return true;
  }
  
  
  // the cell is a candidate cell if there is a change of sign
  //	in one of the vector components among all eight vertices of the cell
  if(ChangeInSign(cv, 8)) {

#ifdef TRACE
    /*
      ForceVector a;
      double l;
      a = interpolation(critPt->x, critPt->y, critPt->z, sX, sY, sZ, ForceField);
      l = sqrt(a.xd*a.xd + a.yd*a.yd + a.zd*a.zd);
      printf("\n\tCell size too small. Assume critical point is in the middle. \n");
      printf("\tForceVector lenght = %f\n", l);
      
      printf("\t\tForces:\n");
      for(int ii=0; ii < 8; ii++) {
      printf("\t\t\t(%lf,\t%lf,\t%lf)\n", cv[ii].xd, cv[ii].yd, cv[ii].zd);
      }
    */
#endif
    
    //
    // if we've reached the maximum recursion depth
    //
    if(recursionDepth >= maxRecursionDepth) {
      if(!useNewton) {
	//
	// case 1: stop here and assume critical point is in the center 
	//   of the cell
	//  - does NOT use Newton's method
	//
	critPt->x = x + (cellsize / 2.00);
	critPt->y = y + (cellsize / 2.00);
	critPt->z = z + (cellsize / 2.00);
	
#ifdef TRACE
	if(((int)x == DBG_CELL_X) && ((int)y == DBG_CELL_Y) && 
	   ((int)z == DBG_CELL_Z)) 
	{
	  printf("Cell size is too small. Critical point assumed to be in the center. In cell (%.16lf, %.16lf, %.16lf), size: %.16lf.\n",
		 x, y, z, cellsize);
	}
#endif
	
	return true;
      }
      else { // use Newton
	//
	// case 2: use Newton's method to find the critical point
	//
	
#ifdef TRACE	
	if(((int)x == DBG_CELL_X) && ((int)y == DBG_CELL_Y) && 
	   ((int)z == DBG_CELL_Z)) 
	{
	  printf("Candidate cell is too small - will use Newton method. Forces:\n");
	  for(int ii=0; ii < 8; ii++) {
	    printf("\t\t\t(%.16lf,\t%.16lf,\t%.16lf)\n", 
		   cv[ii].xd, cv[ii].yd, cv[ii].zd);
	  }
	}
#endif
	
	// before going into this procedure, which is very expensive, 
	// let's make sure we don't already know where the critical point is. 
	// That is, we will look at all the 8 vertices of this cell and make 
	// sure none is exactly 0.
	
#ifdef TRACE
	if(((int)x == DBG_CELL_X) && ((int)y == DBG_CELL_Y) && 
	   ((int)z == DBG_CELL_Z)) 
	{
	  printf("We are about to lauch Newton function. First, I will check whether one of the vertices is 0.\n");
	}
#endif
	
	i = 0;
	for(kk=0; kk < 2; kk++) {
	  for(jj=0; jj < 2; jj++) {
	    for(ii=0; ii < 2; ii++) {
	      if(IS_ZERO(cv[i].xd) && IS_ZERO(cv[i].yd) && 
		 IS_ZERO(cv[i].zd)) 
	      {
		// we found a critical point
		
#ifdef TRACE
		if(((int)x == DBG_CELL_X) && ((int)y == DBG_CELL_Y) && 
		   ((int)z == DBG_CELL_Z)) 
		{
		  printf("One of the vertices is 0! (%d, %d, %d). Checking if inside the object...\n", 
			 ii, jj, kk);
		}
#endif
		
		critPt->x = x + (ii * cellsize);
		critPt->y = y + (jj * cellsize);
		critPt->z = z + (kk * cellsize);
		
		// before we report it, 
		// let's make sure it's not outside the object
		ix = (int) round(critPt->x);
		iy = (int) round(critPt->y);
		iz = (int) round(critPt->z);
		idx = iz*sX*sY + iy*sX + ix;
		if(flags[idx] != EXTERIOR) {
		  
#ifdef TRACE
		  if(((int)x == DBG_CELL_X) && ((int)y == DBG_CELL_Y) && 
		     ((int)z == DBG_CELL_Z)) 
		  {
		    printf("Vertex is inside. Reporting critical point (%.16lf, %.16lf, %.16lf).\n",
			   critPt->x, critPt->y, critPt->z);
		  }
#endif
		  
		  // not outside the object - 
		  // report the critical point
		  return true;
		}
		else {
#ifdef TRACE
		  if(((int)x == DBG_CELL_X) && ((int)y == DBG_CELL_Y) && 
		     ((int)z == DBG_CELL_Z)) 
		  {
		    printf("Vertex is outside the object. Critical point not reported.\n");
		  }
#endif
		}
	      }
	      i++;
	    }
	  }
	}
	
	return 
	  FindCritPointNewton(x, y, z, cellsize, sX, sY, sZ, ForceField, flags, 
			      critPt);
      } // if(!useNewton)
      
      // this point will not be reached
    }
    
    
    // it is a candidate cell and we haven't reached the maximum recursion 
    // depth yet
    // divide the cell in 8 subcells
    // and try to find the critical point in one of the subcells
    // printf("\tCandidate subcell\n");
    int nrCPFound = 0;
    for(kk=0; kk < CELL_SUBDIVISION_FACTOR; kk++) {
      for(jj=0; jj < CELL_SUBDIVISION_FACTOR; jj++) {
	for(ii=0; ii < CELL_SUBDIVISION_FACTOR; ii++) {
	  if(FindCriticalPointInFloatCell(
	       x + (ii * (cellsize / (double)CELL_SUBDIVISION_FACTOR)), 
	       y + (jj * (cellsize / (double)CELL_SUBDIVISION_FACTOR)), 
	       z + (kk * (cellsize / (double)CELL_SUBDIVISION_FACTOR)), 
	       cellsize / (double)CELL_SUBDIVISION_FACTOR,
	       sX, sY, sZ, ForceField, flags, 
	       maxRecursionDepth, recursionDepth + 1, useNewton, 
	       critPt))
	  {
	    // crtPt is already set
	    //printf("here\n");
	    nrCPFound++;
	    return true;
	  }
	}
      }
    }
    if(nrCPFound >= 1) {
      printf("OOPS: Found %d critical points in a single FLOAT cell!\n", 
	     nrCPFound);
    }
  }

#ifdef TRACE
  if(((int)x == DBG_CELL_X) && ((int)y == DBG_CELL_Y) && 
     ((int)z == DBG_CELL_Z)) 
  {
    printf("Critical point not found in FLOAT cell (%.16lf, %.16lf, %.16lf), size: %.16lf. Returning false.\n",
	   x, y, z, cellsize);
  }
#endif

  return false;
}


bool ConstructJacobian(TNT::Array2D<double> *Jac, 
		       VoxelPositionDouble *pos, double vdist, 
		       int L, int M, int N, ForceVector *ForceField, 
		       unsigned char *flags, 
		       bool *foundAZeroNeighbor, ForceVector *zeroPosVector)
{
  ForceVector cv[6];
  (*foundAZeroNeighbor) = false;

  // let's make sure the current position +- vdist is within volume bounds
  // if, not we will try to adjust vdist
  double minVDist;
  minVDist = vdist;
  // X
  if((pos->x + vdist) > (L-1)) {
    if(((L-1) - pos->x) < minVDist) {
      minVDist = (L-1) - pos->x;
    }
  }
  if((pos->x - vdist) < 0) {
    if(pos->x < minVDist) {
      minVDist = pos->x;
    }
  }
  
  // Y
  if((pos->y + vdist) > (M-1)) {
    if(((M-1) - pos->y) < minVDist) {
      minVDist = (M-1) - pos->y;
    }
  }
  if((pos->y - vdist) < 0) {
    if(pos->y < minVDist) {
      minVDist = pos->y;
    }
  }

  // Z
  if((pos->z + vdist) > (N-1)) {
    if(((N-1) - pos->z) < minVDist) {
      minVDist = (N-1) - pos->z;
    }
  }
  if((pos->z - vdist) < 0) {
    if(pos->z < minVDist) {
      minVDist = pos->z;
    }
  }

  // quit if the minimum usable minVDist is <= 0
  if(IS_ZERO(minVDist) || (minVDist < 0)) {
    printf("ERROR: ConstructJacobian(...): the point is too close or outside the volume bounding box.\n");
    return false;
  }

  // else, use minVDist as vdist to guarantee that we can interpolate the force
  // at those locations
#ifdef TRACE
  if(((int)pos->x == DBG_CELL_X) && ((int)pos->y == DBG_CELL_Y) && 
     ((int)pos->z == DBG_CELL_Z)) 
  {
    if(minVDist < vdist) {
      printf("vdist was changed from %.16lf to %.16lf\n", vdist, minVDist);
      //exit(1);
    }
  }
#endif

  if(minVDist < vdist) {
    vdist = minVDist;
  }


  //
  // compute forces at x+, x-, y+, y-, z+ and z-
  // if any one of them is 0, I report it back to the caller
  // this can be used when looking for critical points to stop
  // looking and report that position as a critical point
  // when this is used to simply classify the critical points, this information
  // is ignored
  //
  cv[0] = interpolation(pos->x + vdist, pos->y, pos->z,
			L, M, N, ForceField, flags);
  if(IS_ZERO(cv[0].xd) && IS_ZERO(cv[0].yd) && IS_ZERO(cv[0].zd)) {
    (*foundAZeroNeighbor) = true;
    zeroPosVector->xd = +vdist; zeroPosVector->yd = 0; zeroPosVector->zd = 0;
  }

  cv[1] = interpolation(pos->x - vdist, pos->y, pos->z,
			L, M, N, ForceField, flags);
  if(IS_ZERO(cv[1].xd) && IS_ZERO(cv[1].yd) && IS_ZERO(cv[1].zd)) {
    (*foundAZeroNeighbor) = true;
    zeroPosVector->xd = -vdist; zeroPosVector->yd = 0; zeroPosVector->zd = 0;
  }

  cv[2] = interpolation(pos->x, pos->y + vdist, pos->z,
			L, M, N, ForceField, flags);
  if(IS_ZERO(cv[2].xd) && IS_ZERO(cv[2].yd) && IS_ZERO(cv[2].zd)) {
    (*foundAZeroNeighbor) = true;
    zeroPosVector->xd = 0; zeroPosVector->yd = +vdist; zeroPosVector->zd = 0;
  }

  cv[3] = interpolation(pos->x, pos->y - vdist, pos->z,
			L, M, N, ForceField, flags);
  if(IS_ZERO(cv[3].xd) && IS_ZERO(cv[3].yd) && IS_ZERO(cv[3].zd)) {
    (*foundAZeroNeighbor) = true;
    zeroPosVector->xd = 0; zeroPosVector->yd = -vdist; zeroPosVector->zd = 0;
  }

  cv[4] = interpolation(pos->x, pos->y, pos->z + vdist,
			L, M, N, ForceField, flags);
  if(IS_ZERO(cv[4].xd) && IS_ZERO(cv[4].yd) && IS_ZERO(cv[4].zd)) {
    (*foundAZeroNeighbor) = true;
    zeroPosVector->xd = 0; zeroPosVector->yd = 0; zeroPosVector->zd = +vdist;
  }

  cv[5] = interpolation(pos->x, pos->y, pos->z - vdist,
			L, M, N, ForceField, flags);
  if(IS_ZERO(cv[5].xd) && IS_ZERO(cv[5].yd) && IS_ZERO(cv[5].zd)) {
    (*foundAZeroNeighbor) = true;
    zeroPosVector->xd = 0; zeroPosVector->yd = 0; zeroPosVector->zd = -vdist;
  }
  
#ifdef TRACE
  if(((int)pos->x == DBG_CELL_X) && ((int)pos->y == DBG_CELL_Y) && 
     ((int)pos->z == DBG_CELL_Z)) 
  {
    printf("Force values for central differencing(+x, -x, +y, -y, +z, -z\n");
    for(int j=0; j < 6; j++) {
      printf("\t\t(%.16lf, \t%.16lf, \t%.16lf)\n", 
	     cv[j].xd, cv[j].yd, cv[j].zd);
    }
  }
#endif

    // construct the Jacobian matrix
    // is central differencing ok ???
    (*Jac)[0][0] = (cv[0].xd - cv[1].xd) / (2 * vdist);
    (*Jac)[0][1] = (cv[2].xd - cv[3].xd) / (2 * vdist);
    (*Jac)[0][2] = (cv[4].xd - cv[5].xd) / (2 * vdist);
    
    (*Jac)[1][0] = (cv[0].yd - cv[1].yd) / (2 * vdist);
    (*Jac)[1][1] = (cv[2].yd - cv[3].yd) / (2 * vdist);
    (*Jac)[1][2] = (cv[4].yd - cv[5].yd) / (2 * vdist);
    
    (*Jac)[2][0] = (cv[0].zd - cv[1].zd) / (2 * vdist);
    (*Jac)[2][1] = (cv[2].zd - cv[3].zd) / (2 * vdist);
    (*Jac)[2][2] = (cv[4].zd - cv[5].zd) / (2 * vdist);
  
  return true;
}


//
// implements Newton's method in 3D to find the critical point inside a 
// small enough cell.
//
bool FindCritPointNewton(double x, double y, double z, double cellsize,
			 int sX, int sY, int sZ, ForceVector *ForceField,
			 unsigned char *flags,
			 VoxelPositionDouble *critPt)
{
  //
  // initial guess P(0) = center of the cell
  // next guess = P(k+1) = P(k) + dP
  //   P(k) - guess at previous step
  //   dP = solution of the system: J(P(k))*dP = -F(P(k))
  //     where J(P) = Jacobian at point P
  //           F(P) = force vector at point P
  //
  // stop after MAX_NR_ITERATIONS or if we move out of the cell
  //
  
  TNT::Array2D<double> J(3, 3);
  TNT::Array1D<double> Pk(3), dP(3), FPk(3);
  VoxelPositionDouble P;
  ForceVector F;
  short cnt;
  bool foundAZeroNeighbor;
  ForceVector zeroPos;

  //
  // initial guess P0 = center of the cell
  //
  Pk[0] = x + (cellsize / 2.00);
  Pk[1] = y + (cellsize / 2.00);
  Pk[2] = z + (cellsize / 2.00);
  
  P.x = Pk[0];
  P.y = Pk[1];
  P.z = Pk[2];

#ifdef TRACE
  if(((int)x == DBG_CELL_X) && ((int)y == DBG_CELL_Y) && 
     ((int)z == DBG_CELL_Z)) 
  {
    printf("\
Starting Newton's method at: %.16lf, %.16lf, %.16lf. Cellsize: %.16lf\n", 
	   P.x, P.y, P.z, cellsize);
  }
#endif
  
  //
  // evaluate the force at Pk
  //
  F = interpolation(P.x, P.y, P.z, sX, sY, sZ, ForceField, flags);
  
#ifdef TRACE
  if(((int)x == DBG_CELL_X) && ((int)y == DBG_CELL_Y) && 
     ((int)z == DBG_CELL_Z)) 
  {
    printf("Force here:: %.16lf, %.16lf, %.16lf.\n", F.xd, F.yd, F.zd);
  }
#endif
  
  // 
  // while the force is not 0, we have not found the critical point
  //
  cnt = 0; // iteration count
  while((!IS_ZERO(F.xd) || !IS_ZERO(F.yd) || !IS_ZERO(F.zd)) && 
	(cnt < MAX_NR_ITERATIONS)) 
  {
    
#ifdef TRACE
    if(((int)x == DBG_CELL_X) && ((int)y == DBG_CELL_Y) && 
       ((int)z == DBG_CELL_Z)) 
    {
      printf("Newton step %d.\n", cnt);
    }
#endif
    
    FPk[0] = -F.xd;
    FPk[1] = -F.yd;
    FPk[2] = -F.zd;
    
    //
    // evaluate the jacobian at Pk
    //
    foundAZeroNeighbor = false;
    if(!ConstructJacobian(&J, &P, cellsize / 2.00, sX, sY, sZ, 
			  ForceField, flags,
			  &foundAZeroNeighbor, &zeroPos))
    {
      return false;
    }
    
    //
    // solve J(Pk)*dP = -F for dP
    //
    JAMA::LU<double> LUDecomp(J);
    dP = LUDecomp.solve(FPk);
    if(dP.dim() == 0) {
      //
      // the Jacobian is a singular matrix => system has no solution
      // In this case, this is not a critical point. ??
      //
#ifdef TRACE
      if(((int)x == DBG_CELL_X) && ((int)y == DBG_CELL_Y) && 
	 ((int)z == DBG_CELL_Z)) 
      {
	printf("Singular Jacobian !!\n");
      }
#endif
      
      // BUT !!!
      // check out the foundAZeroNeighbor parameter for ConstructJacobian
      // it may be that while constructing the Jacobian and evaluating the 
      // forces around this point, we evaluated a force to 0.
      // in this case, we can just report the critical point at that position
      /*
	Let's not use this yet - we shouldn't actually find 0's when looking at the neighbors
	This can happen for example in thin regions where the surface voxels have no interior neighbors and so the force at these voxels (which are not outside) remains 0.
	
	
	if(foundAZeroNeighbor) {
	critPt->x = P.x + zeroPos.xd;
	critPt->y = P.y + zeroPos.yd;
	critPt->z = P.z + zeroPos.zd;
	return true;
	}
      */
      return false;
    }

    //
    // next guess: Pk = Pk + dP
    //
    Pk[0] = Pk[0] + dP[0];
    Pk[1] = Pk[1] + dP[1];
    Pk[2] = Pk[2] + dP[2];
    P.x = Pk[0];
    P.y = Pk[1];
    P.z = Pk[2];
    
#ifdef TRACE
    if(((int)x == DBG_CELL_X) && ((int)y == DBG_CELL_Y) && 
       ((int)z == DBG_CELL_Z)) 
    {
      printf("\
Moving: %.16lf, %.16lf, %.16lf. \nTo: %.16lf, %.16lf, %.16lf.\n", 
	     dP[0], dP[1], dP[2], P.x, P.y, P.z);
    }
#endif

    //
    // stop if we are moving out of the integer cell
    //
    if((P.x < (int)x) || (P.x >= ((int)x + 1)) ||
       (P.y < (int)y) || (P.y >= ((int)y + 1)) ||
       (P.z < (int)z) || (P.z >= ((int)z + 1)))
    {

#ifdef TRACE
      //printf("FindCritPointNewton: Out of the original cell ! - stop.\n");
#endif
      
      cnt = MAX_NR_ITERATIONS;
      break;
    }


    //
    // evaluate the force at the new guess
    //
    F = interpolation(P.x, P.y, P.z, sX, sY, sZ, ForceField, flags);
    
#ifdef TRACE
    if(((int)x == DBG_CELL_X) && ((int)y == DBG_CELL_Y) && 
       ((int)z == DBG_CELL_Z)) 
    {
      printf("Force here:: %.16lf, %.16lf, %.16lf.\n", F.xd, F.yd, F.zd);
    }
#endif
    
    //
    // increment step counter
    //
    cnt++; 
  }

  // 
  // the force is either 0 
  //  or we made too many iterations 
  //  or we moved out of the cell
  //
  if(cnt >= MAX_NR_ITERATIONS) {
    // too many iterations or we moved out of the cell
    
#ifdef TRACE
    if(((int)x == DBG_CELL_X) && ((int)y == DBG_CELL_Y) && 
       ((int)z == DBG_CELL_Z)) 
    {
      printf("Too many iterations !\n");
    }
#endif
    
    return false;
  }
  
  //
  // the force is 0 at P - we found it
  //
  (*critPt) = P;
  
  return true;

}





bool ClassifyCriticalPoint(CriticalPoint &critPt, 
			   ForceVector *ForceField,    // [in] vector field
			   unsigned char *flags,
			   int L, int M, int N        // [in] volume size
			   )
{
  // clasify the critical point as: atracting/repelling nodes or saddles
  // use the default recursion depth to compute the distance to neighbors
  double vdist = (1.00 / pow(CELL_SUBDIVISION_FACTOR, 20)) / 2.00;
  
  TNT::Array2D<double> 	Jac(3, 3, 0.00);
  TNT::Array2D<double>	EigVals(3, 3, 0.0);
  TNT::Array2D<double>	EigVects(3, 3, 0.0);
  
  bool foundAZeroNeighbor;
  ForceVector zeroPos;
  
  char nrp, nrn, nrz;
  int i;
  
  // since critical points are at least 1 voxel inside the boundaing box, 
  //	we will not check that the neighbor coordinates are inside the 
  //    volume bounds, because they should be
  
  //
  // interpolate the force vector at 6 neighbors of this point.
  //

  /*
#ifdef TRACE
  printf("Point (%.16lf, %.16lf %.16lf):\n",
	 critPt.position.x, 
	 critPt.position.y, 
	 critPt.position.z);
#endif
  */
  
  // get the Jacobian at the critical point
  
  if(!ConstructJacobian(&Jac, &(critPt.position), vdist, 
			L, M, N, ForceField, flags, 
			&foundAZeroNeighbor, &zeroPos))
  {
    return false;
  }

  // find the eigenvalues and eigenvectors of the Jacobian
    
  JAMA::Eigenvalue<double>		ev(Jac);
  ev.getD(EigVals);
  ev.getV(EigVects);
  
#ifdef TRACE
  printf("\tThe eigenvalues of Jac:\n");
  printf("\t\t%.16lf  %.16lf  %.16lf\n", 
	 EigVals[0][0], EigVals[0][1], EigVals[0][2]);
  printf("\t\t%.16lf  %.16lf  %.16lf\n", 
	 EigVals[1][0], EigVals[1][1], EigVals[1][2]);
  printf("\t\t%.16lf  %.16lf  %.16lf\n", 
	 EigVals[2][0], EigVals[2][1], EigVals[2][2]);
  
  printf("\tThe eigenvectors of Jac:\n");
  printf("\t\t%.16lf  %.16lf  %.16lf\n", 
	 EigVects[0][0], EigVects[0][1], EigVects[0][2]);
  printf("\t\t%.16lf  %.16lf  %.16lf\n", 
	 EigVects[1][0], EigVects[1][1], EigVects[1][2]);
  printf("\t\t%.16lf  %.16lf  %.16lf\n", 
	 EigVects[2][0], EigVects[2][1], EigVects[2][2]);
#endif
  
  // analyze the eigenvalues:
  
  // Aug 16, 2006
  // a negative eigenvalue indicates attracting flow
  // a positive eigenvalue indicates repelling flow
  // a zero eigenvalue indicates stationary flow - not attracting nor repelling
  // To classify a point, we will ignore zero eigenvalues. In fact, we should 
  // show a warning if we have 0 eigenvalues - I believe it means the point 
  // cannot be fully classified
  
  // count how many eigenvalues are positive, negative and zero
  nrp = 0; nrn = 0; nrz = 0;
  for(i=0; i < 3; i++) {
    if(EigVals[i][i] == 0) {
      nrz++;
    }
    else {
      if(EigVals[i][i] > 0) {
	nrp++;
      }
      else {
	if(EigVals[i][i] < 0) {
	  nrn++;
	}
	else {
	  printf("ERROR: eigenvalue %d of critical point (%lf, %lf, %lf) is not a valid number !\n", 
		 i, critPt.position.x, critPt.position.y, critPt.position.z);
	  return false;
	}
      }
    }
  }

  // print a warning if there are zero eigenvalues !!
  if(nrz != 0) {
    printf("WARNING: critical point (%lf, %lf, %lf) could not be completely classified (a zero eigenvalue was found) !\n", 
	   critPt.position.x, critPt.position.y, critPt.position.z);
  }
  
  // all non-zero real parts of the eigenvalues are negative: - attracting node
  
  if(nrn == (3 - nrz)) {
    critPt.type = CPT_ATTRACTING_NODE;
#ifdef TRACE
    printf("Attracting node.\n");
#endif
  }
  else {
    // all non-zero real parts of the eigenvalues are pozitive:- repelling node
    if(nrp == (3 - nrz)) {
      critPt.type = CPT_REPELLING_NODE;
#ifdef TRACE
      printf("Repelling node.\n");
#endif
      
    }
    else {
      // some real parts of the eigenvalues are of one sign 
      //   and the others have the opposite sign
      //	- this is a saddle point
      critPt.type = CPT_SADDLE;
#ifdef TRACE
      printf("Saddle point.\n");
#endif
    }
  }
  
  
  critPt.eval[0] = EigVals[0][0];
  critPt.eval[1] = EigVals[1][1];
  critPt.eval[2] = EigVals[2][2];
  
  // normalize the eigenvectors
  for(i=0; i < 3; i++) {
    critPt.evect[i].xd = EigVects[0][i];
    critPt.evect[i].yd = EigVects[1][i];
    critPt.evect[i].zd = EigVects[2][i];

    Normalize(&(critPt.evect[i]));
  }

  return true;
}





// reads in critical points from a file
// the array has to be allocated by the caller
bool ReadCriticalPoints(char *filename, 
			DynamicArray<CriticalPoint> &critPts)
{
  FILE *fin;
  char line[500];
  double f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14, f15;
  int t;
  
  CriticalPoint cp;
  
  fin = fopen(filename, "r");
  if(fin == NULL) {
    printf("Cannot open %s for reading.\n", filename);
    return false;
  }

  while(!feof(fin)) {
    if(fgets(line, 500, fin) != NULL) {
      //printf("%s\n", line);
      
      if(sscanf(line, "%lf %lf %lf %d %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf", 
		&f1, &f2, &f3, &t, 
		&f4, &f5, &f6, &f7, &f8, &f9, &f10, &f11, &f12, 
		&f13, &f14, &f15) == 16) 
      {	
	cp.position.x = f1;
	cp.position.y = f2;
	cp.position.z = f3; 
	cp.type = (CriticalPointType)t;
	cp.eval[0] = f4;
	cp.eval[1] = f5;
	cp.eval[2] = f6;
	cp.evect[0].xd = f7;
	cp.evect[0].yd = f8;
	cp.evect[0].zd = f9;
	cp.evect[1].xd = f10;
	cp.evect[1].yd = f11;
	cp.evect[1].zd = f12;
	cp.evect[2].xd = f13;
	cp.evect[2].yd = f14;
	cp.evect[2].zd = f15;
	  
	critPts.Append(cp);
      }
      else {
	printf("Error reading critical points file.\n");
	return false;
      }
    }
  }	

  fclose(fin);
  return true;
}


// save critical points to a file
bool SaveCriticalPoints(DynamicArray<CriticalPoint> &critPts, 
			char *filename)
{
  FILE *fout;
  int i;

  if ((fout = fopen(filename,"w")) == NULL) {
    printf("Cannot open %s for writing\n", filename);
    return false;
  }
  
  for(i=0; i < critPts.GetNrElem(); i++) {
    fprintf(fout,"%.16lf %.16lf %.16lf %d %.16lf %.16lf %.16lf %.16lf %.16lf %.16lf %.16lf %.16lf %.16lf %.16lf %.16lf %.16lf\n", 
	    critPts[i].position.x, critPts[i].position.y, 
	    critPts[i].position.z, 
	    critPts[i].type, 
	    critPts[i].eval[0], critPts[i].eval[1], critPts[i].eval[2], 
	    critPts[i].evect[0].xd, critPts[i].evect[0].yd, 
	    critPts[i].evect[0].zd, 
	    critPts[i].evect[1].xd, critPts[i].evect[1].yd, 
	    critPts[i].evect[1].zd, 
	    critPts[i].evect[2].xd, critPts[i].evect[2].yd, 
	    critPts[i].evect[2].zd);
  }
  fclose(fout);
  return true;
}

