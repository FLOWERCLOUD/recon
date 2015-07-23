//////////////////////////////////////////////////////////////////////////////
// ----  Computes the skeleton of a volume, using the potential field method.
//
// Input:	volume file name
//		size of volume
//		distance from object boundary where to place the charges (>=0)
//		potential field strength (1 .. 10)
//		output file name
//
// Output:	skeleton points/segments
//
// Last change: by Nicu D. Cornea
//
//////////////////////////////////////////////////////////////////////////////

// version 
#define pfSkel_Version "1.3"

char skelOutputFormat = 0;

#include "pfSkel.h"

// #define TRACE

// callback functions for the pfSkel module
// for interactive mode
bool ChgParams(Skeleton* Skel, pfSkelCommand *cmd, void* other);  
// for non interactive mode
bool NotInteractive(Skeleton* Skel, pfSkelCommand *cmd, void* other);



int main(int argc, char *argv[]) {
  /*
  printf("Roundoff error may place some points outside the original object\n\
also, charges distance combined with low fs values drives some segments \n\
outside\n");    
  */

// "
// return 1;

  int L,M,N;         // Sizes in x,y,z dimensions

  int distCharges, fieldStrength;
  float  percHDPts;
  

  bool interactive;
  char *vectorfieldinputfile, *vectorfieldoutputfile;
  char *cL, *cM, *cN;

  cbChangeParameters chgParamsFunction = NULL;
  void* chgParamsArg = NULL;

  HDSelection hdSel = HDS_LocMin;
  PFNorm norm = PF_NORM_L2;
  char *phds, *pvn;

  char *skelOutFile, *volInFile;
  bool remOutSegments;
  FieldType fType = PF;
  bool useDistanceField = false;
  
  // print out version information
  printf("\npfSkel - version %s\n", pfSkel_Version);

  SetStartTime();

  if (argc < 2) {
    printf("\
Usage: \n\
  %s <volFile> [Options].\n\
 \n\
Parameters:\n\
   <volfile>       - input volume file name. \n\
\n\
[Options]. Valid options are:\n\
   -fieldtype | -ft <type>                -field type. Possible values:\n\
                                            PF  - potential field.\n\
                                            GDF - gradient diffusion field.\n\
                                            Default: PF\n\
   -fieldstrength | -fs <n>               - field strength (3 to 10).\n\
                                             Default: 6\n\
   -percenthd | -phd <n>                  - percentage of high divergence \n\
                                             points to be used (0 to 100).\n\
                                             Default: 0\n\
   -distcharges | -dc <n>                 - place charges <n> voxels away \n\
                                             from real object boundary.\n\
                                             Default: 1\n\
   -size | -s <sizeX> <sizeY> <sizeZ>     - specify size of input volume.\n\
                                             Default: get from file name.\n\
   -interactive | -int                    - interactive mode. \n\
   -vectorfieldinputfile | -vfin <file>   - load vector field from <file>. \n\
   -vectorfieldoutputfile | -vfout <file> - dump vector field to <file>.  \n\
   -outputformat | -of <format>           - format of skeleton output. \n\
                                             <format> = 0 - points.\n\
                                                      = 1 - line segments.\n\
                                                      = 2 - full structure.\n\
                                             Default: 0\n\
   -hdselection | -hds <t>                - high divergence points selection\n\
                                             <t> = all    - all points.\n\
                                                 = locmin - local min.only.\n\
                                             Default: locmin\n\
   -vectornorm | -vn <n>                  - vector norm to be used.\n\
                                             <n> = L1 - faster\n\
                                                 = L2 - more accurate\n\
                                             Default: L2\n\
   -skeletonoutputfile | -out <skelfile>  - root name of output file.\n\
                                             Default: \n\
                                               <volFile-root>-dc<n>-fs<n>\n\
   -removepoutsegments | -ros             - remove segments that go outside \n\
                                             the original object.\n\
   -usedistfield | -udf                   - use distance field\n\
                                            IGNORED !!\n\
\n\
", argv[0]); //"

/* Options to add next
   -halfboundarypoints | -hbp = use only half of the boundary points to \n\
      place charges - faster but less accurate. IGNORED\n\

*/
    exit(1);
  }

  // input file name
  volInFile = argv[1];

  //
  // set default values for program parameters
  //
  L 		= 0;
  M 		= 0;
  N 		= 0;
  cL = NULL; 
  cM = NULL; 
  cN = NULL; 

  distCharges 	= 1;
  fieldStrength = 6;
  percHDPts	= 0;

  vectorfieldinputfile = NULL;
  vectorfieldoutputfile = NULL;
  interactive = false;
  skelOutputFormat = 0;
  remOutSegments = false;
  skelOutFile = NULL;
  


  //
  // get program options from the command line.
  //
  Option prgOptions[15];
  // -size
  strcpy(prgOptions[0].shortName, "-s");
  strcpy(prgOptions[0].longName,  "-size");
  prgOptions[0].nrValues = 3;
  // -interactive
  strcpy(prgOptions[1].shortName, "-int");
  strcpy(prgOptions[1].longName,  "-interactive");
  prgOptions[1].nrValues = 0; 
  // -vfin
  strcpy(prgOptions[2].shortName, "-vfin");
  strcpy(prgOptions[2].longName,  "-vectorfieldinputfile");
  prgOptions[2].nrValues = 1;
  // -vfout
  strcpy(prgOptions[3].shortName, "-vfout");
  strcpy(prgOptions[3].longName,  "-vectorfieldoutputfile");
  prgOptions[3].nrValues = 1;
  // -outputformat
  strcpy(prgOptions[4].shortName, "-of");
  strcpy(prgOptions[4].longName,  "-outputformat");
  prgOptions[4].nrValues = 1;
  // -hds
  strcpy(prgOptions[5].shortName, "-hds");
  strcpy(prgOptions[5].longName,  "-hdselection");
  prgOptions[5].nrValues = 1;
  // -norm
  strcpy(prgOptions[6].shortName, "-vn");
  strcpy(prgOptions[6].longName,  "-vectornorm");
  prgOptions[6].nrValues = 1;
  // -skeletonoutputfile
  strcpy(prgOptions[7].shortName, "-out");
  strcpy(prgOptions[7].longName,  "-skeletonoutputfile");
  prgOptions[7].nrValues = 1;
  // -removeoutsegments
  strcpy(prgOptions[8].shortName, "-ros");
  strcpy(prgOptions[8].longName,  "-removeoutsegments");
  prgOptions[8].nrValues = 0;
  // -distchanges
  strcpy(prgOptions[9].shortName, "-dc");
  strcpy(prgOptions[9].longName,  "-distcharges");
  prgOptions[9].nrValues = 1;
  // -fieldstrength
  strcpy(prgOptions[10].shortName, "-fs");
  strcpy(prgOptions[10].longName,  "-fieldstrength");
  prgOptions[10].nrValues = 1;
  // -percenthd
  strcpy(prgOptions[11].shortName, "-phd");
  strcpy(prgOptions[11].longName,  "-percenthd");
  prgOptions[11].nrValues = 1;
  // -fieldtype
  strcpy(prgOptions[12].shortName, "-ft");
  strcpy(prgOptions[12].longName,  "-fieldtype");
  prgOptions[12].nrValues = 1;
  // -usedistancefield
  strcpy(prgOptions[13].shortName, "-udf");
  strcpy(prgOptions[13].longName,  "-usedistancefield");
  prgOptions[13].nrValues = 0;

  GetProgramOptions(argv, argc, 2, prgOptions, 14);

  //
  // import optional parameters
  //
  // -size
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
    GetSizeFromFilename(volInFile, &L, &M, &N);
  }
  // if one of the dimensions id still <= 0, something is not right - abort
  if((L <= 0) || (M <= 0) || (N <= 0)) {
    PrintErrorMessage("Could not determine size of input volume.\n");
    return 1;
  }

  // -interactive
  if(prgOptions[1].found) {
    interactive = true;
  }
  // -vfin
  if((prgOptions[2].found) && (prgOptions[2].nrValues == 1)) {
    vectorfieldinputfile = prgOptions[2].values[0];
  }
  // -vfout
  if((prgOptions[3].found) && (prgOptions[3].nrValues == 1)) {
    vectorfieldoutputfile = prgOptions[3].values[0];
  }
  // -outputformat
  if((prgOptions[4].found) && (prgOptions[4].nrValues == 1)) {
    if(strcmp(prgOptions[4].values[0], "0") == 0) {
      skelOutputFormat = 0;
    }
    else {
      if(strcmp(prgOptions[4].values[0], "1") == 0) {
	skelOutputFormat = 1;
      }
      else {
	if(strcmp(prgOptions[4].values[0], "2") == 0) {
	  skelOutputFormat = 2;
	}
	else {
	  printf("\
Unrecognized output format specification: %s. Using default value: 0.\n",
		 prgOptions[4].values[0]);
	  skelOutputFormat = 0;
	}
      }
    }
  }
  // -hds
  if((prgOptions[5].found) && (prgOptions[5].nrValues == 1)) {
    phds = prgOptions[5].values[0];
    if(strcmp(phds, "all") == 0) {
      hdSel = HDS_All;
    }
    else {
      if(strcmp(phds, "locmin") == 0) {
	hdSel = HDS_LocMin;
      } 
      else {
        printf("\
Unrecognized hdselection specification: %s. Using default value: locmin.\n",
               phds);
        hdSel = HDS_LocMin;
      }
    }
  }
  // -vectornorm
  if((prgOptions[6].found) && (prgOptions[6].nrValues == 1)) {
    pvn = prgOptions[6].values[0];
    if(strcmp(pvn, "L1") == 0) {
      norm = PF_NORM_L1;
    }
    else {
      if(strcmp(pvn, "L2") == 0) {
        norm = PF_NORM_L2;
      } 
      else {
        printf("\
Unrecognized norm specification: %s. Using default value: L2.\n",
               pvn);
        norm = PF_NORM_L2;
      }
    }
  }

  // -skeletonoutputfile
  if((prgOptions[7].found) && (prgOptions[7].nrValues == 1)) {
    skelOutFile = prgOptions[7].values[0];
  }

  // -removeoutsegments
  if(prgOptions[8].found) {
    remOutSegments = true;
  }

  // -distcharges
  if((prgOptions[9].found) && (prgOptions[9].nrValues == 1)) {
    distCharges = atoi(prgOptions[9].values[0]);
  }
  // -fieldstrength
  if((prgOptions[10].found) && (prgOptions[10].nrValues == 1)) {
    fieldStrength = atoi(prgOptions[10].values[0]);
  }
  // -percenthd
  if((prgOptions[11].found) && (prgOptions[11].nrValues == 1)) {
    percHDPts = (float) atof(prgOptions[11].values[0]);
  } 
  // -fieldtype
  if((prgOptions[12].found) && (prgOptions[12].nrValues == 1)) {
    if(strcmp(prgOptions[12].values[0], "PF") == 0) {
      fType = PF;
    }
    else {
      if(strcmp(prgOptions[12].values[0], "GDF") == 0) {
	fType = GDF;
      }
      else {
	printf("\
Unrecognized field type specification: %s. Using default value: PF.\n",
               prgOptions[12].values[0]);
        fType = PF;
      }
    }
  }
  // - usedistancefield
  if(prgOptions[13].found) {
    useDistanceField = true;
  }

  //
  // build output file name
  //
  if(skelOutFile == NULL) {
    // build the output file name
    BuildOutputRootFileName(volInFile, distCharges, fieldStrength, 
			    &skelOutFile);
  }
 
  //
  // print out program parameters:
  //
  printf("- input file: %s. Size: %d %d %d\n", volInFile, L, M, N);
  
  if(interactive) {
    printf("- interactive mode\n");
  }
  if(vectorfieldinputfile != NULL) {
    printf("- read vector field from file: %s.\n", vectorfieldinputfile);
  }
  if(vectorfieldoutputfile != NULL) {
    printf("- save vector field to file: %s.\n", vectorfieldoutputfile);
  }
  printf("- skeleton output format: %d.\n", skelOutputFormat);
  printf("- high divergence points selection method: ");
  switch(hdSel) {
  case HDS_LocMin: 
    printf("local minima.\n");
    break;
  case HDS_All:
    printf("all points.\n");
    break;
  default:
    printf("UNKNOWN !!!???.\n");
    return 1;
  }
  printf("- vector norm to be used: ");
  switch(norm) {
  case PF_NORM_L1: 
    printf("L1 - Manhattan (city block) distance.\n");
    break;
  case PF_NORM_L2:
    printf("L2 - Euclidean distance.\n");
    break;
  default:
    printf("UNKNOWN !!!???.\n");
    return 1;
  }
  printf("- output file name (root): %s.\n", skelOutFile);
  printf("- charges distance: %d\n", distCharges);
  printf("- field strength: %d\n", fieldStrength);
  printf("- percent HD points: %f\n", percHDPts);
  if(remOutSegments) {
    printf("- remove segments going outside the original object.\n");
  }
  else {
    printf("- keep segments going outside the original object.\n");
  }

  //
  // setup interactive mode 
  //
  if(interactive) {
#ifdef _DEBUG
    printf("Interactive mode.\n");
#endif
    chgParamsFunction = &ChgParams;
    chgParamsArg = (void*)skelOutFile;
  }
  else {
    // not interactive
    chgParamsFunction = &NotInteractive;
    chgParamsArg = (void*)skelOutFile;
  }

  // The Skeleton.
  Skeleton Skel;
  
  //
  // compute the skeleton
  //
  pfSkel(volInFile, L, M, N,
	 distCharges,
	 fieldStrength,
	 percHDPts, 
	 &Skel, 
	 chgParamsFunction,
	 chgParamsArg, 
	 vectorfieldinputfile,
	 vectorfieldoutputfile, 
	 norm,
	 hdSel,
	 remOutSegments,
	 fType,
	 useDistanceField);
  
  printf("Done.\n");
  PrintElapsedTime("");
  return 0;
}


