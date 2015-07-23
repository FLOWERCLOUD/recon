//////////////////////////////////////////////////////////
// Common include file for the skeletonization project.
//
// Nicu D. Cornea - Wed May 28 16:20:04 EDT 2003
//////////////////////////////////////////////////////////

#ifndef NCD_SKEL_COMMON_DEFINED
#define NCD_SKEL_COMMON_DEFINED

// Includes

#ifdef WIN32
	#include <windows.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "stack.h"
#include "dynamicArray.h"

#ifndef WIN32
	#include <sys/time.h>
#else
	#include <time.h>
#endif


// Macros

#define MIN(x,y) (((x) < (y))?(x):(y))
#define MAX(x,y) (((x) > (y))?(x):(y))

#define EPSILON                 0.00000000001
#define IS_ZERO(nr)             (((nr) < EPSILON) && ((nr) > -EPSILON))
#define EQUAL(nr1, nr2)	        (IS_ZERO((nr1) - (nr2)))


// Types

enum FieldType {
  PF = 0,   /* Potential Field */
  GDF       /* Gradient Diffusion Field */
};

typedef struct {
	short x;
	short y;
	short z;
} VoxelPosition;

// For large datasets, using double will cause memory shortage
typedef struct {
  double /*float*/ xd;   
  double /*float*/ yd;
  double /*float*/ zd;
} ForceVector;

enum CriticalPointType {
	CPT_SADDLE = 1,
	CPT_ATTRACTING_NODE,
	CPT_REPELLING_NODE,
	CPT_UNKNOWN
};

struct  VoxelPositionDouble
{
  double /*float*/ x;
  double /*float*/ y;
  double /*float*/ z;
};

struct CriticalPoint {
	VoxelPositionDouble		position;
	CriticalPointType		type;
        ForceVector 			evect[3];
	double				eval[3];
};

struct DivergencePoint {
  VoxelPositionDouble position;
  double              div;
};


// norm of vector
typedef enum {
  PF_NORM_L1 = 0,
  PF_NORM_L2
} PFNorm;


typedef enum {
  HDS_LocMin = 0,
  HDS_All
} HDSelection;


typedef struct {
  char          shortName[10];
  char          longName[30];
  unsigned char nrValues;
  bool          found;
  char*         values[10];   
} Option;


// Constants

#define SURF 			100		// surface voxel
#define SUB_SURF                 99             // sub-surface voxel
#define BOUNDARY		110		// boundary voxel - participates in potential field calculation
#define INTERIOR 		200		// interior voxel
#define PADDING_MIN		210		// added voxels in order to thick the object
#define NR_PAD_VALUES	 40		// are in this range: PADDING_MIN to PADDING_MIN + NR_PAD_VALUES
#define EXTERIOR		  0		// background (exterior to the object) voxel (air)
#define OUTSIDE_FORCE_MARKER  -2.00             // marks the force for an outside voxel

// some constants
#define SKEL_SEG_LEFT   0   // left end point of the segment
#define SKEL_SEG_RIGHT  1   // right end point of the se
#define SKEL_SEG_FIRST  2   // first point of the segment 
                            //    excluding the left end point
#define SKEL_SEG_LAST   3   // last point of the segment, 
                            //   excluding the right end point

// maximum number of critical points
// #define MAX_NUM_CRITPTS	1500

// maximum number of high divergence points
// #define MAX_NUM_HDPTS	10000

// maximum number of skel points and segments
// #define MAX_NUM_SKEL	      1000000
// #define MAX_NUM_SKEL_SEGMENTS 10000



// weights for averaging
#define gf_ws   0.25                           // self
#define gf_wf  (0.50) / 6.00                  // face
#define gf_we  (0.25) * (3.00 / 4.00) / 12.00 // edge
#define gf_wv  (0.25) * (1.00 / 4.00) / 8.00  // vertex


// Functions
void SetStartTime();
void PrintElapsedTime(const char* message);

//
// Information or debug info messages
//
void inline PrintInfoMessage(const char *message) {
  printf(message);
  fflush(stdout);
}

void inline PrintInfoMessage(const char *message, const char *arg1) {
  printf(message, arg1);
  fflush(stdout);
}

