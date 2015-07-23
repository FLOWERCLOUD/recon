////////////////////////////////////////////////////////////////////////////
// driver program for the expandVol module
////////////////////////////////////////////////////////////////////////////

#include "expandVol.h"

int main(int argc, char *argv[])
{
  FILE *fout;

  int L,M,N;         // Sizes in x,y,z dimensions
  char *cL, *cM, *cN;

  unsigned char *f;
  long idx, slsz, sz;
  int layers;

  char *sInFile, *sOutFile;

#ifdef _DEBUG
	SetStartTime();
#endif

  if (argc < 4) {
    printf("\n\
Add/remove voxels at/from the object's surface\n\
Usage: \n\
  %s <volfile> <nroflayers> <paddedvolfile> [Options]\n\
\n\
Options:\n\
  -s <sX> <sY> <sZ> = specify size of input volume.\n\
                      DEFAULT: get size from file name <VolFile>.\n\
\n\
",argv[0]);
    exit(1);
  }

  sInFile = argv[1];
  layers = atoi(argv[2]);
  sOutFile = argv[3];

//
  // get program options from the command line.
  //
  Option prgOptions[1];
  // -size
  strcpy(prgOptions[0].shortName, "-s");
  strcpy(prgOptions[0].longName, "");
  prgOptions[0].nrValues = 3;
  

  GetProgramOptions(argv, argc, 4, prgOptions, 1);

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

  f = NULL;
  if(!ReadVolume(sInFile, L, M, N, &f)) {
    return 1;
  }


#ifdef _DEBUG
  PrintElapsedTime("Phase 1: reading volume from file.");
#endif

  // flag the volume
  FlagVolume(f, L, M, N);


#ifdef _DEBUG
  PrintElapsedTime("Phase 2: flag the volume.");
#endif

  printf("Padding the object with %d layers of extra voxels\n", layers);
  if(!ExpandNLayers(L, M, N, f, layers)) {
    return 1;
  }


#ifdef _DEBUG
  PrintElapsedTime("Phase 3: expand the object.");
#endif

  // make the volume binary
  for(idx=0; idx < L*M*N; idx++) {
    if(f[idx] != EXTERIOR) {
      f[idx] = 255;
    }
  }

#ifdef _DEBUG
  PrintElapsedTime("Phase 4: make binary volume.");
#endif

// save the new volume
  if ((fout = fopen(sOutFile,"wb")) == NULL) {
    printf("Cannot open output file %s for writing\n",sOutFile);
    exit(1);
  }

  if(fwrite(f, sizeof(unsigned char), sz, fout) != ((size_t)sz)) {
    printf("Error writing to the output file\n");
    exit(1);
}

#ifdef _DEBUG
  PrintElapsedTime("Phase 5: saving new volume to file.");
#endif
  fclose(fout);


#ifdef _DEBUG
  PrintElapsedTime("");
#endif

// free the allocated memory
  delete [] f;

  printf("Done.\n");
  return 0;
}



