//
// driver program for the CritPts module
//

// #include "../common.h"
#include "critPts.h"

// #include <fstream>
// #include <iostream>

int main(int argc, char *argv[])
{

  int L,M,N;         // Sizes in x,y,z dimensions
  char *cL, *cM, *cN;

  int i;
  ForceVector *fv;

  DynamicArray<CriticalPoint> CritPts;

  long slsz;

  unsigned char *f;		// flags
  bool inOut = false;
  bool notCloseToSurf = false;
  
  int sd, at, re, un;

  char *sInFile, *sFieldFile, *sOutFile;
  
  int maxRecursionDepth = 20;
  bool useNewton = true;
  
#ifdef _DEBUG
  SetStartTime();
#endif

  if (argc < 4)
  {
    printf("\n\
Computes the critical points of a vector field.\n\
Usage: \n\
  %s <volume file> <vector file> <critical points file> [Options].\n\
Options:\n\
  -s <sX> <sY> <sZ> = specifies the size of input volume.\n\
                      DEFAULT: get size from file name <volume file>.\n\
  -rd <recDepth>    = specifies the maximum depth of the recursion.\n\
                      DEFAULT: 20.\n\
  -nn               = do not use Newton's method to locate critical points\n\
                      exactly.\n\
  -ipcs             = ignore points close to surface (1 voxel away)\n\
  -out <flag>       = specifies whether you want to look for critical points\n\
                      outside the object too, not just inside.\n\
                      flag = 0 - inside only\n\
                           = 1 - inside and outside.\n\
                      DEFAULT: 0.\n\
", argv[0]);
    exit(1);
  }

  sInFile = argv[1];
  sFieldFile = argv[2];
  sOutFile = argv[3];

  L = 0; M = 0; N = 0;
  inOut = false;

 //
  // get program options from the command line.
  //
  Option prgOptions[5];
  // -size
  strcpy(prgOptions[0].shortName, "-s");
  strcpy(prgOptions[0].longName, "");
  prgOptions[0].nrValues = 3;
  // -out
  strcpy(prgOptions[1].shortName, "-out");
  strcpy(prgOptions[1].longName, "");
  prgOptions[1].nrValues = 1;
  // -ipcs
  strcpy(prgOptions[2].shortName, "-ipcs");
  strcpy(prgOptions[2].longName, "");
  prgOptions[2].nrValues = 0;
  // -nn
  strcpy(prgOptions[3].shortName, "-nn");
  strcpy(prgOptions[3].longName, "");
  prgOptions[3].nrValues = 0;
  // -rd
  strcpy(prgOptions[4].shortName, "-rd");
  strcpy(prgOptions[4].longName, "");
  prgOptions[4].nrValues = 1;


  GetProgramOptions(argv, argc, 4, prgOptions, 5);

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
  
  // inOut
  inOut = false;
  if((prgOptions[1].found) && (prgOptions[1].nrValues == 1)) {
    if(strcmp(prgOptions[1].values[0], "1") == 0) {
      inOut = true;
    }
  }

  // ipcs
  notCloseToSurf = false;
  if((prgOptions[2].found)) {
    notCloseToSurf = true;
  }

  // -nn
  useNewton = true;
  if(prgOptions[3].found) {
    useNewton = false;
  }
  
  // -rd
  maxRecursionDepth = 20; // default
  if((prgOptions[4].found) && (prgOptions[4].nrValues == 1)) {
    maxRecursionDepth = atoi(prgOptions[4].values[0]);
    if((maxRecursionDepth <= 0) || (maxRecursionDepth > 49)) {
      maxRecursionDepth = 20;
      printf("\
Invalid maximum rcursion depth value: %s. Valid range is 1..49\n\
Using the default value: %d.\n",
	     prgOptions[4].values[0], maxRecursionDepth);
    }
  }
  
   
  
  // size determined successfuly
  printf("Input dataset: %s. Size: %dx%dx%d.\n", sInFile, L, M, N);

  slsz = L*M;		// slice size

  // read volume
  ReadVolume(sInFile, L, M, N, &f);

  // check volume padding
  if(!CheckVolumePadding(f, L, M, N, 1)) {
    printf("Object touches volume bounding box. Increase volume padding !\n");
    return 1;
  }
  
  // set flags
  FlagVolume(f, L, M, N);

  /* output the flags 
  FILE *fo = fopen("flags.vol", "wb");
  fwrite(f, sizeof(unsigned char), L*M*N, fo);
  fclose(fo);
  */

#ifdef _DEBUG
  PrintElapsedTime("Phase1: reading volume and setting flags");
#endif

  if((fv = new ForceVector[L*M*N]) == NULL) {
    printf("OOPS! - Error allocating memory for the input vector field. Abort.\n");
    exit(1);
  }

  // read in force vectors
  if(!ReadVectorField(fv, L, M, N, sFieldFile)) {
    exit(1);
  }


#ifdef _DEBUG
  PrintElapsedTime("Phase2: reading in force vectors");
#endif

  /*  No smoothing - 
  printf("Optional step: Smoothing vector field ... \n");
  // smooth the vector field
  SmoothVectorField(fv, L, M, N);

#ifdef _DEBUG
  PrintElapsedTime("Phase2.1 - options: smooth vector field");
#endif
  */

  GetCriticalPoints(fv, L, M, N, f, CritPts,
		    inOut, notCloseToSurf, maxRecursionDepth, useNewton);


#ifdef _DEBUG
  PrintElapsedTime("Phase4: Getting the critical points.");
  printf("**  %d critical points returned.\n", CritPts.GetNrElem());
#endif
  
  delete [] f;

  ///save critical points to file
  SaveCriticalPoints(CritPts, sOutFile);

#ifdef _DEBUG
  // count how many saddles, attracting and repelling points we have
  sd = 0;
  at = 0;
  re = 0;
  un = 0;
  
  for(i=0; i < CritPts.GetNrElem(); i++) {
#ifdef TRACE
    printf("writing point (%f, %f, %f) to output file.\n", 
	   CritPts[i].position.x, CritPts[i].position.y, CritPts[i].position.z);
#endif
    
    switch(CritPts[i].type) {
    case CPT_SADDLE:
      sd++;
      break;
    case CPT_ATTRACTING_NODE:
      at++;
      break;
    case CPT_REPELLING_NODE:
      re++;
      break;
    case CPT_UNKNOWN:
      un++;
      break;
    }
  }
  
  printf("\
Critical points: \n\
  %d saddles, \n\
  %d attracting nodes, \n\
  %d repelling nodes, \n\
  %d unknown.\n", 
	 sd, at, re, un);
  PrintElapsedTime("Phase5: Writing output file");
#endif
  
  printf("End \n");
  
  return 0;
}
