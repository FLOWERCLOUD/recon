// Form streamlines from critical points and seed points
//
// --- Author: Xiaosong Yuan, Bala, Nicu D. Cornea - Vizlab, Rutgers University
// --- Date: 9/28/2002
// --- Original file name: skel_streamline_float.cpp
//
// --- Changed to StreamLn.cpp by Nicu D. Cornea, on Tuesday, July 22, 2003
//
//


// #define TRACE

#include "streamLn.h"
#include "skeleton.h"
#include "critPts.h"

#define MAX_STEP_SIZE               0.4  // maximum size of the step
#define MIN_STEP_SIZE               0.05 // minimum size of the step

   // used to advance to the next position when following the vector field
   // 0.2 seems to be the best value.

#define PROGRESS_CHECK_INTERVAL  1000   // at each ... th step, we check 
   // whether we made same progress in the last ... steps (some more points
   // were added to the skeleton. If yes, we go on for another ... steps.
   // If not, we stop.
#define MAX_NUM_STEP_INTERVALS   50    // once we made 
// MAX_NUM_STEP_INTERVALS * PROGRESS_CHECK_INTERVAL, we should stop
// following the vector field. We are probably stuck near an attacting node
//

#define MAX_NUM_STEPS           6000 
// this is the maximum number of steps we can make when tracing a single 
// segment.

#define SP_SMALL_DISTANCE	0.5		// defines "close to ..."
	// in terms of Manhattan distance |x1-x2| + |y1-y2| + |z1-z2|
	//

#define HD_SMALL_DISTANCE	2.00	// defines how close a high divergence
        // point should be
	// to the skeleton, for it to be ignored

#define SKEL_SEGMENTS_INC      5


#define  MIN_SEG_LENGTH  5  // minimum segment length (number of points)
   // only segments containing at least that many points will be included 
   //   in the skeleton. However, if the segment originated in a critical 
   //   point, we should keep it regardless of the length. Removing it, 
   //   might disconnect the basic skeleton.


#define COS_OF_MAX_ANGLE -0.999997695 // cos(~180 deg)

// if the angle between the previous and the current directions is larger than
// this angle (cos is smaller), half the step size
#define STEP_HALF_ANGLE 0.342020143  // cos(70 deg)

// if the angle between the prev. and the current directions is less tha this 
// angle (cos is larger than...), double the step size
#define STEP_DOUBLE_ANGLE 0.984807753  // cos(10 deg)


#define SP_CLOSE_TO_SEGMENT_ENDPOINT  5  // defines the maximum number of 
   // skeleton points that can separate an intersection with a 
   // skeleton segment from the segment's end points so that the 
   // intersection is not reported. 
   // When constructing a skeleton segment, at each step we test for
   // intersection with the other existing segments. If we are close to 
   // another skeleton segment, we also check if we are also close to
   // one of that segment's end points. If we are, we keep going even if
   // the points in the 2 segments overlap, in the hope that we can end the 
   // current segment at the same point as the segment we are intersecting,
   // thus reducing the number of joints. Of course this does not work if
   // we are close to an end point but we are actualy moving towards the
   // other end of the segment, but in that case, the current segment will
   // be terminated once we get far enough from the end point.


typedef enum{
  FSL_EXS_ERROR = 0,
  FSL_EXS_TOO_MANY_STEPS,
  FSL_EXS_CLOSE_TO_SKEL,
  FSL_EXS_CLOSE_TO_CP,
  FSL_EXS_STUCK, 
  FSL_EXS_OUTSIDE_BBOX,
  FSL_EXS_OUTSIDE_VOLUME
} 
FSL_ExitStatus;


inline void rk2(double x, double y, double z, int sizx, int sizy, int sizz,
		double steps,
		ForceVector *Force_ini, 
		unsigned char *flags,
		VoxelPositionDouble *nextPos);

inline void rk4(double x, double y, double z, int sizx, int sizy, int sizz,
		double steps,
		ForceVector *Force_ini, 
		unsigned char *flags,
		VoxelPositionDouble *nextPos);

bool FollowStreamlines(
  VoxelPositionDouble &startPt,  // [in] start position
  ForceVector *whereTo,	         // [in] initial direction
  int sX, int sY, int sZ,        // [in] size of dataset
  ForceVector *ForceField,       // [in/out] force vectors
  unsigned char *flags,          // [in] volume flags
  DynamicArray<CriticalPoint> &CritPts, // [in/out] critical points
  //int *critPtInSkeleton,
  Skeleton *Skel,                // [in/out] skeleton
  float *distField,              // [in] distance field
  int originCP,                  // [in] id of critical point if startPos is a 
                                 // critical point. Otherwise, set this to -1
  bool lookInCP                  // [in] specifies whether we should check 
                                 // if we are close to a critical point during 
                                 // tracing. Set this to true for the level 1
                                 // skeleton and to false for level 2
);


inline bool CloseToCP(VoxelPositionDouble &pos, int	originCP, 
		      DynamicArray<CriticalPoint> &CritPts, double maxDist,
		      int *closestCP);

inline bool CloseToSkel( VoxelPositionDouble &pos,
			 Skeleton *Skel, int crtSegmentLength, 
			 double maxDist,
			 int *closestSeg, int *closestPoint
			 );


