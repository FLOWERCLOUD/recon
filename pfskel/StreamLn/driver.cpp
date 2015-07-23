//
// driver program for the StreamLn module
//

// #include "../common.h"
#include "streamLn.h"
#include "critPts.h"
#include "getDT.h"
#include "highDiverg.h"

// #define TRACE

int main(int argc, char *argv[])
{
  // FILE *fseeds;

  int L, M, N;         // Sizes in x,y,z dimensions
  char *cL, *cM, *cN;
  int i; // ,j;
  ForceVector *fv;
  float *df;
  unsigned char *flags = NULL;
  
  DynamicArray<DivergencePoint> HDPoints;
  DynamicArray<VoxelPositionDouble> BdSeeds;
  Skeleton Skel;

  long slsz, sz;

  DynamicArray<CriticalPoint> CritPts;

  // int read;
  int skelOutType = 0;  // point skeleton - default

  char *sVolFile, *sFieldFile, *sOutFile, *sCPFile, *sSeedsFile;

  // VoxelPositionDouble pos;

#ifdef _DEBUG
  SetStartTime();
#endif

  if (argc < 6)
  {
    printf("\n\
Computes the skeleton given a potential field, critical points \n\
  and other seed points.\n\
Usage: \n\
  %s <volumeFile> <vectorFile> <CritPtsFile> <seedsFile|-noseeds> <skelFile> [Options]\n\
    if -noseeds is specified intead of a seeds file name, no seeds are read.\n\
Options:\n\
  -s <sX> <sY> <sZ> = specify size of input volume.\n\
                      DEFAULT: get size from file name <vectorFile>.\n\
  -f <f>            = skeleton output format.\n\
                      <f> can be: 0 - save as points\n\
                                  1 - save as line segments\n\
                                  2 - save full structure\n\
                      DEFAULT: 0 - skeleton is saved as points.\n\
  -df <distanceFieldFile> = name of distance field file.\n\
",argv[0]);
    return 1;
  }

  sVolFile = argv[1];
  sFieldFile = argv[2];
  sCPFile    = argv[3];
  sSeedsFile = argv[4];
  sOutFile   = argv[5];

  L = 0; M = 0; N = 0;

 //
  // get program options from the command line.
  //
  Option prgOptions[3];
  // -size
  strcpy(prgOptions[0].shortName, "-s");
  strcpy(prgOptions[0].longName, "");
  prgOptions[0].nrValues = 3;
  
  // -lineskel
  strcpy(prgOptions[1].shortName, "-f");
  strcpy(prgOptions[1].longName, "");
  prgOptions[1].nrValues = 1;

  // -df
  strcpy(prgOptions[2].shortName, "-df");
  strcpy(prgOptions[2].longName, "");
  prgOptions[2].nrValues = 1;

  GetProgramOptions(argv, argc, 6, prgOptions, 3);

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
    GetSizeFromFilename(sVolFile, &L, &M, &N);
  }
  // if one of the dimensions id still <= 0, something is not right - abort
  if((L <= 0) || (M <= 0) || (N <= 0)) {
    PrintErrorMessage("Could not determine size of input volume.\n");
    return 1;
  }
  
  // size determined successfuly

  printf("Input dataset: %s. Size: %dx%dx%d.\n", sVolFile, L, M, N);

  skelOutType = 0; // point skeleton
  if(prgOptions[1].found && (prgOptions[1].nrValues == 1)) {
    
    skelOutType = atoi(prgOptions[1].values[0]);  // line skeleton
    printf("Skeleton ouput format: %d.\n", skelOutType);
  }

  slsz = L*M;		// slice size
  sz = slsz*N;

  // allocate space for the force vector field
  if((fv = new ForceVector[L*M*N]) == NULL) {
    printf("OOPS! - Error allocating memory for the input vector field. \
Abort.\n");
    exit(1);
  }

  // read in volume and set flags
  if(!ReadVolume(sVolFile, L, M, N, &flags)) return 1;
  if(!FlagVolume(flags, L, M, N)) return 1;
  
  // read in force vectors
  if(!ReadVectorField(fv, L, M, N, sFieldFile)) return 1;

#ifdef _DEBUG
  PrintElapsedTime("Phase2: reading in force vectors");
#endif
  
  // reading in critical points
  if(!ReadCriticalPoints(sCPFile, CritPts)) {
    return 1;
  }
  
  
#ifdef _DEBUG
  PrintElapsedTime("Phase3: reading in critical points");
  printf("** Read %d critical points\n", CritPts.GetNrElem());
  int nrs, nra, nrr, nru;
  nrs = 0;	nra = 0;	nrr = 0; nru = 0;
  for(i=0; i < CritPts.GetNrElem(); i++) {
    
    switch(CritPts[i].type) {
    case CPT_SADDLE:
      nrs++;
      break;
    case CPT_ATTRACTING_NODE:
      nra++;
      break;
    case CPT_REPELLING_NODE:
      nrr++;
      break;
    case CPT_UNKNOWN:
      nru++;
      break;
    }
  }
  printf("%d saddles, %d attracting nodes, %d repelling nodes, %d unknown.\n", nrs, nra, nrr, nru);
#endif
  
// reading seed points
  if(strncmp(sSeedsFile, "-noseeds", 8) == 0) {
    // do not open the seeds file
  }
  else {
    if(!ReadHDPoints(sSeedsFile, HDPoints)) {
      return 1;
    }
  }
  
#ifdef _DEBUG
  PrintElapsedTime("Phase3: reading in seed points");
#endif

#ifdef TRACE	  
  printf("Read %d seed points:\n", HDPoints.GetNrElem());
  for(i=0; i < HDPoints.GetNrElem(); i++) {
    printf("%f %f %f\n", HDPoints[i].position.x, HDPoints[i].position.y, HDPoints[i].position.z);
  }
#endif

  
  // read distance field if given
  // distance field
  df = NULL;
  if(prgOptions[2].found && (prgOptions[2].nrValues == 1)) {
    if(!ReadDistanceField(prgOptions[2].values[0], L, M, N, &df)) {
      df = NULL;
      // do not return -- continue anyway
    }
  }
  

  if(df != NULL) {
    // print max and min in dt
    float mind, maxd;
    mind = df[0];
    maxd = df[0];
    for(i=0; i < L*M*N; i++) {
      if(df[i] < mind) mind = df[i];
      if(df[i] > maxd) maxd = df[i];
    }
    printf("Distance field: %f - %f\n", mind, maxd);
  }

  GetStreamLines(fv, flags, L, M, N, 
		 CritPts, BdSeeds, HDPoints,
		 Skel, df);
  
#ifdef _DEBUG
  PrintElapsedTime("Phase4: Getting the streamlines\n");
#endif

  delete [] flags;
  delete [] fv;
  if(df != NULL) {
    delete [] df;
  }
  
  // RemoveDisconnectedSegments(Skel);
  Skel.SaveToFile(sOutFile, skelOutType);
  
  
#ifdef _DEBUG
  PrintElapsedTime("Phase5: Writing output file");
#endif
  printf("End \n");

  return 0;
}
