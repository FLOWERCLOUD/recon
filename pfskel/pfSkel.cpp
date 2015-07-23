//
// Computes the skeleton of a 3D object
//

#include "common.h"
#include "pfSkel.h"
#include "GradField/gradField.h"

// #define TRACE

bool RemoveOutsideSegments(char *volFileName, int L, int M, int N, 
			   Skeleton *Skel);
bool DeleteSkelSegment(Skeleton *Skel, int segmentNr);


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
	cbChangeParameters pfnChangeParams /*= NULL*/, 
	                              // [in]  callback function to change 
	                              //    parameters on the run.
	void *other /*= NULL*/,       // [in] value to be passed to the 
	                              //    callback function when called
	char *vfInFile /*= NULL*/,    // [in] vector field input file. If this 
	                              //   parameter is not NULL, the 
	                              //   potential field is read from the 
                                      //   file instead of being calculated 
                                      //   here.
	char *vfOutFile /*= NULL*/,   // [in] dump vector field to this file
	PFNorm norm /*=PF_NORM_L2*/,  // [in] vector norm to use
	HDSelection hdSel /*=HDS_LocMin*/,
	                              // [in] specifies how the high divergence
	                              //    points are selected:
	                              //    from all points (HDS_All) or only
	                              //    from local minima (HDS_LocMin)
	                              // DEFAULT: HDS_LocMin
	bool remOutSegs /*= true*/,   // [in] remove segments that go outside
                                      //    the original object
	                              // DEFAULT: true
	FieldType fType /*=PF*/,      // [in] type of vector field to use:
	                              //    PF  = potential field (slower)
	                              //    GDF = gradient diffusion field 
	                              //          (faster)
	bool useDistanceField /*=false*/ 
	                              // [in] use distance field
	                              // DEFAULT: false

){
  unsigned char *f;
  ForceVector *force;
  long sz, slsz;

  DynamicArray<VoxelPosition> HCBPoints;
  DynamicArray<CriticalPoint> CritPts;
  DynamicArray<DivergencePoint> HDPts;

  // this variable should be removed and added as a parameter of this function
  float percHCPts = 0;

  float percHDPts = pHDPts;

  int dL, dM, dN;  // used to make tight bounding box
  int oldL, oldM, oldN;  // original values of volume size

  //
  // save current values of L, M and N 
  //
  oldL = L;
  oldM = M;
  oldN = N;


  // distance of charges should be >= 0 but it is ignored if vfInFile 
  //   is not NULL
  if((distCharges < 0) && (vfInFile == NULL))  {
    printf("pfSkel.cpp: <distCharges> parameter cannot be < 0.\n");
    return false;
  }

  // field strength should be between 1 and 10 but it is ignored if vfInFile 
  //    is not NULL
  if(((fieldStrenght < 1) || (fieldStrenght > 10)) && (vfInFile == NULL)) {
    printf("pfSkel.cpp: <fieldStrenght> parameter must be in the [1..10] \
range. Recommended values [4..9].\n");
    return false;
  }
  
  // read in the volume
  // volume array f is allocated inside this function
#ifdef _DEBUG
  printf("PFS-1: Reading volume data...\n");
#endif

  if(!ReadVolume(volFileName, L, M, N, &f)) {
    return false;
  }

#ifdef _DEBUG
  PrintElapsedTime(" ");
#endif

  
  PrintInfoMessage("Resizing volume ...");
  //
  // make the bounding box as tight as possible, to save memory
  //
  if(!PadVolume(&f, &L, &M, &N, 
		distCharges + 1, // should allow for object expansion
		&dL, &dM, &dN))
  {
    PrintErrorMessage("Resize operation failed. Stop.\n");
    return false;
  }
  // resize operation successful
  // save dL, dM, and dN into the skeleton structure. Needed when saving the 
  //   skeleton
  Skel->SetOffsets(dL, dM, dN);
  printf("New size: %dx%dx%d.\n", L, M, N);
  
  sz = L*M*N;
  slsz = L*M;
  
  // Expand the volume first, then make it solid
  // This way, some small holes on the surface will be covered before 
  // flood-filling the volume
  //

  //
  // thicken the object with <distCharges> layers of extra voxels
  //
#ifdef _DEBUG
  printf("PFS-4: Placing charges outside the object (at %d layers)...\n", 
	 distCharges);
#endif
    
  ExpandNLayers(L, M, N, f, distCharges);
    
#ifdef _DEBUG
  PrintElapsedTime(" ");
#endif


#ifdef _DEBUG
  printf(" PFS-2: Make solid volume...\n ");
#endif

  // 
  // Make sure the volume does not have holes in it
  //
  MakeSolidVolume(L, M, N, f, EXTERIOR, INTERIOR); 

#ifdef _DEBUG 
  PrintElapsedTime(" ");
#endif


#ifdef _DEBUG
  printf("PFS-3: Calculating high curvature boundary points - NOT DONE.\n");
#endif

  //
  // detecting the high curvature boundary points, and retrieve the first 1%
  //
  // Module is not working yet, ...
  // GetHighCurvatureBoundaryPoints(cf, L, M, N, HCBPoints, 1.00);

  // flag the volume
  FlagVolume(f, L, M, N);

#ifdef _DEBUG
    printf("Phase 4: Calculating distance field...\n");
#endif

  float *distField = NULL;
  // Compute distance field if required.
  if (useDistanceField) {
    distField = new float[L*M*N];
    if (distField == NULL) {
      printf("Error allocating memory for the working data structures. Abort\n");
      exit(1);
    }
    
    if (!GetDT(f, L, M, N, distField)) {
      printf("Error computing distance field. Abort\n");
      exit(1);
    }
  }

  force = new ForceVector[L*M*N];			// potential field
  if(force == NULL) {
    printf("Error allocating memory for the working data structures. Abort\n");
    exit(1);
  }

  if(vfInFile == NULL) {
    
    //
    // Calculating potential field...
    //
#ifdef _DEBUG
    printf("Phase 5: Calculating vector field...\n");
#endif
    
    // Compute force vectors
    switch(fType) {
      case PF: {
	CalculatePotentialField(L, M, N, f, fieldStrenght, force, false, norm);
	break;
      }
      case GDF: {
	CalculateGradientField(L, M, N, f, force, false);
	break;
      }
      default: {
	printf("Unrecognized field type: %d !\n", fType);
	exit(1);
      }
    }

    /*
#ifdef _DEBUG
    printf("Phase 5.1: Smooth potential field...\n");
#endif
    // smooth vector field
    SmoothVectorField(force, L, M, N);
    */

#ifdef _DEBUG
    PrintElapsedTime(" ");
#endif
  }
  else {
    // do not compute the potential field, just read it from the specified file
    printf("Reading vector field from file %s...", vfInFile);
    if(!ReadVectorField(force, L, M, N, vfInFile)) {
      return false;
    }
    printf("done.\n");
  }


  //
  // If vfOutFile id not null, dump the vector field to that file
  //
  if(vfOutFile != NULL) {
    SaveVectorField(force, L, M, N, vfOutFile);
  }

  
  //
  // Return the volume to the original shape by reverting the expand process
  // This way, we will compute critical points, high divergence points and 
  // the skeleton only inside the original object
  //

#ifdef _DEBUG
  printf("PFS-4: Reverting to original object (solid)...\n");
#endif
    
  ExpandNLayers(L, M, N, f, -distCharges);
  
#ifdef _DEBUG
  PrintElapsedTime(" ");
#endif
  

  //
  // Detecting critical points
  //

#ifdef _DEBUG
  printf("Phase 6: Detecting critical points...\n");
#endif

  GetCriticalPoints(force, L, M, N, f, CritPts, false, false, 20, true);


#ifdef _DEBUG
  PrintElapsedTime(" ");
#endif


  //
  // generating the skeleton
  //
#ifdef _DEBUG
  printf("Phase 7: Detecting skeleton points...\n");
#endif

  bool stop = false;
  pfSkelCommand cmd;

  // in interactive mode, we allow the user to change parameters and then run
  // the streamlines part again, until he/she is satisfied with the result.

  // first we compute the level 1 skeleton that connects only critical points.
  // The user has no control over the computation of this part, so it is done
  //   only once and then reused in case the parameters change.
  //
  GetLevel1Skeleton(force, f, L, M, N, CritPts, Skel, distField);

#ifdef _DEBUG
  printf("Phase 7.1: Level 1 skeleton generation completed.\n");
#endif
  //
  // save the level 1 skeleton
  //
  Skeleton level1Skel(*Skel);

  // compute level 2 skeleton using as basis the level 1 skeleton
  stop = false;
  while(!stop) {
    //
    // Get top ... % of high divergence points
    //
    GetHighDivergencePoints(force, L, M, N, f, percHDPts, HDPts,
			    false, hdSel);
    
    //
    // get the skeleton
    //
    GetLevel2Skeleton(force, f, L, M, N, HDPts, Skel, distField);
    
    //
    // remove segments that contain points outside the original objects 
    //  (if necessary)
    //
    if(remOutSegs) {
      // have to use saved values of volume size
      RemoveOutsideSegments(volFileName, oldL, oldM, oldN, Skel);
    }

    // at this point we give the user a chance to save the skeleton and
    // change the parameters if he/she wishes.
    //
    cmd.HDP = percHDPts;
    cmd.HCP = percHCPts;
    
    if((*pfnChangeParams)(Skel, &cmd, other)) {
      switch(cmd.cmdCode[0]) {
      case 'q':
	// quit
	stop = true;
	break;
      case 'p':
	// change parameters request
	percHDPts = cmd.HDP;
	percHCPts = cmd.HCP;
	break;
      default:
	// what could this be ???
	stop = true;
	break;
      }
    }
    else {
      stop = true;
    }

    // return to level1 skeleton.
    *Skel = level1Skel;
  }

#ifdef _DEBUG
  PrintElapsedTime(" ");
#endif

  //
  // free the allocated memory
  //
  delete [] f;
  delete [] force;

  return true;
}
		