bool FindCriticalPoint(VoxelPositionDouble &Startpos, 
		       VoxelPositionDouble &Nextpos, 
		       ForceVector *ForceField,
		       unsigned char *flags, 
		       int sX, int sY, int sZ,
		       CriticalPoint *critPt);

//////////////////////////////////////////////////////////////////////////////
// Get basic skeleton (level 1): connects only critical points
//////////////////////////////////////////////////////////////////////////////
bool GetLevel1Skeleton(
        ForceVector *ForceField, 		// [in] vector field
	unsigned char *flags,            // [in] volume flags
	int L, int M, int N,		// [in] vector field size (X, Y and Z)
	DynamicArray<CriticalPoint> &CritPts,  // [in] critical points array
	Skeleton *Skel,                 // [out] skeleton
	float *distField                // [in] distance field - may be NULL
) {

#ifdef TRACE
  printf("TRACE: Starting GetLevel1Skeleton function...\n");
#endif

  long slsz;
  int i,j;
  slsz = L*M;		// slice size

  ForceVector whereTo;

  if(CritPts.GetNrElem() < 1) {
    return true;
  }

  
#ifdef TRACE
  printf("Critical points:\n");
  for(i=0; i < CritPts.GetNrElem(); i++) {
    printf("%d: (%f %f %f). type: %d\n", i, CritPts[i].position.x,
	   CritPts[i].position.y, CritPts[i].position.z, CritPts[i].type);
  }
#endif

  //
  // follow the streamlines starting at saddles in the direction of the
  // positive eigenvector(s)
  //	until we are close to one of the skeleton points or a critical point,
  //    ignoring the points in the current segment.
  //

  for(i = 0; i < CritPts.GetNrElem(); i++) {
    // start to follow force field at saddle points only
    if(CritPts[i].type != CPT_SADDLE) {
      // printf("Critical point %d not a saddle\n", i);
      continue;
    }

    printf("L1: Processed %d critical points out of %d.\r", i, 
	   CritPts.GetNrElem());
    fflush(stdout);
    
#ifdef TRACE
    int idx = (int)CritPts[i].position.z * slsz + (int)CritPts[i].position.y *L
      + (int)CritPts[i].position.x;
    printf("Critical Point: (%f, %f, %f): idx = %d, force here: (%f, %f, %f)\n",
	   CritPts[i].position.x, CritPts[i].position.y, CritPts[i].position.z,
	   idx,
	   ForceField[idx].xd, ForceField[idx].yd, ForceField[idx].zd);
#endif
    
    
#ifdef TRACE
    printf("Starting to follow the force field...\n");
#endif

    // the point is a saddle, so we should go in the direction pointed by
    // the pozitive eigenvectors (i.e. eigenvectors corresponding to pozitive
    // eigenvalues)
    for(j=0; j < 3; j++) {
      if(CritPts[i].eval[j] > 0) {
	
	// direction given by the eigenvector
	whereTo.xd = CritPts[i].evect[j].xd;
	whereTo.yd = CritPts[i].evect[j].yd;
	whereTo.zd = CritPts[i].evect[j].zd;
	Normalize(&whereTo);
	
	FollowStreamlines(
	  CritPts[i].position,
	  &whereTo,
	  L, M, N, ForceField, flags,
	  CritPts,
	  //critPtInSkeleton,
	  Skel, distField,
	  i, true);

	// the exact opposite direction of the eigenvector
	whereTo.xd = - CritPts[i].evect[j].xd;
	whereTo.yd = - CritPts[i].evect[j].yd;
	whereTo.zd = - CritPts[i].evect[j].zd;
	Normalize(&whereTo);
	
	FollowStreamlines(
	  CritPts[i].position,
	  &whereTo,
	  L, M, N, ForceField, flags,
	  CritPts,
	  //critPtInSkeleton,
	  Skel, distField,
	  i, true);
	
      }
    }

  }

  printf("Done.\n");
#ifdef _DEBUG
  PrintElapsedTime("\tSL-2: Following streamlines starting at the critical points.");
#endif

  //delete [] critPtInSkeleton;

  return true;
}


bool GetStreamLines(
        ForceVector *ForceField, unsigned char *flags, int L, int M, int N,
	DynamicArray<CriticalPoint> &CritPts,
	DynamicArray<VoxelPositionDouble> &BdSeeds,
	DynamicArray<DivergencePoint> &HDPoints,
        Skeleton *Skel, 
	float *distField)
{

#ifdef TRACE
  printf("TRACE: Starting GetStreamLines function...\n");
#endif

  if(Skel == NULL) {
    printf("Argument 1 (*Skel) to GetStreamlines cannot be NULL! Abort.\n");
    exit(1);
  }
  
  GetLevel1Skeleton( ForceField, flags, L, M, N,
		     CritPts,
		     Skel, distField);
  
  GetLevel2Skeleton( ForceField, flags, L, M, N,
		     HDPoints,
		     Skel, distField);

  return true;
}




