#include "common.h"
//
// Constants
//
// consider only half of the boundary points
//	- uncomment the definition bellow
// #define HALF_BOUNDARY_POINTS
// consider all the boundary points
//	- comment the definition above


// if a boundary point is at a distance greater than the PF_THRESHOLD, 
//	then it is ignored i.e. it does not influence the field at his point
// Setting this to a very low value is not a good ideea: 
//    example imagine a very long cylinder. Setting thhis threshold smaller 
//    than half of the length of the cylinder, will cause the field not to 
//    flow towards the one attracting point in the middle of the cylinder. 
//    Instead it will only go towards the center, creating a critical point
//    at each slice along the cylinder.
#define PF_THRESHOLD	150


// Functions

bool SortByX(int startAt, int endAt, VoxelPosition Bound[]);
bool SortByY(int startAt, int endAt, VoxelPosition Bound[]);
bool SortByZ(int startAt, int endAt, VoxelPosition Bound[]);


///////////////////////////////////////////////////////////////////////////////
// Function CalculatePotentialField
//   Computes the potential field generated by placing electrical charges
//   on the boundary of a 3D object.
///////////////////////////////////////////////////////////////////////////////
bool CalculatePotentialField(
	int L, int M, int N, 	      // [in] size of volume
	unsigned char* f, 	      // [in] volume flags
	int fieldStrenght,	      // [in] potential field strenght
	ForceVector *force,		      // [out] force field	
	bool inOut = false,           // [in] in/out flag
	PFNorm norm = PF_NORM_L2      // [in] vector norm to use
);