///////////////////////////////////////////////////////////////////////////////
// Remove skeleton segments that go outside the object
///////////////////////////////////////////////////////////////////////////////
bool RemoveOutsideSegments(char *volFileName, int L, int M, int N, 
			   Skeleton *Skel) 
{
  PrintDebugMessage("Checking skeleton points...");

  unsigned char *f;
  int xs, ys, zs;
  long idx, slsz;
  int i, j;
  bool deleteThisSegment = false;
  int remCnt = 0;
  int dx, dy, dz;  // skeleton offsets.

  // read the original value, make it solid
  if(!ReadVolume(volFileName, L, M, N, &f)) {
    return false;
  }
  if(!MakeSolidVolume(L, M, N, f, EXTERIOR, INTERIOR)) {
    return false;
  }

  // get skeleton offsets.
  Skel->GetOffsets(&dx, &dy, &dz);

  slsz = L*M;
  remCnt = 0;
  // for each segment, check if it contains one point outside the original 
  //   object
  // if YES - remove the whole segment from the skeleton
  for(i=0; i < Skel->GetNumberOfSegments(); i++) {
    deleteThisSegment = false;

    for (j=0; j < (*Skel)[i].GetNumberOfPoints(); j++) {
      // check the left end point of the segment
      xs = (int) ((*Skel)[i][j].position.x + dx);
      ys = (int) ((*Skel)[i][j].position.y + dy);
      zs = (int) ((*Skel)[i][j].position.z + dz);
    
      idx = zs*slsz + ys*L + xs;
      if(f[idx] == EXTERIOR) {
	//this segment should be deleted
	deleteThisSegment = true;
	break;
      }
    }
    
    if(deleteThisSegment) {
      // delete the segment
      Skel->RemoveSegment(i);
      remCnt ++;
    }
  }
  
  // free the memory
  delete [] f;

  PrintDebugMessage("done.\n");
  PrintDebugMessage("  %d segments were removed.\n", remCnt);
  return true;
}