inline void rk2(double x, double y, double z, int sizx, int sizy, int sizz,
		double steps, ForceVector *Force_ini, 
		unsigned char *flags,
		VoxelPositionDouble *nextPos)
{
  ForceVector startForce, midForce;
  
  // interpolate to find the direction at the begining of the interval
  startForce=interpolation(x,y,z,sizx,sizy,sizz,Force_ini, flags);
  // normalize 
  Normalize(&startForce);
  
  // find the force at the middle of the interval
  midForce = interpolation(x + (startForce.xd * steps / 2.00),
			   y + (startForce.yd * steps / 2.00),
			   z + (startForce.zd * steps / 2.00),
			   sizx, sizy, sizz, Force_ini, flags);
  // normalize
  Normalize(&midForce);

  // use force at middle of interval for entire interval
  
  nextPos->x = x + (midForce.xd * steps);
  nextPos->y = y + (midForce.yd * steps);
  nextPos->z = z + (midForce.zd * steps);
  
  
#ifdef TRACE
  printf("Next position: (%lf, %lf, %lf) force here: (%lf, %lf, %lf)\n",
	 nextPos->x, nextPos->y, nextPos->z,
	 midForce.xd, midForce.yd, midForce.zd);
  
#endif

  return;
}



inline void rk4(double x, double y, double z, int sizx, int sizy, int sizz,
		double steps, ForceVector *Force_ini, 
		unsigned char *flags,
		VoxelPositionDouble *nextPos)
{
  ForceVector startForce, midForce1, midForce2, endForce, finalDirection;
  
  // step 1
  // interpolate to find the direction at the begining of the interval
  startForce=interpolation(x,y,z,sizx,sizy,sizz,Force_ini, flags);
  // normalize 
  Normalize(&startForce);
  
  // step 2
  // find the force at the middle of the interval using startForce to
  // figure out where the middle is
  midForce1 = interpolation(x + (startForce.xd * steps / 2.00),
			   y + (startForce.yd * steps / 2.00),
			   z + (startForce.zd * steps / 2.00),
			   sizx, sizy, sizz, Force_ini, flags);
  // normalize
  Normalize(&midForce1);

  // step 3
  // find the force at the middle of the interval again using midForce1 to
  // figure out where the middle is
  midForce2 = interpolation(x + (midForce1.xd * steps / 2.00),
			    y + (midForce1.yd * steps / 2.00),
			    z + (midForce1.zd * steps / 2.00),
			    sizx, sizy, sizz, Force_ini, flags);
  // normalize
  Normalize(&midForce2);


  // step 4
  // find the force at the end of the interval using midForce2 to figure out 
  // where the end of the interval is
  endForce = interpolation(x + (midForce2.xd * steps),
			   y + (midForce2.yd * steps),
			   z + (midForce2.zd * steps),
			   sizx, sizy, sizz, Force_ini, flags);
  // normalize
  Normalize(&endForce);


  // final direction is determined using a weighted sum of the 4 forces 
  // computed in the previous steps. weights: 1/6, 2/6, 2/6, 1/6

  finalDirection.xd = 
    (startForce.xd + (2 * midForce1.xd) + (2 * midForce2.xd) + endForce.xd) 
    / 6.00;
  finalDirection.yd =  
    (startForce.yd + (2 * midForce1.yd) + (2 * midForce2.yd) + endForce.yd) 
    / 6.00;
  finalDirection.zd =  
    (startForce.zd + (2 * midForce1.zd) + (2 * midForce2.zd) + endForce.zd) 
    / 6.00;

  Normalize(&finalDirection);
  
  nextPos->x = x + finalDirection.xd * steps;
  nextPos->y = y + finalDirection.yd * steps;
  nextPos->z = z + finalDirection.zd * steps;

  
#ifdef TRACE
  printf("Next position: (%f, %f, %f) force here: (%f, %f, %f)\n",
	 nextPos->x, nextPos->y, nextPos->z,
	 endForce.xd, endForce.yd, endForce.zd);
  
#endif

  return;
}



