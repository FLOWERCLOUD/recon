#include "common.h"


bool GetAllDivPoints(
        double *divergence,           // [in] divergence array
	int L, int M, int N,	      // [in] size of vector field (X, Y and Z)
	unsigned char *flags,	      // [in] flags array
	float perc,		      // [in] percentage of high div. points 
	                              //         to be returned (top <perc> %)
	DynamicArray<DivergencePoint> &HDPts,  // [out] high divergence 
	                              //   point list
	bool inOut = false            // [in] flag specifying if we should look
	                              //    outside the object too (if true).
	                              // DEFAULT: false
);
