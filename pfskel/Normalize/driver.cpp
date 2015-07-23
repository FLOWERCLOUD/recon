///////////////////////////////////////////////////////////////////////////////
// driver program for the normalize module
///////////////////////////////////////////////////////////////////////////////

#include "common.h"
#include "normalize.h"

int main(int argc, char *argv[])
{
  int L, M, N;         // Sizes in x,y,z dimensions
  char *cL, *cM, *cN;
  int i;
  unsigned char *f;    // volume
  long idx, slsz, sz;  

  int sphereDiameter;  
  int padding;

  char *sInFile;
  char sOutFile[200];
  char srootOutFile[200];
  char *p;


#ifdef _DEBUG
	SetStartTime();
#endif

  if (argc < 2) {
    printf("\n\
Normalize a 3D object = include it in a sphere of specified diameter.\n\
Usage: \n\
  %s <volFile> [Options] \n\
Options:\n\
  -o <outRootName>    = specify the root name of the output file. \n\
                        The size of the new volume will be appended to it + \n\
                        the .vol extension.\n\
                        DEFAULT: extract the root name from the input file.\n\
  -s <sX> <sY> <sZ>   = specify size of input volume.\n\
                        DEFAULT: get size from file name <volFile>.\n\
  -d <sphereDiameter> = specify diameter of enclosing sphere in voxels.\n\
                        DEFAULT: 100\n\
  -p <padding>        = specifies the number of voxels to be added \n\
                        between the sphere enclosing the object and the \n\
                        bounding box of the resulting volume.\n\
                        DEFAULT: 1.\n\
", argv[0]);
    exit(1);
  }

  sInFile = argv[1];
  sphereDiameter = 100;
  padding = 1;
  strcpy(sOutFile, "");
  L = 0; M = 0; N = 0;

  // extract the root of the output file name from the input file
  // find the first point in the file name
  p = strchr(sInFile, '.');
  if(p != NULL) {
    strncpy(srootOutFile, sInFile, p - sInFile + 1);
    srootOutFile[p - sInFile + 1] = '\0';
  }

  //
  // get program options from the command line.
  //
  Option prgOptions[4];
  // -size
  strcpy(prgOptions[0].shortName, "-s");
  strcpy(prgOptions[0].longName, "");
  prgOptions[0].nrValues = 3;
  // -d
  strcpy(prgOptions[1].shortName, "-d");
  strcpy(prgOptions[1].longName, "");
  prgOptions[1].nrValues = 1; 
  // -p
  strcpy(prgOptions[2].shortName, "-p");
  strcpy(prgOptions[2].longName, "");
  prgOptions[2].nrValues = 1;
  // -o
  strcpy(prgOptions[3].shortName, "-o");
  strcpy(prgOptions[3].longName, "");
  prgOptions[3].nrValues = 1;

  GetProgramOptions(argv, argc, 2, prgOptions, 4);

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
  
  // sphere diameter
  if((prgOptions[1].found) && (prgOptions[1].nrValues == 1)) {
    sphereDiameter = atoi(prgOptions[1].values[0]); 
    if(sphereDiameter <= 0) {
      PrintErrorMessage("Invalid sphere diameter.\n");
      return 1;
    }
  }

  // padding
  if((prgOptions[2].found) && (prgOptions[2].nrValues == 1)) {
    padding = atoi(prgOptions[2].values[0]);
    if(padding < 0) {
      PrintErrorMessage("Invalid sphere diameter.\n");
      return 1;
    }
  }

  // out root file name
  if((prgOptions[3].found) && (prgOptions[3].nrValues == 1)) {
    strcpy(srootOutFile, prgOptions[3].values[0]);
  }

  // size determined successfuly
  printf("Input dataset: %s. Size: %dx%dx%d.\n", sInFile, L, M, N);
  printf("sphere diameter: %d, padding: %d\n", sphereDiameter, padding);
  printf("Out root file name: %s.\n", srootOutFile);

  
  //
  // read in the volume
  //
  f = NULL;
  if(!ReadVolume(sInFile, L, M, N, &f)) {
    return 1;
  }

#ifdef _DEBUG
  PrintElapsedTime("Phase 3: reading volume from file.");
#endif

  // call the normalization routine
  if(!NormalizeVolume(&f, &L, &M, &N, sphereDiameter, padding)) 
  {
    PrintErrorMessage("Normalization failed !");
    delete [] f;
    return 1;
  }
  
#ifdef _DEBUG
  PrintElapsedTime("Phase 5: normalization.");
#endif

  // construct output file name
  sprintf(sOutFile, "%s%dx%dx%d.vol", srootOutFile, L, M, N);
  printf("Size of the new volume: %dx%dx%d. Output file: %s.\n", 
	 L, M, N, sOutFile);
  SaveVolume(sOutFile, L, M, N, f);
 
#ifdef _DEBUG
  PrintElapsedTime("Phase 6: saving new volume to file.");
#endif

#ifdef _DEBUG
  PrintElapsedTime("");
#endif

  // free the allocated memory
  delete [] f;

  printf("Done.\n");
  return 0; 
}


