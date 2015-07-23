
///////////////////////////////////////////////////////////////////////////////
// finds the connected components in a volume and keeps the largest one
// everything else is set to 0
///////////////////////////////////////////////////////////////////////////////
bool CheckConnectivity(unsigned char *vol, int L, int M, int N);


///////////////////////////////////////////////////////////////////////////////
// Flood fill object starting at a given voxel, effectively marking a connected
// component.
// Returns the number of voxels in that component.
///////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
// !! This function will not work if there isn't at least one layer of empty
// voxels between the object and the bounding box.
// CHECK VOLUME PADDING BEFORE CALLING THIS FUNCTION 
////////////////////////////////////////////////////////////////////////////
unsigned long FloodFill(
  unsigned char *vol,         // [in] volume
  int L, int M, int N,        // [in] size of volume
  unsigned char objValue,     // [in] value of object voxels
  unsigned long startAt,      // [in] seed position
  unsigned char compNr        // [in] component number
);   
