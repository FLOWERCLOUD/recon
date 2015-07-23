// Interface for the high divergence module
// This function serves only as a non-changing interface to the high divergence
//    module that has it's implementation in another file
// It just calls the right implementation and returns

#include "highCurv.h"
#include "localMinCurv.h"
#include "allCurv.h"

bool GetHighCurvaturePoints(
	unsigned char* flags, 	      // [in] input volume (flags)
	int L, int M, int N,	      // [in] size of vector field (X, Y and Z)
	unsigned char *flags,	      // [in] flags array
	float perc,		      // [in] percentage of high div. points 
	                              //         to be returned (top <perc> %)
	VoxelPositionDouble **HDPts,  // [out] high divergence point list
	int *numHDPts,		      // [out] number of points in the list
	bool inOut,                   // [in] flag specifying if we should look
	                              //    outside the object too (if true).
	                              // DEFAULT: false
	HDSelection hdSel             // [in] specifies how the high divergence
	                              //    points are selected:
	                              //    from all points (HDS_All) or only
	                              //    from local minima (HDS_LocMin)
	                              // DEFAULT: HDS_LocMin 
) {

  
  return true;
}
