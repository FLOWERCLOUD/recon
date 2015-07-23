#include "common.h"
#include "PotField/potVect.h"
// #include "HCB/hcBound.h"
#include "CritPts/critPts.h"
#include "StreamLn/streamLn.h"
#include "ExpandVol/expandVol.h"
#include "MakeSolidVol/makeSolidVol.h"
#include "HighDiverg/highDiverg.h"
#include "DistField/getDT.h"

typedef struct {
  char cmdCode[1];
  float HDP;
  float HCP;
} pfSkelCommand;

typedef bool (*cbChangeParameters)(Skeleton* Skel, pfSkelCommand* cmd, 
				   void *other);

//
// Function pfSkel : computes the skeleton of a 3D object
//
//    When passed to the function, Skel has to be a pointer to a Skeleton
//      structure.
//    !! 
//    Use AllocateSkeleton(&Skel) to allocate and initialize the structure;
//    Use FreeSkeleton(&Skel) to deallocate the structure.
//
bool pfSkel(
	char* volFileName,            // [in]  volume file name
	int L, int M, int N,	      // [in]  volume size (x, y and z)
	int distCharges,              // [in]  distance from object boundary
	                              //    where to place the charges (>=0)
	int fieldStrenght,	      // [in]  potential field strenght (4..9)
	float pHDPts,	              // [in]  percentage of high divergence 
	                              //    points to use
	Skeleton *Skel,               // [out] pointer to a Skeleton structure
	                              //    that will hold the skeleton
	cbChangeParameters pfnChangeParams = NULL, 
	                              // [in]  callback function to change 
	                              //    parameters on the run.
	void *other = NULL,           // [in] value to be passed to the 
	                              //    callback function when called
	char *vfInFile = NULL,        // [in] vector field input file. If this 
	                              //   parameter is not NULL, the 
	                              //   potential field is read from the 
                                      //   file instead of being calculated 
                                      //   here.
	char *vfOutFile = NULL,       // [in] dump vector field to this file
	PFNorm norm = PF_NORM_L2,     // [in] vector norm to use
	HDSelection hdSel = HDS_LocMin,
	                              // [in] specifies how the high divergence
	                              //    points are selected:
	                              //    from all points (HDS_All) or only
	                              //    from local minima (HDS_LocMin)
	                              // DEFAULT: HDS_LocMin
	bool remOutSegs = true,       // [in] remove segments that go outside
                                      //    the original object
	                              // DEFAULT: true
	FieldType fType = PF,         // [in] type of vector field to use:
	                              //    PF  = potential field (slower)
	                              //    GDF = gradient diffusion field 
	                              //          (faster)
	bool useDistanceField = false // [in] use distance field
	                              // DEFAULT: false
);