void inline PrintInfoMessage(const char *message, const int arg1) {
  printf(message, arg1);
  fflush(stdout);
}

// Debug messages
void inline PrintDebugMessage(const char *message) {
  #ifdef _DEBUG
    printf(message);
    fflush(stdout);
  #endif
}

void inline PrintDebugMessage(const char *message, const int arg1) {
  #ifdef _DEBUG
    printf(message, arg1);
    fflush(stdout);
  #endif
}

void inline PrintDebugMessage(const char *message, const char *arg1) {
  #ifdef _DEBUG
    printf(message, arg1);
    fflush(stdout);
  #endif
}

//
// Error messages
//
void inline PrintErrorMessage(const char *message) {
  printf("** Error ** ");
  printf(message);
  fflush(stdout);
}

///////////////////////////////////////////////////////////////////////////////
// Evaluates the force at an outside poisiton by averaging the known neighbors
// !!! For this to work, the outside voxel must have at least one neighbor 
// that is SURF, BOUNDARY or INTERIOR.
// Otherwise, the function returns false and the output vector will be 0
///////////////////////////////////////////////////////////////////////////////
bool EvaluateOutsideForceVector(int x, int y, int z, 
				ForceVector *field, int L, int M, int N, 
				unsigned char *flags, ForceVector *force);


///////////////////////////////////////////////////////////////////////////////
// reads volume data from a given file
///////////////////////////////////////////////////////////////////////////////
bool ReadVolume(char *filename, int L, int M, int N, unsigned char **vol);

///////////////////////////////////////////////////////////////////////////////
// write volume data from a given file
///////////////////////////////////////////////////////////////////////////////
bool SaveVolume(char *filename, int L, int M, int N, unsigned char *vol);


///////////////////////////////////////////////////////////////////////////////
// Function ReadVectorField
//   Reads a vector field from a file into a ForceVector array.
///////////////////////////////////////////////////////////////////////////////
bool ReadVectorField(ForceVector *field, int L, int M, int N, char *fileName);


///////////////////////////////////////////////////////////////////////////////
// Function SaveVectorField
//   Saves a vector field to a file
///////////////////////////////////////////////////////////////////////////////
bool SaveVectorField(ForceVector *field, int L, int M, int N, char *fileName);





///////////////////////////////////////////////////////////////////////////////
// Checks that the volume is padded with at lease 1 empty plane 
//  in all 3 directions, also considering that the object will be expanded
//  by a number of layers specified by distCharges
///////////////////////////////////////////////////////////////////////////////
bool CheckVolumePadding(
        unsigned char *vol,    // [in] volume to be checked
	int L, int M, int N,   // [in] volume size (X, Y and Z).
	int distCharges = 1    // [in] charges distance from object boundary 
	);

///////////////////////////////////////////////////////////////////////////////
// Pads a volume so that it has at least <n> layers of empty voxels in every 
//   direction between the actual object and the bounding box of the volume.
// !!
// !! This will change the volume size and the volume pointer !!
// !! The memory occupied by vol before the call to this function will be
// !!   freed using delete [] (*vol), a new space will be allocated for the
// !!   new volume and a pointer to that location will be returned in (*vol).
// !!
// Returns the new dimensions and the number of layers added/deleted in each 
//   direction at the origin (layers added before the object starts).
///////////////////////////////////////////////////////////////////////////////
bool PadVolume(
        unsigned char **vol, 	         // [in, out] volume to be padded
	int *L, int *M, int *N,          // [in, out] volume size X,Y and Z. 
	int nEmpty,                      // [in] number of minimum empty 
	                                 //   layers in each direction 
	int *dL, int *dM, int *dN        // [out] # layers added/del'd at the 
	                                 //   origin
);


///////////////////////////////////////////////////////////////////////////////
// for volume vol, sets the flags array to SURF, INTERIOR or EXTERIOR for 
//     every voxel
// Creates the flags array
///////////////////////////////////////////////////////////////////////////////
// bool SetFlags(unsigned char *vol, int L, int M, int N, unsigned char **flags);