//
// function FollowStreamlines
// creates an entire skeleton segment
//
bool FollowStreamlines(
  VoxelPositionDouble &startPt,  // [in] start position
  ForceVector *whereTo,	         // [in] initial direction
  int sX, int sY, int sZ,        // [in] size of dataset
  ForceVector *ForceField,       // [in/out] force vectors
  unsigned char *flags,          // [in] volume flags
  DynamicArray<CriticalPoint> &CritPts, // [in/out] critical points
  //int *critPtInSkeleton,
  Skeleton *Skel,                // [in/out] skeleton
  float *distField,              // [in] distance field
  int originCP,                  // [in] id of critical point if startPos is a 
                                 // critical point. Otherwise, set this to -1
  bool lookInCP                  // [in] specifies whether we should check 
                                 // if we are close to a critical point during 
                                 // tracing. Set this to true for the level 1
                                 // skeleton and to false for level 2
  )
{

#ifdef TRACE
  printf("Starting FollowStreamlines function\n\
\toriginCP = %d\n", originCP);
#endif

  VoxelPositionDouble Startpos, Nextpos;
  //int nrAlreadyInSkel;
  int step;
  bool stop;
  bool endpoint;
  int crtSegLength = 0;
  int saveCrtSegLen;
  float dt;
  
  FSL_ExitStatus exit_status = FSL_EXS_ERROR;
  int exit_data1 = -1;
  int exit_data2 = -1;
  bool first_iteration = true;
  int idx;
  int nidx;
  int nix, niy, niz;
  
  int closestSeg, closestPoint, closestCP;
  float stepSize = MIN_STEP_SIZE;
  double angleCos, aCos2;

  SkeletonSegment skelSeg; // skeleton segment
  
  // we keep track of previous and current direction of the forces so that we
  // can figure out when we get stuck near an undetected critical point
  ForceVector prevDir, crtDir, otherCPPosVector;
  

  // stores previous direction
  prevDir.xd = 0.00;
  prevDir.yd = 0.00;
  prevDir.zd = 0.00;

  // stores current direction
  crtDir.xd = 0.00;
  crtDir.yd = 0.00;
  crtDir.zd = 0.00;
  
  
  Startpos.x = startPt.x;
  Startpos.y = startPt.y;
  Startpos.z = startPt.z;
  
  // oCPSkelPos = -1;  // the position of the original critical point
                    // in the Skeleton array if it was already there
                    // at the start of this function

  
  
  exit_status = FSL_EXS_ERROR;

  first_iteration = true;
  step = 0;
  stop = false;
  saveCrtSegLen = crtSegLength;
  
  
  while(!stop)   {
    
    step++;
    
    // at every step, we are moving for sure - no need to check
    // we will check later (once we compute the next position) 
    // if the direction we are moving in is almost opposite
    // to the previous direction. That will tell us we are stuck.
#ifdef TRACE
    printf("Step %d.", step);
#endif

    // if we made more than MAX_NUM_STEPS steps we should stop
    if(step > MAX_NUM_STEPS) {
      printf("Too many steps when tracing segment %d.\n", 
	     Skel->GetNumberOfSegments());
      stop = true;
      exit_status = FSL_EXS_TOO_MANY_STEPS;
      break;
    }

    //
    // add Startpos to the current skeleton segment 
    //
    dt = 1.0;
    if(distField != NULL) {
      dt = interpolation(Startpos.x, Startpos.y, Startpos.z, sX, sY, sZ, 
			 distField);
      // printf("interpolated dt: %f\n", dt);
    }
    skelSeg.InsertPoint(Startpos, dt);
    
#ifdef TRACE
    printf("Current position is: (%lf, %lf, %lf)\n",
	   Startpos.x, Startpos.y, Startpos.z);
#endif
    
    // if Startpos is on the bounding box or outside, remove this segment
    if( (Startpos.x <= 0) || (Startpos.x >= (sX-1)) || 
	(Startpos.y <= 0) || (Startpos.y >= (sY-1)) || 
	(Startpos.z <= 0) || (Startpos.z >= (sZ-1))) 
    {
      exit_status = FSL_EXS_OUTSIDE_BBOX;
      printf("segment %d goes outside bounding box\n", 
	     Skel->GetNumberOfSegments());
      stop = true;
      break;
    }


    //
    // get segment length so far
    //
    crtSegLength = skelSeg.GetNumberOfPoints();

    //    
    // if this point is close to another point that is already part of the
    //   skeleton, we should stop
    // But not if the start position was a critical point. In that case, we 
    // only care if we are close to a critical point
   
#ifdef TRACE
    printf("Checking if close to a skeleton point...\n");    
#endif 

    // 
    // if we are starting from a critical point
    //   don't check if we are close to a skeleton point
    //
    if(originCP != -1) {
      // don't check 
#ifdef TRACE
      printf("Segment starting from a critical point. Will not check if close to a skeleton point.\n");
#endif      
    }
    else {
      // check if we are close to a skeleton point
      if(CloseToSkel(Startpos, Skel, skelSeg.GetNumberOfPoints(), 
		     stepSize*stepSize, 
		     &closestSeg, &closestPoint))
      {

#ifdef TRACE
	printf("Position is close to a skeleton point (%d, %d). Break\n", 
	       closestSeg, closestPoint);
#endif
	exit_status = FSL_EXS_CLOSE_TO_SKEL;
	exit_data1 = closestSeg;
	exit_data2 = closestPoint;
	stop = true;
	break;
      }
    }
    
    //
    // check if we are close to a critical point (if lookInCP is true)
    //

#ifdef TRACE
    printf("Checking if close to a critical point...\n");
#endif

    if(lookInCP) {
      if(CloseToCP( Startpos, originCP, CritPts, stepSize*stepSize, 
		    &closestCP)) 
      {
	// if we are close to another critical point and this is the first 
	// iteration and we are moving away from that critical point
	// then ignore it.
	// It means the two critical points were very close to each other
	// to begin with but we are moving away from them...
	// the whereTo vector should tell us if we are moving away from that
	// other critical point or not
	// we are moving away if the whereTo vector and the vector to the 
	// other critical point form an angle of more than 90 deg (cos < 0)
	
	// get position vector of the other critical point relative to 
	// the current position
	otherCPPosVector.xd = CritPts[closestCP].position.x - Startpos.x;
	otherCPPosVector.yd = CritPts[closestCP].position.y - Startpos.y;
	otherCPPosVector.zd = CritPts[closestCP].position.z - Startpos.z;
	// normalize it
	Normalize(&otherCPPosVector);
	
	// whereTo should be normalized already and we can compute the angle 
	// between them
	aCos2 = (otherCPPosVector.xd * whereTo->xd) + 
	  (otherCPPosVector.yd * whereTo->yd) + 
	  (otherCPPosVector.zd * whereTo->zd);
	
	if(!first_iteration || (aCos2 > 0)) {
#ifdef TRACE
	  printf("Position is close to a critical point (index = %d). Break\n"
		 , closestCP);
#endif
	  exit_status = FSL_EXS_CLOSE_TO_CP;
	  exit_data1 = closestCP;
	  exit_data2 = -1;
	  stop = true;
	  break;
	}
	else {
#ifdef TRACE
	  printf("Position is close to a critical point (index = %d), but this is the first iteration and we are moving away from it.\n"
		 , closestCP);
#endif 
	}
      }
    }
    
    // move to the next position
    // for the first iteration, the next position is given by the 
    // whereTo vector, if it's not NULL. If it is NULL, then we take
    // the force field value at the start position
    //
#ifdef TRACE
    printf("Moving to the next position...\n");
#endif 
    
    if(first_iteration &&  (whereTo != NULL)) {
      first_iteration = false;
      
      Nextpos.x = Startpos.x + (whereTo->xd * stepSize);
      Nextpos.y = Startpos.y + (whereTo->yd * stepSize);
      Nextpos.z = Startpos.z + (whereTo->zd * stepSize);
    }
    else {
      // for the subsequent iterations (or if whereTo is NULL), we use the 
      // vector field value at the current position
      rk4( Startpos.x, Startpos.y, Startpos.z, sX, sY, sZ, stepSize,
	   ForceField, flags, &Nextpos);
    }
    

    // -- DEBUG
    // current movement direction
    // save prev direction
    prevDir = crtDir;
    crtDir.xd = Nextpos.x - Startpos.x;
    crtDir.yd = Nextpos.y - Startpos.y;
    crtDir.zd = Nextpos.z - Startpos.z;
    Normalize(&crtDir);

    // compute cos of angle between previous and current directions
    angleCos = (crtDir.xd * prevDir.xd) + (crtDir.yd * prevDir.yd) + 
      (crtDir.zd * prevDir.zd);
    
    
    // look at the angle between the current and previous movement directions
    // if the cos(angle) is less than  COS_OF_MAX_ANGLE, we are going back 
    // This will happen if we reached an undetected critical point or 
    // a point that is almost a critical point but not exactly
    // and we should stop  
    //     
    if(angleCos <  COS_OF_MAX_ANGLE)  {
      // we are moving back and forth
#ifdef TRACE
      printf("pos: (%lf, %lf, %lf), prevDir:(%lf, %lf, %lf), crtDir:(%lf, %lf, %lf)\n\
There should be a critical point here. Let's look for it ...\n",
	     Startpos.x, Startpos.y, Startpos.z, 
	     prevDir.xd, prevDir.yd, prevDir.zd, 
	     crtDir.xd, crtDir.yd, crtDir.zd);
#endif
      
      
      // here we should try to detect the critical point and add it to the 
      // array of critical points if we find it
      // if we cannot find a critical point here, we just assume there is a
      // critical point just between the Startpos and the Nextpos
      // Anyway, we cannot continue tracing under these conditions because 
      // we will be chasing our own tail
      CriticalPoint critPt;

      if(!FindCriticalPoint(Startpos, Nextpos, ForceField, flags, 
			    sX, sY, sZ, &critPt))
      {
	//  I could not find a critical point - let's just say that there is a 
	// critical point just between Startpos and Nextpos even if there isn't
	critPt.position.x = (Startpos.x + Nextpos.x) / 2.00;
	critPt.position.y = (Startpos.y + Nextpos.y) / 2.00;
	critPt.position.z = (Startpos.z + Nextpos.z) / 2.00;
	critPt.type = CPT_UNKNOWN;
	
	// try to classify it...
	if(!ClassifyCriticalPoint(critPt, ForceField, flags, sX, sY, sZ)) {
	  // I cannot classify it
	  printf("\
WARNING: Unable to classify pseudo-critical point at (%lf, %lf, %lf) !\n" , 
		 critPt.position.x, critPt.position.y, critPt.position.z);
#ifdef TRACE
	  printf("I am stuck !! Could not find a critical point !!!\n");
#endif
	  exit_status = FSL_EXS_STUCK;
	  stop = true;
	  break;
	}
	else {
#ifdef TRACE
	  printf("Ending current segment at pseudo-critical point.\n");
#endif
	}
      }
   
      // a critical point was found
      // or a pseudo-critical point was assumed

      // if the critical point is not a saddle - the skeleton may be 
      // disconnected in the end
      if(critPt.type != CPT_SADDLE) {
	printf("WARNING: skeleton segment was terminated at a non saddle critical point.\nThe skeleton may be disconnected.\n");
      }
      
      // if the critical point we just found is just the one we started from 
      // ignore it !!, otherwise we will get into an infinite loop
      // we know it's not another critical point because if we were close
      // to another CP we wouldn't be at this point in the code
      if(originCP != -1) {
	if(IS_ZERO(critPt.position.x - CritPts[originCP].position.x) &&
	   IS_ZERO(critPt.position.y - CritPts[originCP].position.y) &&
	   IS_ZERO(critPt.position.z - CritPts[originCP].position.z))
	{
#ifdef TRACE
	  printf("The critical point found is exactly the one we started from - ognored.\n");
#endif
	  printf("\
WARNING: Unable to move away from critical point (%lf, %lf, %lf) !\nSkeleton may be disconnected !\n" , 
		 critPt.position.x, critPt.position.y, critPt.position.z);
#ifdef TRACE
	  printf("I am stuck at a saddle point ?!\n");
#endif
	  exit_status = FSL_EXS_STUCK;
	  stop = true;
	  break;
	}
      }
      
      //      
      // add the critical point to the list of critical points
      if(CritPts.Append(critPt)) {
	// then end the current segment at this critical point
	exit_status = FSL_EXS_CLOSE_TO_CP;
	exit_data1 = CritPts.GetNrElem() - 1;
	exit_data2 = -1;
	stop = true;
#ifdef TRACE
	printf("Ending current segment at critical point %d.\n", exit_data1);
#endif
	break;
      }
    }
      
    
    //
    // if Nextpos = Startpos we are stuck and we should stop
    // This should never happen !!
    if(	EQUAL(Nextpos.x, Startpos.x)	&&
	EQUAL(Nextpos.y, Startpos.y)	&&
	EQUAL(Nextpos.z, Startpos.z))
      {
#ifdef TRACE
      printf("New position is the same as start position ! We are stuck !\n");
#endif
      
      exit_status = FSL_EXS_STUCK;
      stop = true;
      break;
    }

#ifdef TRACE    
    // distance between nextpos and startpos should be exactly stepSize
    printf("distance moved: %lf. Direction: (%lf, %lf, %lf)\n", 
	   sqrt((Startpos.x - Nextpos.x)*(Startpos.x - Nextpos.x) + 
		(Startpos.y - Nextpos.y)*(Startpos.y - Nextpos.y) + 
		(Startpos.z - Nextpos.z)*(Startpos.z - Nextpos.z)),
	   crtDir.xd, crtDir.yd, crtDir.zd);
#endif

    // if the angle between the current direction and the previous one is less
    // than STEP_DOUBLE_ANGLE (cos is larger than .. ), double the stepSize
    if(angleCos > STEP_DOUBLE_ANGLE) {
      stepSize = stepSize * 2.0;
      if(stepSize > MAX_STEP_SIZE) stepSize = MAX_STEP_SIZE;
    }
    else {
      // if the angle between the previous and the current directions is more
      // than STEP_HALF_ANGLE (cos is smaller than ...), half the step size
      if(angleCos < STEP_HALF_ANGLE) {
	stepSize = stepSize / 2.0;
	if(stepSize < MIN_STEP_SIZE) stepSize = MIN_STEP_SIZE;
      }
    }


    //
    // if the next positio is outside the volume, stop
    //
    nix = (int) round(Nextpos.x);
    niy = (int) round(Nextpos.y);
    niz = (int) round(Nextpos.z);
    if((nix < 0) || (nix >= sX) || (niy < 0) || (niy >= sY) || 
       (niz < 0) || (niz >= sZ))
    {
      exit_status = FSL_EXS_OUTSIDE_VOLUME;
      stop = true;
      break;
    }
    else {
      nidx = ((int)round(Nextpos.z)*sX*sY) + ((int)round(Nextpos.y)*sX) + 
	(int)round(Nextpos.x);
      if(flags[nidx] == EXTERIOR) {
	exit_status = FSL_EXS_OUTSIDE_VOLUME;
	stop = true;
	break;
      }
    }

    //
    // next position becomes current position
    //
    Startpos = Nextpos;
    first_iteration = false;
    
  } // end while loop
  
  
  // usually, if the current segment is not long enough 
  // we are not going to keep it.
  // however, if the segment originated in a critical point, we should 
  // keep it regardless of the length. Removing it, might disconnect the 
  // skeleton.

  //
  // check segment length only if not originated at a critical point
  //
  if(originCP == -1) {
    if(skelSeg.GetNumberOfPoints() < MIN_SEG_LENGTH) {
      // the segment is not long enough and it will not be added to the skel
      // #ifdef _DEBUG
      printf("Current segment is not long enough.\n");
      // #endif
      
      return true;
    }
  }
  
  //
  // we are going to keep the segment, let;s finish the job
  //
  switch(exit_status) {
    
  case FSL_EXS_TOO_MANY_STEPS:
    // nothing to do :)
    idx = (int)Startpos.z * sX*sY + (int)Startpos.y *sX
      + (int)Startpos.x;
    printf("WARNING: Too many steps performed during tracing.\n");
    printf("Current position is: (%f, %f, %f). Force here: (%lf, %lf, %lf)\n",
	   Startpos.x, Startpos.y, Startpos.z,
	   ForceField[idx].xd, ForceField[idx].yd, ForceField[idx].zd);
    printf("Skeleton will be disconnected !!\n");
    
    // do not terminate program - just print the warning
    break;
    
  case FSL_EXS_CLOSE_TO_SKEL:
    // we are close to a skeleton point, belonging to another segment.
    
    // end the current segment at the intersection point
    skelSeg.InsertPoint((*Skel)[exit_data1][exit_data2]);
    
    // the segment we are close to will be split into 2 if we are not close to 
    // one of it's end points
    endpoint = false;
    if((exit_data2 == 0) || 
       (exit_data2 == (*Skel)[exit_data1].GetNumberOfPoints() - 1))
    {
      endpoint = true;
    }
    
    if(!endpoint) {
      // not close to one of the endpoints - the segment must be split 
      // create a new skeleton segment starting at the intersection point
      // and ending where the original segment ended
      SkeletonSegment newSeg;
      for(int i=exit_data2; i < (*Skel)[exit_data1].GetNumberOfPoints(); i++) {
	newSeg.InsertPoint((*Skel)[exit_data1][i]);
      }
      // remove the last ... points from the original segment
      (*Skel)[exit_data1].RemoveLastNPoints(
					    (*Skel)[exit_data1].GetNumberOfPoints() - exit_data2 - 1);
      // add the new segment to the skeleton
      Skel->InsertSegment(newSeg);
    }
    else {
      // we are close to one of the endpoints of an existing segment
      // we are happy - the original segment doesn't have to be split
    }
    break;

  case FSL_EXS_CLOSE_TO_CP:
    // we are close to a critical point

    // add the critical point to the current segment
    dt = 1.0;
    if(distField != NULL) {
      dt = interpolation(CritPts[exit_data1].position.x, 
			 CritPts[exit_data1].position.y, 
			 CritPts[exit_data1].position.z, 
			 sX, sY, sZ, distField);
      // printf("interpolated dt: %f\n", dt);
    }
    
    skelSeg.InsertPoint(CritPts[exit_data1].position, dt);

    break;
   
  case FSL_EXS_STUCK:
    // we are stuck in a place - nothing to do, just exit
    idx = (int)Startpos.z * sX*sY + (int)Startpos.y *sX
      + (int)Startpos.x;
    
    printf("WARNING: Force following algorithm is stuck !\nCurrent position is: (%f, %f, %f). Force here: (%lf, %lf, %lf)\nSkeleton may be disconnected !\n",
	   Startpos.x, Startpos.y, Startpos.z,
	   ForceField[idx].xd, ForceField[idx].yd, ForceField[idx].zd
	   );
    break;
  case FSL_EXS_OUTSIDE_BBOX:
    
    // the segment touched the bounding box
    // the segment will not be added to the skeleton

    // #ifdef _DEBUG
    printf("Current segment touched the bounding box. Removed.\n");
    // #endif
    return false;
    break;
    
  case FSL_EXS_OUTSIDE_VOLUME:
    // the segment goes outside the volume
    // terminate the segment here and keep it ???
    printf("WARNING: Current segment goes outside volume. Segment is terminated.\nSkeleton will be disconnected !\n");
    
    break;
    
  case FSL_EXS_ERROR:
    printf("I shouldn't be here !!! Abort\n");
    exit(1);
    break;
  }
  
  // I should only get here if all os ok and I can add the current segment 
  // to the skeleton

  // add the current skeleton segment to the skeleton
  Skel->InsertSegment(skelSeg);
  
#ifdef TRACE  
  printf("FollowStreamLines: done\n");
#endif

  return true;
}


