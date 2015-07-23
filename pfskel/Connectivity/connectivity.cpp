#include "connectivity.h"
#include "common.h"
// #include "stack.h"


bool CheckConnectivity(unsigned char *vol, int L, int M, int N) {
  int i, nrCompPoints, nrOV;
  unsigned char compNr;
  unsigned char largestComponent;
  int lcNrVoxels;
  unsigned char objValue = 255;  // this must be the largest number supported 
                                 // by this data type
  
  compNr = 1; // the number of the first component
  // keeps track of the largest component
  largestComponent = 0;
  lcNrVoxels = 0;
  
  // make volume binary
  for(i=0; i < L*M*N; i++) {
    if(vol[i] != 0) vol[i] = objValue;
  }

  // count the number of object voxels
  nrOV = 0;
  for(i = 0; i < L*M*N; i++) {
    if(vol[i] == objValue) {
      nrOV++;
    }
  }
  printf("Total number of object voxels: %d\n", nrOV);

  // find the first object voxel not already assigned to a component
  for(i=0; i < L*M*N; i++) {
    if(vol[i] == objValue) {
      // found the first non-assigned object voxel
      printf("Component %d: ...(counting)...", compNr);
      fflush(stdout);
      // build the <compNr>th component starting from this voxel
      nrCompPoints = FloodFill(vol, L, M, N, objValue, i, compNr);
      printf("\tnr points: %d\n", nrCompPoints);

      // compare with current max
      if(nrCompPoints > lcNrVoxels) {
	lcNrVoxels = nrCompPoints;
	largestComponent = compNr;
      }
      
      // if the current largest component has more than 50% of the total number
      // of voxels, stop searching
      if(lcNrVoxels > (int) (0.50 * nrOV)) {
	printf("Found a component with more than 50%% of the object voxels. \
Will not continue searching for other components.\n");
	break;
      }
      
      // else - continue with the next component
      compNr++;
      
      if(compNr >= objValue) {
	printf("\
CheckConnectivity: too many components in dataset ( > %d)!\n", objValue);
	return false;
      }
    }
  }
  
  // keep the main component is there is one
  if(largestComponent != 0) {
    // color it with object color, and make everything else background
    printf("Keeping component %d as the largest component (%d voxels).\n",
	   largestComponent, lcNrVoxels);
    for(i = 0; i < L*M*N; i++) {
      if(vol[i] != 0) {
	if(vol[i] == largestComponent) {
	  vol[i] = objValue;
	}
	else {
	  vol[i] = 0;
	}
      }
    }
  }
  else {
    printf("Could not identify the largest component.\n");
  }
  
  return true;
}

////////////////////////////////////////////////////////////////////////////
// !! This function will not work if there isn't at least one layer of empty
// voxels between the object and the bounding box.
////////////////////////////////////////////////////////////////////////////
unsigned long FloodFill(
  unsigned char *vol,         // [in] volume
  int L, int M, int N,        // [in] size of volume
  unsigned char objValue,     // [in] value of object voxels
  unsigned long startAt,      // [in] seed position
  unsigned char compNr        // [in] component number
) {
  Stack<unsigned long>  pStack;
  unsigned long i, crtIdx, nrPoints;
  bool done, found;
  int neighbors[26];

  // face 
  neighbors[0] = +1;       neighbors[1] = -1;
  neighbors[2] = +L;       neighbors[3] = -L;
  neighbors[4] = +(L*M);   neighbors[5] = -(L*M);

  // edge
  neighbors[6]  = +(L*M) + 1;       neighbors[7]  = +(L*M) - 1;
  neighbors[8]  = +(L*M) + L;       neighbors[9]  = +(L*M) - L;
  neighbors[10] = -(L*M) + 1;       neighbors[11] = -(L*M) - 1;
  neighbors[12] = -(L*M) + L;       neighbors[13] = -(L*M) - L;
  neighbors[14] = +L + 1;           neighbors[15] = +L - 1;
  neighbors[16] = -L + 1;           neighbors[17] = -L - 1;

  // vertex
  neighbors[18] = +(L*M) + 1 + L;       neighbors[19] = +(L*M) + 1 - L;
  neighbors[20] = +(L*M) - 1 + L;       neighbors[21] = +(L*M) - 1 - L;
  neighbors[22] = -(L*M) + 1 + L;       neighbors[23] = -(L*M) + 1 - L;
  neighbors[24] = -(L*M) - 1 + L;       neighbors[25] = -(L*M) - 1 - L;
  
  crtIdx = startAt;
  // set current point to the component value
  vol[crtIdx] = compNr;

  nrPoints = 1;

  // check the 26 neighbors of this point
  // if any of them is an object voxel, push the current point onto the stack
  //   and move to that neighbor
  // if no neighbor is an object voxel, the current point becomes the first
  //   point in the stack
  //   if the stack is empty, return

  done = false;
  while(!done) {

    found = false;
    i = 0;
    while( (i < 26) && !found) {
      if(vol[crtIdx + neighbors[i]] == objValue) {
	found = true;
	// set neighbor to the component value
	vol[crtIdx + neighbors[i]] = compNr;
	nrPoints++;
	// printf("\rcounting: %d", nrPoints);
	// push current point onto stack
	pStack.Push(&crtIdx);
	// move current point to the neighbor position
	crtIdx = crtIdx + neighbors[i];
      }

      i++;
    }

    if(!found) {
      // no neighbor with object value was found
      done = !pStack.Pop(&crtIdx);
    }
  }

  return nrPoints;
}