///////////////////////////////////////////////////////////////////////////////
// for volume vol, sets the voxel values to SURF, INTERIOR or EXTERIOR
// in place - does not make a copy of the volume
///////////////////////////////////////////////////////////////////////////////
bool FlagVolume(unsigned char *vol, int L, int M, int N);


///////////////////////////////////////////////////////////////////////////////
// tests if the line between the 2 points crosses the boundary of the object
//	that is, if the line passes through a "blank" voxel
///////////////////////////////////////////////////////////////////////////////
bool IsLineCrossingBoundary(
	short x1, short y1, short z1,
	short x2, short y2, short z2,
	int sX, int sY, int sZ,
	unsigned char* flags);


///////////////////////////////////////////////////////////////////////////////
// Function GetSizeFromFilename
//   Parses a filename and returns the size encoded in the filename as
//      <name><nn>x<nn>x<nn>.vol
///////////////////////////////////////////////////////////////////////////////
bool GetSizeFromFilename(char* filename, int *L, int *M, int *N);


///////////////////////////////////////////////////////////////////////////////
// Function ChangeSizeInFilename
//   Parses a filename and changes the size encoded in the filename as
//      <name><nn>x<nn>x<nn>.vol to the specified values
//   If there is no size specified in the file name, the size is inserted
//     before the extension. If there is not extension, the size is appended
//     at the end of the name.
///////////////////////////////////////////////////////////////////////////////
bool ChangeSizeInFilename(char* oldFilename, int newL, int newM, int newN, 
			  char *newFilename);

///////////////////////////////////////////////////////////////////////////////
// Function GetVolExtent
//   Get the volume extents: minimum and maximum coordinates of object voxels.
///////////////////////////////////////////////////////////////////////////////
bool GetVolExtent(unsigned char *vol, int L, int M, int N, 
		  int *minX, int *maxX, int *minY, int *maxY, 
		  int *minZ, int *maxZ);


///////////////////////////////////////////////////////////////////////////////
// Function GetProgramOptions
//   Reads program options from the command line.
// Options start with a - and no other parameters are allowed to start with -
///////////////////////////////////////////////////////////////////////////////
bool GetProgramOptions(char *argv[], int argc, int optStartIndex, 
                       Option *opts, int nrOpts);

///////////////////////////////////////////////////////////////////////////////
// Function BuildOutputRootFileName
//   Builds the root of the output file name as:
//     - the root of the input file (what comes before the size)
//     - dc<nn> charges distance
//     - fs<nn> field stregth
//   Final root name: <name>-dc<n1>-fs<n2>
// At this, the percentage of high divergence points will be added and .skel 
///////////////////////////////////////////////////////////////////////////////
bool BuildOutputRootFileName(char* inputFileName, int distCharges, 
			     int fieldStrength, char** outFileName);


///////////////////////////////////////////////////////////////////////////////
// Average values over a vector field
///////////////////////////////////////////////////////////////////////////////
bool SmoothVectorField(ForceVector *field, int L, int M, int N);