//
// callback function for the pfSkel module for interactive mode
//   This function is called from inside pfSkel function
//      when the computation is done and it allows changing of the parameters

bool ChgParams(Skeleton *Skel, pfSkelCommand *cmd, void *other) {
  char c[2];
  float newHD; // , newHC;

#ifdef TRACE
  printf("Start ChgParams\n");
  printf("Skel = %p, cmd = %p, other = %p\n", Skel, cmd, other);
#endif

  if(cmd == NULL) {
    return false;
  }
  if(other == NULL) {
    return false;
  } 

  // first thing: save the current skeleton.
  // cmd->newHDP contains the current value for the high divergence percentage
  // other should be a pointer to the filename
  //
  newHD = cmd->HDP;

#ifdef TRACE
  printf("Base output file name: %s\n", (char*)other);
#endif
  
  char *outFile = NULL;
  int len = strlen((char*)other) + 20;

#ifdef TRACE
  printf("Allocating %d chars for new file name.\n", len);
#endif

  if((outFile = new char[len]) == NULL) {
    printf("Error allocating memory for the working data structures. Abort.\n\
");
    return false;
  }
  

  sprintf(outFile, "%s-hd%3.2f.skel", (char*)other, newHD);
  PrintDebugMessage("Saving skeleton to file %s ...", outFile);
  Skel->SaveToFile(outFile, skelOutputFormat);

#ifdef TRACE
  printf("Deleting outFile....\n");
#endif
  delete [] outFile;

  PrintDebugMessage("done.\n");

  
  //
  // present a menu
  //
  printf("Available commands: \n");
  printf("\
\tq - quit\n\
\tp - change parameters (you will be prompted for new values)\n");
  printf("Command: ");
  
  while(true) {
    scanf("%1c", &c);
    switch(c[0]) {
    case 'q':
      // quit
      cmd->cmdCode[0] = c[0];
      return false;
    case 'p':      
      cmd->cmdCode[0] = c[0];
      printf("New high divergence percentage (current value: %f): ", 
	     cmd->HDP);
      scanf("%f", &newHD);
      cmd->HDP = newHD;
      getchar();
      return true;
    default:
      printf("Invalid command: %c. See available commands above\nCommand: ",c);
    }
  }
  
  return false;
}



