#include <stdio.h>
#include <stdlib.h>

#include "common.h"
#include "makeSolidVol.h"

int main(int argc, char *argv[])
{
  FILE *fout;
  unsigned char *vol;
  int L,M,N;     		// Sizes in x,y,z dimensions
  char *cL, *cM, *cN;
  char *sInFile, *sOutFile;
  bool skipX, skipY, skipZ;

  if (argc < 3) {
    printf("\
Makes a 3D object solid, filling all the holes in the object.\n\
Usage: \n\
  %s <VolFile> <newVolFile> [options]\n\
    <VolFile>    = volume data file name\n\
    <newVolFile> = output file name\n\
Options:\n\
  -s <sX> <sY> <sZ> = specify size of input volume.\n\
                      DEFAULT: get size from file name <VolFile>.\n\
  -skipX            = skip flood-fills in X planes.\n\
  -skipY            = skip flood-fills in Y planes.\n\
  -skipZ            = skip flood-fills in Z planes.\n\
",argv[0]);

    exit(1);
  }

  sInFile = argv[1];
  sOutFile = argv[2];

  //
  // get program options from the command line.
  //
  Option prgOptions[4];
  // -size
  strcpy(prgOptions[0].shortName, "-s");
  strcpy(prgOptions[0].longName, "");
  prgOptions[0].nrValues = 3;
  
  // -skipX
  strcpy(prgOptions[1].shortName, "-skipX");
  strcpy(prgOptions[1].longName, "");
  prgOptions[1].nrValues = 0;

  // -skipY
  strcpy(prgOptions[2].shortName, "-skipY");
  strcpy(prgOptions[2].longName, "");
  prgOptions[2].nrValues = 0;

  // -skipZ
  strcpy(prgOptions[3].shortName, "-skipZ");
  strcpy(prgOptions[3].longName, "");
  prgOptions[3].nrValues = 0;

  GetProgramOptions(argv, argc, 3, prgOptions, 4);

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

  // skip X, Y or Z
  skipX = false;
  skipY = false;
  skipZ = false;
  if(prgOptions[1].found) {
    skipX = true;
  }
  if(prgOptions[2].found) {
    skipY = true;
  }
  if(prgOptions[3].found) {
    skipZ = true;
  }

  //  
  // read the volume.
  // space is allocated inside this function
  //
  PrintInfoMessage("MSV: Reading the volume ...");  
  vol = NULL;
  if(!ReadVolume(sInFile, L, M, N, &vol)) {
     // could not read the input volume - abort
    return 1;
  }
  PrintInfoMessage("done.\n");  

  PrintInfoMessage("MSV: Processing volume ...");  
  MakeSolidVolume(L, M, N, vol, 0, 255, skipX, skipY, skipZ);
  PrintInfoMessage("done.\n");

  // output new volume to file
  PrintInfoMessage("MSV: Writing the volume to output file %s...", sOutFile);
  if ((fout = fopen(sOutFile,"wb")) == NULL) {
    printf("Cannot open output file %s for writing\n",sOutFile);
    exit(1);
  }

  if(fwrite(vol, sizeof(unsigned char), L*M*N, fout) < ((size_t)L*M*N)) {
  	PrintInfoMessage("Error writing to output file. Abort.\n");
  	exit(1);
  }
  fclose(fout);

  // free the allocated memory
  delete [] vol;
  PrintInfoMessage("done.\n");
  PrintInfoMessage("ALL OK.\n");
  return 0;
}