///////////////////////////////////////////////////////////////////////////////
// tri-linear interpolation of force vector in a voxel cell
///////////////////////////////////////////////////////////////////////////////
inline ForceVector interpolation(
  double x, double y, double z,
  int sizx, int sizy, int sizz,
  ForceVector *forcevec, unsigned char *flags)
{
  double alpha, beta, gamma, factor;
  ForceVector forceInt;
  long slsz;
  
  // !!!!!!
  // we are using (int)x+1 in the interpolation process, so we need 
  // x <= (sizx - 1). 
  // x = (sizx - 1) works because alpha is 0 and anything 
  // trying to read x + 1 will be multiplied with 0 and should not affect the 
  // final result. 
  // !! However, if the value read from position x+1 is NAN, the 
  // final result will be NAN !!!
  // So accessing position sizx should be avoided !
  // This observation should be applied to all coordinates
  // !!!!!!
  
  forceInt.xd = 0.00;
  forceInt.yd = 0.00;
  forceInt.zd = 0.00;
  
  if((x > (sizx - 1)) || (x < 0) || 
     (y > (sizy - 1)) || (y < 0) || 
     (z > (sizz - 1)) || (z < 0))
  {
    printf("Cannot interpolate here: (%lf, %lf, %lf) !\n",
	   x, y, z);
#ifdef _DEBUG
    exit(1);
#endif
    return forceInt;
  }
  
  slsz=sizy*sizx;
  
  int ix, iy, iz;
  ix = int(x);  iy = int(y);  iz = int(z);

  alpha = x - ix;
  beta  = y - iy;
  gamma = z - iz;
  
  int ii, jj, kk, idx;
  
  for(kk=0; kk < 2; kk++) {
    for(jj=0; jj < 2; jj++) {
      for(ii=0; ii < 2; ii++) {
	
	idx = (iz+kk)*slsz + (iy+jj)*sizx  + (ix+ii);
	
	

	factor = 
	  ((1 - alpha) - ii*(1 - 2*alpha))*
	  ((1 - beta)  - jj*(1 - 2*beta))*
	  ((1 - gamma) - kk*(1 - 2*gamma));
	
	/*
	  multiplication by:
	  1-alpha when using int(x)       (i.e. ii = 0)
	  alpha   when using int(x) + 1   (i.e. ii = 1)
	  same for all coordinates
	*/
	
	if(factor != 0) {
	  // if position idx is outside the object, 
	  // and the force is 0, try to evaluate the force
	  // by averaging known neighbors (neighbors on the object)
	  if(flags[idx] == EXTERIOR) {
	    if((forcevec[idx].xd == 0.00) && (forcevec[idx].yd == 0.00) && 
	       (forcevec[idx].zd == 0.00))
	    {
	      if(!EvaluateOutsideForceVector(ix+ii, iy+jj, iz+kk, 
					     forcevec, sizx, sizy, sizz, 
					     flags, forcevec+idx))
	      {
		// I was not able to evaluate the force - maybe this voxel
		// has no object neighbors, 
		// interpolation will fail - return a force = O
		printf("WARNING: interpolation(...): attempt to evaluate force at an EXTERIOR voxel (%lf, %lf, %lf) failed !\n\
Try to increase the resolution of your dataset.\n", x, y, z);
		forceInt.xd = 0.00;
		forceInt.yd = 0.00;
		forceInt.zd = 0.00;
		ii=3; jj=3; kk=3;
		break;
	      }
	    }
	  }

	  // now do the interpolation
	  forceInt.xd = forceInt.xd + (forcevec[idx].xd * factor);
	  forceInt.yd = forceInt.yd + (forcevec[idx].yd * factor);
	  forceInt.zd = forceInt.zd + (forcevec[idx].zd * factor);
	}
      }
    }
  }
  //printf("here\n");
#ifdef _DEBUG
  // check that force components are valid numbers
  if(((forceInt.xd != 0) && (!(forceInt.xd > 0)) && (!(forceInt.xd < 0))) ||
     ((forceInt.yd != 0) && (!(forceInt.yd > 0)) && (!(forceInt.yd < 0))) ||
     ((forceInt.zd != 0) && (!(forceInt.zd > 0)) && (!(forceInt.zd < 0))))
  {
    printf("ERROR - result of interpolation is not a number: (%.16lf, %.16lf, %.16lf) at (%lf, %lf, %lf)\n\
Forces:\n", 
	   forceInt.xd, forceInt.yd, forceInt.zd,
	   x, y, z);
    
    for(kk=0; kk < 2; kk++) {
      for(jj=0; jj < 2; jj++) {
	for(ii=0; ii < 2; ii++) {
	  
	  idx = (iz+kk)*slsz + (iy+jj)*sizx  + (ix+ii);
	  
	  factor = 
	    ((1 - alpha) - ii*(1 - 2*alpha))*
	    ((1 - beta)  - jj*(1 - 2*beta))*
	    ((1 - gamma) - kk*(1 - 2*gamma));
	    
	  if(factor != 0) {
	    printf("%.16lf  %.16lf  %.16lf\n", 
		   forcevec[idx].xd, forcevec[idx].yd, forcevec[idx].zd);
	  }
	}
      }
    }  
    
    printf("Abort.\n");
    
    exit(1);
  }
#endif

  
  return(forceInt);
}