//
// callback function for the pfSkel module for non interactive mode
//   This function is called from inside pfSkel function
//      when the computation is done and it allows saving the skeleton

bool NotInteractive(Skeleton *Skel, pfSkelCommand *cmd, void *other) {
  float newHD; // newHC;

#ifdef TRACE
  printf("Start Not Interactive\n");
  printf("Skel = %p, cmd = %p, other = %p\n", Skel, cmd, other);
#endif

  if(cmd == NULL) {
    return false;
  }
  if(other == NULL) {
    return false;
  } 

  // first thing: save the current skeleton.
  // cmd->newHDP contains the current value for the high divergence percentage
  // other should be a pointer to the filename
  //
  newHD = cmd->HDP;

#ifdef TRACE
  printf("Base output file name: %s\n", (char*)other);
#endif
  
  char *outFile = NULL;
  int len = strlen((char*)other) + 20;

#ifdef TRACE
  printf("Allocating %d chars for new file name.\n", len);
#endif

  if((outFile = new char[len]) == NULL) {
    printf("Error allocating memory for the working data structures. Abort.\n\
");
    return false;
  }
  

  sprintf(outFile, "%s-hd%3.2f.skel", (char*)other, newHD);
  printf("Saving skeleton to file %s ...\n", outFile);
  Skel->SaveToFile(outFile, skelOutputFormat);

#ifdef TRACE
  //  printf("Deleting outFile....\n");
#endif
  delete [] outFile;

  printf("done.\n");

  
  //
  // no menu - just give quit command
  //
  cmd->cmdCode[0] = 'q';
  return false;
}

