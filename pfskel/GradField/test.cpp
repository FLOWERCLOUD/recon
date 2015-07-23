#include "gradField.h"
#include <stdio.h>

int main(int argc, char *argv[]) {
  unsigned char *vol;
  int L, M, N;
  ForceVector *fv;
  int i;
  char outFile[100];

  GetSizeFromFilename(argv[1], &L, &M, &N);
  printf("Size: %dx%dx%d.\n", L, M, N);

  ReadVolume(argv[1], L, M, N, &vol);
  
  fv = new ForceVector[L*M*N];
  sprintf(outFile, "out.%dx%dx%d.gdf", L, M, N);
  
  for(i=0; i < 20; i++) {
    printf("Smoothing steps: %d.\n", i);
    CalculateGradientField(L, M, N, vol, fv, i);
    
    SaveVectorField(fv, L, M, N, outFile);
    getchar();
  }

  delete [] fv;
  return 0;
}
