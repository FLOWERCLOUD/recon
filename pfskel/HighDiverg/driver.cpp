//
// driver program for the HighDiverg module
//

#include "common.h"
#include "highDiverg.h"

#include <fstream>
#include <iostream>

int main(int argc, char *argv[])
{
  int L,M,N;         // Sizes in x,y,z dimensions
  char *cL, *cM, *cN;
  ForceVector *fv;

  DynamicArray<DivergencePoint> HDPoints(100);

  long slsz;

  unsigned char *f;
  float perc;
  bool inOut = false;
  bool allHD = true;

  char *sInFile, *sOutFile, *sFieldFile, *sHDPerc;

#ifdef _DEBUG
  SetStartTime();
#endif

  if (argc < 5)
  {
    printf("\n\
Usage: \n\
  %s <volFile> <vector file> <perc> <dSeedsFile> [Options].\n\
\n\
Options:\n\
  -s <sX> <sY> <sZ> = specify size of input volume.\n\
                      DEFAULT: get size from file name <volume file>.\n\
  -out <flag>       = specifies whether you want to look for divergence seeds\n\
                      outside the object too, not just inside.\n\
                      <flag> = 0 - inside only\n\
                             = 1 - inside and outside.\n\
                      DEFAULT: 0.\n\
  -dsel <dsel>    = how to select divergence seeds:\n\
                      <hdsel> = all    - from all points. \n\
                              = locmin - from local minima only.\n\
                      DEFAULT: all.\n\
", argv[0]);
    exit(1);
  }
  
  sInFile = argv[1];
  sFieldFile = argv[2];
  sHDPerc = argv[3];
  sOutFile = argv[4];

  L = 0; M = 0; N = 0;
  inOut = false;
  allHD = false;

 //
  // get program options from the command line.
  //
  Option prgOptions[3];
  // -size
  strcpy(prgOptions[0].shortName, "-s");
  strcpy(prgOptions[0].longName, "");
  prgOptions[0].nrValues = 3;
  // -vn
  strcpy(prgOptions[1].shortName, "-dsel");
  strcpy(prgOptions[1].longName, "");
  prgOptions[1].nrValues = 1; 
  // -out
  strcpy(prgOptions[2].shortName, "-out");
  strcpy(prgOptions[2].longName, "");
  prgOptions[2].nrValues = 1;
  

  GetProgramOptions(argv, argc, 5, prgOptions, 3);

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
  
  // hdsel
  allHD = true;
  if((prgOptions[1].found) && (prgOptions[1].nrValues == 1)) {
    if(strcmp(prgOptions[1].values[0], "all") == 0) {
      // allHD = true;
    }
    else {
      if(strcmp(prgOptions[1].values[0], "locmin") == 0) {
	allHD = false;
      }
      else {
	printf("\
Option -hdsel: Unrecognized specification. Defaulting to all.\n");
      }
    }
  }
  // inOut
  inOut = false;
  if((prgOptions[2].found) && (prgOptions[2].nrValues == 1)) {
    if(strcmp(prgOptions[2].values[0], "1") == 0) {
      inOut = true;
    }
  }

  // size determined successfuly
  printf("Input dataset: %s. Size: %dx%dx%d.\n", sInFile, L, M, N);


  perc = atof(sHDPerc);

  slsz = L*M;		// slice size

  // read volume
  ReadVolume(sInFile, L, M, N, &f);

  // set flags
  FlagVolume(f, L, M, N);


#ifdef _DEBUG
  PrintElapsedTime("Phase1: reading volume and setting flags");
#endif


 if((fv = new ForceVector[L*M*N]) == NULL) {
   printf("OOPS! - Error allocating memory for the input vector field. Abort.\n");
   exit(1);
  }

 ReadVectorField(fv, L, M, N, sFieldFile);

#ifdef TRACE
 for(idx = 0; idx < L*M*N; idx++) {
   if(fv[idx].xd !=0) {
     printf("NOT zero idx = %d!!!\n", idx);
     break;
   }
 }
#endif
  
#ifdef _DEBUG
  PrintElapsedTime("Phase2: reading in force vectors");
#endif

  /*
  printf("Optional step: smooth vector field\n");
  SmoothVectorField(fv, L, M, N);

#ifdef _DEBUG
  PrintElapsedTime("Phase2.1 - optional: smooth vector field");
#endif
  */

  HDSelection hdSel = HDS_LocMin;
  if(allHD) {
    hdSel = HDS_All;
  }


  GetHighDivergencePoints(fv, L, M, N, f, perc, HDPoints, inOut, hdSel);

#ifdef _DEBUG
  PrintElapsedTime("Phase4: Getting the divergence seeds.");
  printf("**  %d divergence seeds returned.\n", HDPoints.GetNrElem());
#endif

  delete [] f;
  delete [] fv;

  SaveHDPoints(sOutFile, HDPoints);
  
  
#ifdef _DEBUG
  PrintElapsedTime("Phase5: Writing output file");
#endif
  printf("End \n");
  return 0;
}