inline bool CloseToSkel( VoxelPositionDouble &pos,
			 Skeleton *Skel, int crtSegmentLength, 
			 double maxDist,
			 int *closestSeg, int *closestPoint
			 )
{
  double d;
  bool endpoint;
  int dToLeft, dToRight;

  d = Skel->DistanceToPoint(pos, maxDist, closestSeg, closestPoint);
  if(d >= 0) {
    // the point is "close" to a skeleton point
    // if the skeleton point we are close to is also close to one of 
    //   the segment end points, (but it's not an end point)
    //   do not report as being close to the point, wait to get to one
    //   of the segment end points
    endpoint = false;
    if(((*closestPoint) == 0) ||
       ((*closestPoint) == ((*Skel)[(*closestSeg)].GetNumberOfPoints() - 1)))
    {
      endpoint = true;
    }
    
    if(endpoint) {
      // report the intersection if we are at an end point
      return true;
    }
    else {
      //
      // see how close we are to the endpoints of seg
      dToLeft  = (*closestPoint) - 0;
      dToRight = (*Skel)[(*closestSeg)].GetNumberOfPoints() - (*closestPoint) + 1;
      if((dToLeft  > SP_CLOSE_TO_SEGMENT_ENDPOINT) &&
	 (dToRight > SP_CLOSE_TO_SEGMENT_ENDPOINT))
      {
	// if we are NOT close to an end point, 
	//    report the intersection
	return true;
      }		
      
      // otherwise, we are close to a segment given by seg but, we
      // are also just a few points from the seg's end point and we 
      // hope to end the current segment at the same point where seg 
      // ends. However if the current segment's length is comparable
      // to SP_CLOSE_TO_SEGMENT_ENDPOINT then we should stop right 
      // here because this won't look good
      
      //  crtSegLength = Skel->Segments[crtSegment][SKEL_SEG_LAST] - 
      //  Skel->Segments[crtSegment][SKEL_SEG_FIRST];
      
      // comparable means not more than twice as long as
      //    SP_CLOSE_TO_SEGMENT_ENDPOINT
      if(crtSegmentLength <= (2 * SP_CLOSE_TO_SEGMENT_ENDPOINT)) {
	return true;
      }
      
    }
  }
  
  return false;
}


