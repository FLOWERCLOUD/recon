#include "common.h"

bool NormalizeVolume(
  unsigned char **vol, int *L, int *M, int *N, // original volume and dimension
  int sphereDiameter = 100, // sphere enclosing the object after normalization
  int boundingBoxDistance = 1 // distance from sphere to the bounding box
) {

  int newL, newM, newN;
  int newIdx, oldIdx, newSlSize, oldSlSize, oldSize;
  int i, j, k;
  int minX, minY, minZ, maxX, maxY, maxZ;
  int crtDiam;
  float ratio, invRatio;
  unsigned char *newVol;
  int dL, dM, dN;

  newL = 0;
  newM = 0;
  newN = 0;
  

  // find the actual extent of the object  
  if(!GetVolExtent(*vol, *L, *M, *N, 
		   &minX, &maxX, &minY, &maxY, &minZ, &maxZ)) 
  {
    printf("Error: unable to get volume extent.\n");
    return false;
  }

  // the diameter of the sphere currently enclosing the object is the biggest
  // of the 3 extents
  crtDiam = maxX - minX + 1;
  if(crtDiam < (maxY - minY + 1)) {
    crtDiam = maxY - minY + 1;
  }
  if(crtDiam < (maxZ - minZ + 1)) {
    crtDiam = maxZ - minZ + 1;
  }

  // the ratio newDiameter / oldDiameter will be the scaling factor applied to 
  // the object
  // scaling is applied uniformly in all 3 directions
  ratio = (float) sphereDiameter / (float) crtDiam;
  
  // compute the size of the new volume
  newL = ((int)((maxX - minX + 1) * ratio)) + 
    boundingBoxDistance + boundingBoxDistance;
  newM = ((int)((maxY - minY + 1) * ratio)) + 
    boundingBoxDistance + boundingBoxDistance;
  newN = ((int)((maxZ - minZ + 1) * ratio)) + 
    boundingBoxDistance + boundingBoxDistance;

  // allocate space for the new volume
  if((newVol = new unsigned char[newL * newM * newN]) == NULL) {
    printf("Normalize: error allocating memory for the new volume !");
    return false;
  }
  memset(newVol, 0, newL * newM * newN);

  // scan the new volume, inverse transform every voxel to the original volume
  // and copy the value from the original
    
  newSlSize = newL * newM;
  oldSlSize = (*L)*(*M);
  oldSize = (*L)*(*M)*(*N);
  // inverse transform scale ratio
  invRatio = 1.00 / ratio;

  // newIdx = 0;
  for(k = 0; k < (newN); k++) {
    for(j = 0; j < (newM); j++) {
      for(i = 0; i < (newL); i++) {

	newIdx = k*newSlSize + j*newL + i;
	
	oldIdx = 
	  ((((int)round((k-boundingBoxDistance)*invRatio))+minZ)*oldSlSize) + 
	  ((((int)round((j-boundingBoxDistance) * invRatio)) + minY) * (*L)) + 
	  ((((int)round((i-boundingBoxDistance) * invRatio)) + minX));

	if((oldIdx < 0) || (oldIdx >= oldSize)) {
	  PrintErrorMessage("Normalize: index is out of bounds !");
	}
	else {
	  newVol[newIdx] = (*vol)[oldIdx];
	}
      }
    }
  }

  // delete the old volume
  delete [] (*vol);
  
  // set vol to point to the new volume
  (*vol) = newVol;
  *L = newL;
  *M = newM;
  *N = newN;
  
  // just to be safe, check the volume padding
  if(!CheckVolumePadding(*vol, *L, *M, *N, boundingBoxDistance)) {
    return PadVolume(vol, L, M, N, boundingBoxDistance,&dL, &dM, &dN);
  }

  return true;
}
