///////////////////////////////////////////////////////////////////////////////
// driver program for the DistField module
///////////////////////////////////////////////////////////////////////////////

#include "getDT.h"
#include "common.h"

int main(int argc, char *argv[])
{
  int L, M, N;         // Sizes in x,y,z dimensions
  char *cL, *cM, *cN;
  unsigned char *f;    // volume
  float *df;           // distance field
  long idx, slsz, sz;  

  char *sInFile, *sOutFile;

#ifdef _DEBUG
	SetStartTime();
#endif

  if (argc < 3) {
    printf("\n\
Computes the distance field of a 3D object \n\
  (distance to closest boundary point).\n\
Usage: \n\
  %s <volfile> <dfFile> [Options] \n\
Options:\n\
  -s <sX> <sY> <sZ>  = specify size of input volume.\n\
                       DEFAULT: get size from file name <VolFile>.\n\
", argv[0]);
    exit(1);
  }

  sInFile = argv[1];
  sOutFile = argv[2];

  L = 0; M = 0; N = 0;

  //
  // get program options from the command line.
  //
  Option prgOptions[1];
  // -size
  strcpy(prgOptions[0].shortName, "-s");
  strcpy(prgOptions[0].longName, "");
  prgOptions[0].nrValues = 3;

  GetProgramOptions(argv, argc, 3, prgOptions, 1);

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
  
  
  slsz = L*M;		// slice size
  sz = slsz*N;

  //
  // read in the volume
  //
  f = NULL;
  if(!ReadVolume(sInFile, L, M, N, &f)) {
    return 1;
  }

#ifdef _DEBUG
  PrintElapsedTime("Phase 0: reading volume from file.");
#endif

  // make binary
  for (idx = 0; idx < sz; idx++) {
    if (f[idx] > 0) {
      f[idx] = INTERIOR;
    }
    else {
      f[idx] = 0;
    }
  }
  
  // allocate space for the distance field
  df = new float[sz];			

  // Compute distance field
  GetDT(f, L, M, N, df);

#ifdef _DEBUG
  PrintElapsedTime("Phase 4: computing distance field.");
#endif


  // delete volume
  delete [] f;
  f = NULL;

  // save to output file
  SaveDistanceField(sOutFile, L, M, N, df);

  // free the allocated memory
  delete [] df;
 
#ifdef _DEBUG
  PrintElapsedTime("Phase 5: saving distance field to file.");
#endif

#ifdef _DEBUG
  PrintElapsedTime("");
#endif


  printf("Done.\n");
  return 0; 
}


