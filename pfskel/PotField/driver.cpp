///////////////////////////////////////////////////////////////////////////////
// driver program for the potVect module
///////////////////////////////////////////////////////////////////////////////

// #include "common.h"
#include "potVect.h"

int main(int argc, char *argv[])
{
  int L, M, N;         // Sizes in x,y,z dimensions
  char *cL, *cM, *cN;
  int i;
  unsigned char *f;    // volume
  ForceVector *force;       // vector field
  long idx, slsz, sz;  

  int fieldStrenght;   // field strength
  bool inOut = false;  // in/out flag
  PFNorm norm;

  char *sInFile, *sOutFile, *sFieldStrength;

#ifdef _DEBUG
	SetStartTime();
#endif

  if (argc < 4) {
    printf("\n\
Computes the potential field of a 3D object.\n\
Usage: \n\
  %s <volfile> <fieldStrenght> <pfFile> [Options] \n\
Options:\n\
  -s <sX> <sY> <sZ>  = specify size of input volume.\n\
                       DEFAULT: get size from file name <VolFile>.\n\
  -vn <normSpec>     = specify vector norm to be used.\n\
                       normSpec = L1 - faster\n\
                                = L2 - more accurate\n\
                       DEFAULT: L2\n\
  -out <flag>        = specifies whether you want the field computed \n\
                       outside the object too, not just inside.\n\
                       flag = 0 - inside only\n\
                            = 1 - inside and outside.\n\
                       DEFAULT: 0.\n\
", argv[0]);
    exit(1);
  }

  sInFile = argv[1];
  sOutFile = argv[3];
  sFieldStrength = argv[2];

  L = 0; M = 0; N = 0;
  fieldStrenght = atoi(sFieldStrength);
  inOut = false;


 //
  // get program options from the command line.
  //
  Option prgOptions[3];
  // -size
  strcpy(prgOptions[0].shortName, "-s");
  strcpy(prgOptions[0].longName, "");
  prgOptions[0].nrValues = 3;
  // -vn
  strcpy(prgOptions[1].shortName, "-vn");
  strcpy(prgOptions[1].longName, "");
  prgOptions[1].nrValues = 1; 
  // -out
  strcpy(prgOptions[2].shortName, "-out");
  strcpy(prgOptions[2].longName, "");
  prgOptions[2].nrValues = 1;
  

  GetProgramOptions(argv, argc, 4, prgOptions, 3);

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
  
  // norm
  norm = PF_NORM_L2;
  if((prgOptions[1].found) && (prgOptions[1].nrValues == 1)) {
    if(strcmp(prgOptions[1].values[0], "L1") == 0) {
      norm = PF_NORM_L1;
    }
    else {
      if(strcmp(prgOptions[1].values[0], "L2") == 0) {
      }
      else {
	printf("\
Option -norm: Unrecognized norm specification. Defaulting to L2.\n");
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
	PrintElapsedTime("Phase 3: reading volume from file.");
#endif

// initialize the f array
  for (idx = 0; idx < sz; idx++) {
	if (f[idx] > 0) {
	   f[idx] = INTERIOR;
	}
	else {
	   f[idx] = 0;
	}
  }



#ifdef _DEBUG
	PrintElapsedTime("Phase 5: initialize the flags.");
#endif

force = new ForceVector[L*M*N];			// potential field

// Compute force vectors

CalculatePotentialField(L, M, N, f, fieldStrenght, force, inOut, norm);



// print the force vectors.
 SaveVectorField(force, L, M, N, sOutFile);
 
#ifdef _DEBUG
	PrintElapsedTime("Phase 6: saving potential field to file.");
#endif



  #ifdef _DEBUG
	PrintElapsedTime("");
  #endif

// analyse the vector field
// find the max and min vector lenghts:
double maxvl, minvl, vl;
int nrmax, nrmin;

maxvl = -0.00;
minvl = 9999.99;
nrmax = 0;
nrmin = 0;
for(i=0; i < L*M*N; i++) {
	if(f[i] == INTERIOR) {
		vl = 	sqrt((force[i].xd * force[i].xd) + (force[i].yd * force[i].yd) + (force[i].zd * force[i].zd));

		if(vl > maxvl) {
			maxvl = vl;
			nrmax = 1;
		}
		else {
			if(vl == maxvl) {
				nrmax = nrmax + 1;
			}
		}

		if(vl < minvl) {
			minvl = vl;
			nrmin = 1;
		}
		else {
			if(vl == minvl) {
				nrmin = nrmin + 1;
			}
		}
	}
}

/*
printf("ForceVector field analysis:\n");
printf("\tMaximum vector length: %f. Number of points with maximum value: %d\n", maxvl, nrmax);
printf("\tMinimum vector length: %f. Number of points with minimum value: %d\n", minvl, nrmin);
printf("--\n");

double perc[7] = {0.00001, 0.00005, 0.00010, 0.0005, 0.0010, 0.0020, 0.0030};
int percnr[7] = {0, 0, 0, 0, 0, 0, 0};
VoxelPosition pts[7][20];

for (k = 0; k < N; k++) {
	for (j = 0; j < M; j++) {
		for (i = 0; i < L; i++) {
			idx = k*slsz+j*L+i;

			if(f[idx] == INTERIOR) {
				vl = 	sqrt((force[idx].xd * force[idx].xd) + (force[idx].yd * force[idx].yd) + (force[idx].zd * force[idx].zd));

				for(s=0; s < 7; s++) {
					if(vl <= (minvl + ((maxvl - minvl) * perc[s] / 100.00))) {
						if(percnr[s] < 20) {
							pts[s][percnr[s]].x = i;
							pts[s][percnr[s]].y = j;
							pts[s][percnr[s]].z = k;
						}
						percnr[s] = percnr[s] + 1;
					}
				}
			}
		}
	}
}

for(j=0; j < 7; j++) {
	printf("\tNumber of points with vector value < %f %% (%f): %d\n", perc[j],
		minvl + ((maxvl - minvl) * perc[j] / 100.00), percnr[j]);
	for(k=0; k < MIN(percnr[j], 20); k++) {
		printf("\t\t(%d, %d, %d)\n", pts[j][k].x, pts[j][k].y, pts[j][k].z);
	}
}
printf("-------------------------------------\n");
*/


// free the allocated memory
delete [] f;
delete [] force;

printf("Done.\n");
 return 0; 
}


