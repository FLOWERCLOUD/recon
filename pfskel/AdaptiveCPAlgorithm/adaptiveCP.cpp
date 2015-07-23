#include "common.h"
#include "gradField.h"
#include "critPts.h"

int main(int argc, char *argv[]) {
  int L, M, N;         // Sizes in x,y,z dimensions
  char *cL, *cM, *cN;
  unsigned char *f;    // volume
  ForceVector *force;       // vector field
  int smSteps;
  char *sInFile, *sOutFile;
  Adaptive_Smoothing_Algorithm adaptiveAlgorithm;
  DynamicArray<CriticalPoint> CritPts;
  int i, j;
  int dx, dy, dz;

#ifdef _DEBUG
  SetStartTime();
#endif
  
  if (argc < 3) {
    printf("\n\
Computes the gradient field of a 3D object.\n\
Usage: \n\
  %s <volfile> <fieldFile> [Options] \n\
Options:\n\
  -s <sX> <sY> <sZ>  = specify size of input volume.\n\
                       DEFAULT: get size from file name <VolFile>.\n\
  -sms <nrSmoothingSteps> = specify the number of mandatory smoothing steps\n\
                            to be performed before the adaptive algorithm \n\
                            kicks in.\n\
                            DEFAULT: 0.\n\
\n\
", argv[0]);
    exit(1);
  }

  sInFile = argv[1];
  sOutFile = argv[2];

  L = 0; M = 0; N = 0;
  //
  // get program options from the command line.
  //
  Option prgOptions[2];
  // -size
  strcpy(prgOptions[0].shortName, "-s");
  strcpy(prgOptions[0].longName, "");
  prgOptions[0].nrValues = 3;
  // -sms
  strcpy(prgOptions[1].shortName, "-sms");
  strcpy(prgOptions[1].longName, "");
  prgOptions[1].nrValues = 1;
  GetProgramOptions(argv, argc, 3, prgOptions, 2);

  //
  // import optional parameters
  //
  // -size
  cL = NULL; cM = NULL; cN = NULL;
  if((prgOptions[0].found) && (prgOptions[0].nrValues == 3)) {
    cL = prgOptions[0].values[0];
    cM = prgOptions[0].values[1];
    cN = prgOptions[0].values[2];
  }
  // 
  // check the size
  //
  // if the size was specified using the -s option, then use that information
  if((cL != NULL) && (cM != NULL) && (cN != NULL)) {
    L = atoi(cL);
    M = atoi(cM);
    N = atoi(cN);
  }
  else {
    // try to get the size from the input volume filename
    GetSizeFromFilename(sInFile, &L, &M, &N);
  }
  // if one of the dimensions id still <= 0, something is not right - abort
  if((L <= 0) || (M <= 0) || (N <= 0)) {
    PrintErrorMessage("Could not determine size of input volume.\n");
    return 1;
  }
  
  // size determined successfuly
  printf("Input dataset: %s. Size: %dx%dx%d.\n", sInFile, L, M, N);
  
  
  // -sms
  smSteps = 0;
  if((prgOptions[1].found) && (prgOptions[1].nrValues == 1)) {
    smSteps = atoi(prgOptions[1].values[0]);
    if(smSteps < 0) {
      printf("Incorrect value for -sms option: %d. Using default value.\n",
	     smSteps);
      smSteps = 0;
    }
  }
  
  

  //
  // read in the volume
  //
  f = NULL;
  if(!ReadVolume(sInFile, L, M, N, &f)) {
    return 1;
  }

  if(!CheckVolumePadding(f, L, M, N, 1)) {
    printf("Object touches volume bounding box. Increase volume padding !\n");
    return 1;
  }

#ifdef _DEBUG
  PrintElapsedTime("Phase 3: reading volume from file.");
#endif

  // set flags
  FlagVolume(f, L, M, N);

  
#ifdef _DEBUG
  PrintElapsedTime("Phase 5: initialize the flags.");
#endif
  
  force = new ForceVector[L*M*N];			// gradient field
  memset(force, 0, sizeof(ForceVector)*L*M*N);
  
  adaptiveAlgorithm = ASA_NONE;
  bool done = false;
  bool zeroEigenvalue = false;
  bool cpAreClose = false;
  while(!done) {
    printf("Using %d smoothing steps...\n", smSteps);
    // Compute force vectors with current smoothing
    if(!CalculateGradientField(L, M, N, f, force, smSteps, adaptiveAlgorithm))
    {
      printf("CalculateGradientField(...) failed !\n");
      return 1;
    }
    
    // compute critical points
    CritPts.Reset();
    if(!GetCriticalPoints(force, L, M, N, f, CritPts, 
			  /*inOut =*/ false, /*notCloseToSurf =*/ false,
			  /*maxRecursionDepth =*/ 20,
			  /*useNewton =*/ true))
    {
      printf("GetCriticalPoints(...) failed !\n");
      return 1;
    }
    /*
    // print critical points
    for(i=0; i < CritPts.GetNrElem(); i++) {
      printf("%.16lf %.16lf %.16lf %d\n",
	     CritPts[i].position.x, CritPts[i].position.y, 
	     CritPts[i].position.z, CritPts[i].type);
    }
    */
    // check the critical points

    // if any of the critical points has a 0 eigenvalue - not done
    zeroEigenvalue = false;
    for(i=0; (i < CritPts.GetNrElem()) && (!zeroEigenvalue); i++) {
      for(j=0; j < 3; j++) {
	if(CritPts[i].eval[j] == 0) {
	  zeroEigenvalue = true;
	  break;
	}
      }
    }
   
    if(!zeroEigenvalue) {
      // check that every pair of attracting nodes is at least 2 voxels away
      cpAreClose = false;
      for(i=0; (i < CritPts.GetNrElem()) && (!cpAreClose); i++) {
	if(CritPts[i].type == CPT_ATTRACTING_NODE) {
	  for(j=i+1; (j < CritPts.GetNrElem()) && (!cpAreClose); j++) {
	    if(CritPts[j].type == CPT_ATTRACTING_NODE) {
	      // check distance
	      dx = 
		abs(int(CritPts[i].position.x) - int(CritPts[j].position.x));
	      dy = 
		abs(int(CritPts[i].position.y) - int(CritPts[j].position.y));
	      dz = 
		abs(int(CritPts[i].position.z) - int(CritPts[j].position.z));

	      if((dx < 2) && (dy < 2) && (dz < 2)) {
		cpAreClose = true;
		break;
	      }
	    }
	  }
	}
      }
    }

    if(zeroEigenvalue || cpAreClose || (CritPts.GetNrElem() == 0)) {
      smSteps++;
    }
    else {
      // we are done
      done = true;
    } 
  }
    
  printf("Done after %d smoothing steps.\n", smSteps);
  
  // print the force vectors.
  SaveVectorField(force, L, M, N, sOutFile);
  
#ifdef _DEBUG
  PrintElapsedTime("Phase 6: saving potential field to file.");
#endif
  
  
#ifdef _DEBUG
  PrintElapsedTime("");
#endif
  
  // free the allocated memory
  delete [] f;
  delete [] force;
  
  printf("Done.\n");
  return 0; 
}
