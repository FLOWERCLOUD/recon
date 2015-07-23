#include "common.h"
#include "tnt_array2d.h"
#include "tnt_array1d.h"
#include "jama_eig.h"
#include "jama_lu.h"


bool GetCriticalPoints(
	ForceVector *ForceField,    // [in] vector field
	int L, int M, int N,        // [in] volume size
	unsigned char *flags,       // [in] volume flags
	DynamicArray<CriticalPoint> &CritPts, // [out] detected critical points
	//CriticalPoint **CritPts,  
	//int *numCritPts,            // [out] critical points count
	bool inOut = false,         // [in] flag specifying if we know the 
                                    //    inside of the object from the outside
                                    //    false - can distinguish the inside
	                            //    true - cannot distinguish the inside
	                            //    DEFAULF: false
	bool notCloseToSurf = false,// [in] do not search for critical points
	                            //    close to the surface (on the surface
                                    //    or 1 voxel away) 
	int maxRecursionDepth = 20, // [in] specified maximum depth of 
	                            //    recursion when dividing a candidate 
	                            //    cell
	bool useNewton = true       // [in] specified whether Newton's method
	                            // should be used to localize the critical 
	                            // points more accurately
);




bool FindCriticalPointInIntCell(int x, int y, int z, int* inds,
				int sX, int sY, int sZ, 
				ForceVector *ForceField,
				unsigned char *flags, bool inOut,
				int maxRecursionDepth, bool useNewton,
				VoxelPositionDouble *critPt);

bool ClassifyCriticalPoint(CriticalPoint &critPt, 
			   ForceVector *ForceField,    // [in] vector field
			   unsigned char *flags,
			   int L, int M, int N        // [in] volume size
			   );

///////////////////////////////////////////////////////////////////////////////
// Function ReadCriticalPoints
//   Reads critical points from a file
///////////////////////////////////////////////////////////////////////////////
bool ReadCriticalPoints(char *filename, 
			DynamicArray<CriticalPoint> &critPts);
///////////////////////////////////////////////////////////////////////////////
// Function SaveCriticalPoints
//   Saves critical points to a file
///////////////////////////////////////////////////////////////////////////////
bool SaveCriticalPoints(DynamicArray<CriticalPoint> &critPts, 
			char *filename);