// interpolation in the float distance field
inline float interpolation(
  double x, double y, double z, int sizx, int sizy, int sizz, 
  float *df)
{
  double alpha, beta, gamma, factor;
  float res;
  long slsz = sizx * sizy;

  // !!!!!!
  // we are using (int)x+1 in the interpolation process, so we need 
  // x <= (sizx - 1). 
  // x = (sizx - 1) works because alpha is 0 and anything 
  // trying to read x + 1 will be multiplied with 0 and should not affect the 
  // final result. 
  // !! However, if the value read from position x+1 is NAN, the 
  // final result will be NAN !!!
  // So accessing position sizx should be avoided !
  // This observation should be applied to all coordinates
  // !!!!!!

  if((x > (sizx - 1)) || (x < 0) || 
     (y > (sizy - 1)) || (y < 0) || 
     (z > (sizz - 1)) || (z < 0)) 
  {
    printf("Cannot interpolate here ! (%lf, %lf, %lf). Abort.\n",
	   x, y, z);
    exit(1);
    
    return 0.00f;
  }

  int ix, iy, iz;
  ix = int(x);  iy = int(y);  iz = int(z);

  alpha = x - ix;
  beta  = y - iy;
  gamma = z - iz;

  int ii, jj, kk, idx;
  
  res = 0.00f;
  
  for(kk=0; kk < 2; kk++) {
    for(jj=0; jj < 2; jj++) {
      for(ii=0; ii < 2; ii++) {
	
	idx = (iz+kk)*slsz + (iy+jj)*sizx  + (ix+ii);

	factor = 
	  ((1 - alpha) - ii*(1 - 2*alpha))*
	  ((1 - beta)  - jj*(1 - 2*beta))*
	  ((1 - gamma) - kk*(1 - 2*gamma));
	
	/*
	  multiplication by:
	  1-alpha when using int(x)       (i.e. ii = 0)
	  alpha   when using int(x) + 1   (i.e. ii = 1)
	  same for all coordinates
	*/
	
	if(factor != 0) {
	  res = (float) (res + (df[idx] * factor));
	}
      }
    }
  }
  
  // check that res is a valid number !
#ifdef _DEBUG
  // check that force components are valid numbers
  if((res != 0) && (!(res > 0)) && (!(res < 0))) {
    printf("ERROR - result of interpolation is not a number: (%f) at (%lf, %lf, %lf)\n\
Neighbors:\n", 
	   res, x, y, z);
    
    for(kk=0; kk < 2; kk++) {
      for(jj=0; jj < 2; jj++) {
	for(ii=0; ii < 2; ii++) {
	  
	  idx = (iz+kk)*slsz + (iy+jj)*sizx  + (ix+ii);
	  
	  factor = 
	    ((1 - alpha) - ii*(1 - 2*alpha))*
	    ((1 - beta)  - jj*(1 - 2*beta))*
	    ((1 - gamma) - kk*(1 - 2*gamma));
	  
	  if(factor != 0) {
	    printf("%f\n", df[idx]);
	  }
	}
      }
    }  
    
    printf("Abort.\n");
    
    exit(1);
  }
#endif
  

  return res;
}




///////////////////////////////////////////////////////////////////////////////
// Normalize a vector
///////////////////////////////////////////////////////////////////////////////
inline void Normalize(ForceVector *fv) {
  double len;
  len = sqrt((fv->xd * fv->xd) + 
	     (fv->yd * fv->yd) + 
	     (fv->zd * fv->zd));
  
  if(len > 0.00) {
    fv->xd = fv->xd / len;
    fv->yd = fv->yd / len;
    fv->zd = fv->zd / len;
  }
  else {
    fv->xd = 0.00;
    fv->yd = 0.00;
    fv->zd = 0.00;
  }
  return;
}


/*
///////////////////////////////////////////////////////////////////////////////
// Reduce the number of segments in a skeleton
///////////////////////////////////////////////////////////////////////////////
bool ReduceNumberOfSkeletonSegments(Skeleton *skel);
*/



// bool RemoveDisconnectedSegments(Skeleton *skel);



#endif // NCD_SKEL_COMMON_DEFINED

