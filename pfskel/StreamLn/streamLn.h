#include "common.h"
// #include "dynamicArray.h"
#include "skeleton.h"


///////////////////////////////////////////////////////////////////////////////
// Get level 1, 2 and 3 skeletons combined in one step
///////////////////////////////////////////////////////////////////////////////

bool GetStreamLines(
        ForceVector *ForceField, // [in] vector field
	unsigned char *flags,    // [in] volume flags
	int L, int M, int N,     // [in] vector field size (X, Y and Z)
	DynamicArray<CriticalPoint> &CritPts, // [in] critical points array
	DynamicArray<VoxelPositionDouble> &BdSeeds, //[in] boundary seeds array
	DynamicArray<DivergencePoint> &HDPoints, // [in] high divergence 
	                                             //   points array
        Skeleton &Skel,          // [out] skeleton points and segments
	float *distField = NULL         // [in] distance field 
);


//////////////////////////////////////////////////////////////////////////////
// Get basic skeleton (level 1): connects only critical points
//////////////////////////////////////////////////////////////////////////////
bool GetLevel1Skeleton(
        ForceVector *ForceField,	// [in] vector field
	unsigned char *flags,    // [in] volume flags
	int L, int M, int N,		// [in] vector field size (X, Y and Z)
	DynamicArray<CriticalPoint> &CritPts,  // [in] critical points array
	Skeleton *Skel,                 // [out] skeleton
	float *distField                // [in] distance field - may be NULL
);

/////////////////////////////////////////////////////////////////////////////
// GetLevel2Skeleton: connects the high divergence points to the existing
//    skeleton
/////////////////////////////////////////////////////////////////////////////
bool GetLevel2Skeleton(
	ForceVector *ForceField, 	// [in] vector field
	unsigned char *flags,    // [in] volume flags
	int L, int M, int N,		// [in] vector field size (X, Y and Z)
	DynamicArray<DivergencePoint> &HDPoints, // [in] high divergence 
	                                             // points array
	Skeleton *Skel,                 // [in, out] skeleton
                                        //   in - level 1 skeleton
                                        //   out - level 2 skeleton
	float *distField                // [in] distance field - may be NULL
);


/////////////////////////////////////////////////////////////////////////////
// GetLevel3Skeleton: connects the boundary seeds points to the existing
//    skeleton
/////////////////////////////////////////////////////////////////////////////
bool GetLevel3Skeleton(
        ForceVector *ForceField, 	// [in] vector field
	unsigned char *flags,    // [in] volume flags
	int L, int M, int N,		// [in] vector field size (X, Y and Z)
	DynamicArray<VoxelPosition> &BoundarySeeds,// [in] boundary seeds array
	Skeleton *Skel,                 // [in, out] skeleton points and
	                                //              segments
	float *distField = NULL         // [in] distance field
);