inline bool CloseToCP(VoxelPositionDouble &pos, int	originCP, 
		      DynamicArray<CriticalPoint> &CritPts, double maxDist,
		      int *closestCP)
{
  int i;
  double d, mind;
  bool found;
  
  // maxDist is the maximum distance we are interested in. Any point further 
  // away than that can be ignored.
  // To reduce the number of operations, I will compute the distance 
  // incrementally and exit as soon as d is larger than maxDist
  
  *closestCP = -1;
  
  mind = maxDist;
  found = false;
  // see if it's close to a critical point except the one specified in originCP
  for(i=0; i < CritPts.GetNrElem(); i++) {
    // ignore the critical point that started this streamline
    if(i == originCP) continue;
    
    d = ((CritPts[i].position.x - pos.x)*(CritPts[i].position.x - pos.x));
    if(d > mind) continue;
    d = d + ((CritPts[i].position.y - pos.y)*(CritPts[i].position.y - pos.y));
    if(d > mind) continue;
    d = d + ((CritPts[i].position.z - pos.z)*(CritPts[i].position.z - pos.z));
    if(d > mind) continue;

    // d is <= maxDist
    // the point is "close" to a critical point
    if(d < mind) {
      *closestCP = i;
      mind = d;
      found = true;
    }
  }
  
  return found;
}


     


