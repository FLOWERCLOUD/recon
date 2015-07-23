//
// driver program for the Connectivity module
//

#include "common.h"
#include "connectivity.h"



int main(int argc, char *argv[]) {

  int L,M,N;         // Sizes in x,y,z dimensions
  char *cL, *cM, *cN;

  unsigned char *vol;		// volume
  char *sInFile, *sOutFile;
  
#ifdef _DEBUG
  SetStartTime();
#endif

  if (argc < 3)
  {
    printf("\n\
Computes the connected components in a volume and keeps the largest one. \n\
Everything else is set to 0.\n\
The search stops when a component containing more than 50%% of the object \n\
voxels is found.\n\
In case of a tie, the first encountered component is kept.\n\
\n\
Usage: \n\
  %s <input volume> <output volume> [Options].\n\
Options:\n\
  -s <sX> <sY> <sZ> = specifies the size of input volume.\n\
                      DEFAULT: get size from file name <input volume>.\n\
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

  // read volume
  ReadVolume(sInFile, L, M, N, &vol);

  // check volume padding
  if(!CheckVolumePadding(vol, L, M, N, 1)) {
    printf("Object touches volume bounding box. Increase volume padding !\n");
    return 1;
  }
    
#ifdef _DEBUG
  PrintElapsedTime("Phase1: reading volume and setting flags");
#endif

  // compute connected components
  if(CheckConnectivity(vol, L, M, N)) {
    // save volume to output file
    SaveVolume(sOutFile, L, M, N, vol);
  }

#ifdef _DEBUG
  PrintElapsedTime("Phase2: finding largest connected component");
#endif
  
  // free memory
  delete [] vol;

  printf("Done.\n");
  
  return 0;
}
