#include "common.h"


bool GetHighDivergencePoints(
	ForceVector *ForceField,      // [in] vector field
	int L, int M, int N,	      // [in] size of vector field (X, Y and Z)
	unsigned char *flags,	      // [in] flags array
	float perc,		      // [in] percentage of high div. points 
	                              //         to be returned (top <perc> %)
	DynamicArray<DivergencePoint> &HDPts,  // [out] high divergence 
	                              // point list
	bool inOut = false,           // [in] flag specifying if we should look
	                              //    outside the object too (if true).
	                              // DEFAULT: false
	HDSelection hdSel = HDS_LocMin
	                              // [in] specifies how the high divergence
	                              //    points are selected:
	                              //    from all points (HDS_All) or only
	                              //    from local minima (HDS_LocMin)
	                              // DEFAULT: HDS_LocMin 
);

///////////////////////////////////////////////////////////////////////////////
// computes the divergence for every voxel.
///////////////////////////////////////////////////////////////////////////////
bool ComputeDivergence(ForceVector *ForceField, // [in] vector field
		       int L, int M, int N,     // [in] size of vector field 
		                                //      (X, Y and Z)
		       unsigned char *flags,	// [in] flags array
		       bool inOut,              // [in] in/out flag
		       double *div              // [out] divergence
		       );

// save/read HD points to/from file
bool SaveHDPoints(char *fileName, DynamicArray<DivergencePoint>& HDPts);
bool ReadHDPoints(char *fileName, DynamicArray<DivergencePoint>& HDPts);
