// NormalizeVol:
//   Normalize an object by enclosing it into a sphere of the specified radius
//   The bounding box of the object is then set to enclose the sphere and 
//     be at the specified distance (in voxels) from it.
// The volume is changed by this function:
//  - the old volume is deleted using delete []

bool NormalizeVolume(
  unsigned char **vol, int *L, int *M, int *N,// original volume and dimensions
  int sphereRadius = 100,   // sphere enclosing the object after normalization
  int boundingBoxDistance = 1 // distance from sphere to the bounding box
);

