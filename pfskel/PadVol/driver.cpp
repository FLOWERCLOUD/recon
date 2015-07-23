////////////////////////////////////////////////////////////////////////////
// driver program for the expandVol module
////////////////////////////////////////////////////////////////////////////

#include "common.h"

int main(int argc, char *argv[])
{
  FILE *fout;

  int L,M,N;         // Sizes in x,y,z dimensions
  int dL, dM, dN;
  char *cL, *cM, *cN;

  unsigned char *f;
  long slsz, sz;
  int layers;

  char *sInFile, *sOutFile;
  bool outFileAllocated;
  bool deleteInput;

#ifdef _DEBUG
	SetStartTime();
#endif

  if (argc < 3) {
    printf("\n\
Adds/removes layers of empty voxels in each direction, so that there are\n\
at least the specified number of voxels between the object and the bounding \n\
box.\n\
Usage: \n\
  %s <volfile> <nroflayers> [Options]\n\
\n\
Options:\n\
  -s <sX> <sY> <sZ>     = specify size of input volume.\n\
                          DEFAULT: get size from file name <VolFile>.\n\
  -o <paddedVolumeName> = specify the name of the output volume.\n\
                          DEFAULT: the old name, with the size updated \n\
                            (if necessary).\n\
  -d                    = delete input file.\n\
\n\
",argv[0]);
    exit(1);
  }

  sInFile = argv[1];
  layers = atoi(argv[2]);
  //  sOutFile = argv[3];

  //
  // get program options from the command line.
  //
  Option prgOptions[3];
  // -size
  strcpy(prgOptions[0].shortName, "-s");
  strcpy(prgOptions[0].longName, "");
  prgOptions[0].nrValues = 3;
  // -o
  strcpy(prgOptions[1].shortName, "-o");
  strcpy(prgOptions[1].longName, "");
  prgOptions[1].nrValues = 1;
  // -d
  strcpy(prgOptions[2].shortName, "-d");
  strcpy(prgOptions[2].longName, "");
  prgOptions[2].nrValues = 0;

  GetProgramOptions(argv, argc, 3, prgOptions, 3);

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

  sOutFile = NULL;
  // -o - output file name
  if((prgOptions[1].found) && (prgOptions[1].nrValues == 1)) {
    sOutFile = prgOptions[1].values[0];
  }
  else {
    // output file name is not given - will construct it from the input file 
    // name, once we know the new size
    sOutFile = NULL;
  }

  deleteInput = false;
  // -o - output file name
  if(prgOptions[2].found) {
    deleteInput = true;
  }


  slsz = L*M;		// slice size
  sz = slsz*N;

  f = NULL;
  if(!ReadVolume(sInFile, L, M, N, &f)) {
    return 1;
  }


#ifdef _DEBUG
  PrintElapsedTime("Phase 1: reading volume from file.");
#endif


  printf("Padding the volume to %d layers\n", layers);
  if(!PadVolume(&f, &L, &M, &N, layers, &dL, &dM, &dN)) {
    return 1;
  }

  slsz = L*M;		// slice size
  sz = slsz*N;

  printf("New volume dimensions: %dx%dx%d.\n", L, M, N);
  printf("New volume offsets: %d %d %d.\n", dL, dM, dN);

#ifdef _DEBUG
  PrintElapsedTime("Phase 3: pad the volume.");
#endif

  // 
  // if an output file name was not given, construct it from the old name
  // 
  outFileAllocated = false;
  if(sOutFile == NULL) {
    if((sOutFile = new char[300]) == NULL) {
      printf("Not enough memory ?!? - Abort.\n");
      return 1;
    }
    outFileAllocated = true;
   
    ChangeSizeInFilename(sInFile, L, M, N, sOutFile);
  }

  // delete input file if necessary
  if(deleteInput) {
    if(remove(sInFile) != 0) {
      printf("Unable to remove input file !!\n");
    }
  }

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
  if(outFileAllocated) {
    delete [] sOutFile;
  }

  printf("Done.\n");
  return 0;
}