//////////////////////////////////////////////////////////////////////////////
// Get level 2 skeleton: adds segments starting at interior high divergence 
//    points
//////////////////////////////////////////////////////////////////////////////
bool GetLevel2Skeleton(
        ForceVector *ForceField, 		// [in] vector field
	unsigned char *flags,                   // [in] volume flags
	int L, int M, int N,		// [in] vector field size (X, Y and Z)
	DynamicArray<DivergencePoint> &HDPoints,  // [in] high divergence points array
	Skeleton *Skel,                 // [in, out] skeleton
	                                //   in - level 1 skeleton
	                                //   out - level 2 skeleton
	float *distField                // [in] distance field - may be NULL
) {

#ifdef TRACE
  printf("TRACE: GetLevel2Skeleton function...\n");
#endif
  
  long slsz;
  int i;
  slsz = L*M;		// slice size

  
  DynamicArray<CriticalPoint> fakeCP(1); // fake array of critical points
  // required as parameter to FollowStreamLn but not used since lookInCP 
  // parameter is set to false

  //
  // follow the streamlines starting at each high divergence point
  //	until we are close to one of the already existing skeleton points 
  //
  for(i=0; i < HDPoints.GetNrElem(); i++) {
    
#ifdef _DEBUG
    // printf("Starting at high div point %d: (%f, %f, %f)...\n", i, HDPoints[i].x, HDPoints[i].y, HDPoints[i].z);
#endif

    FollowStreamlines(
		      /*VoxelPositionDouble &startPt*/ HDPoints[i].position,
		      /*ForceVector &whereTo*/         NULL,
		      /*int sX, int sY, int sZ*/       L, M, N,
		      /*ForceVector *ForceField*/      ForceField,
		      /*unsigned char *flags*/         flags,
		      /*DynamicArray<CriticalPoint> &CritPts*/ fakeCP, 
		      /*Skeleton &Skel*/               Skel,
		      /*float *distField*/             distField,
		      /*int originCP*/                 -1,
		      /*bool lookInCP*/               false);
     
  }
  
  
#ifdef _DEBUG
  PrintElapsedTime("\tSL-2: Following streamlines starting at the high divergence points.");
#endif

  //////
  return true;
}




bool FindCriticalPoint(VoxelPositionDouble &Startpos, 
		       VoxelPositionDouble &Nextpos, 
		       ForceVector *ForceField,
		       unsigned char *flags, 
		       int sX, int sY, int sZ, 
		       CriticalPoint *critPt)
{
  
  int ix, iy, iz;
  int inds[8];
  int idx, slsz;
  bool found = false;
  
  slsz = sX*sY;
  
  // look in the cell containing Startpos
  ix = int(Startpos.x); iy = int(Startpos.y); iz = int(Startpos.z);
  
  idx = iz*slsz + iy*sX +ix;
  
  // these are the neighbors that will be touched by the interpolation
  inds[0] = idx;
  inds[1] = idx + 1;
  inds[2] = idx + sX;
  inds[3] = idx + sX + 1;
  inds[4] = idx + slsz;
  inds[5] = idx + slsz + 1;
  inds[6] = idx + slsz + sX;
  inds[7] = idx + slsz + sX + 1;     
  if(FindCriticalPointInIntCell(ix, iy, iz, inds,sX, sY, sZ, 
				ForceField, flags, false,
				20, true,
				&(critPt->position)))
  {
#ifdef TRACE
    printf("Critical point found at: (%lf, %lf, %lf) ", 
	   critPt->position.x, critPt->position.y, critPt->position.z);
#endif
    found = true;
  }
  else {
#ifdef TRACE
    printf("Critical point NOT found ");
#endif
  }
#ifdef TRACE
  printf("in cell (%d, %d, %d)\n", ix, iy, iz);
#endif
  
  if(!found) {
    // look in the cell containing Nextpos
    ix = int(Nextpos.x); iy = int(Nextpos.y); iz = int(Nextpos.z);
    
    idx = iz*slsz + iy*sX +ix;
    // these are the neighbors that will be touched by the interpolation
    inds[0] = idx;
    inds[1] = idx + 1;
    inds[2] = idx + sX;
    inds[3] = idx + sX + 1;
    inds[4] = idx + slsz;
    inds[5] = idx + slsz + 1;
    inds[6] = idx + slsz + sX;
    inds[7] = idx + slsz + sX + 1;     
    if(FindCriticalPointInIntCell(ix, iy, iz, inds,sX, sY, sZ, 
				  ForceField, flags, false,
				  20, true,
				  &(critPt->position)))
    {
#ifdef TRACE
      printf("Critical point found at: (%lf, %lf, %lf) ", 
	     critPt->position.x, critPt->position.y, critPt->position.z);
#endif
      found = true;
    }
    else {
#ifdef TRACE
      printf("Critical point NOT found ");
#endif
    }
#ifdef TRACE
    printf("in cell (%d, %d, %d)\n", ix, iy, iz);
#endif
  }
  
  return found;
}

