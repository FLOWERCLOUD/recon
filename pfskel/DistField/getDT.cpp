// ----
// ----  Compute distance transform
// ----  Uses Toriwaki and Saito's DT algorithm. 
// ----  
// ----  Original version by : Nikhil Gagvani, Vizlab, Rutgers University
// ----  Modified by Carlos Correa and Nicu Cornea, Vizlab, Rutgers Univ.
//

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "common.h"
#include "getDT.h"

bool GetDT(unsigned char *cf, int L, int M, int N, float *f) {

  int i,j,k,n;
  float *buff , df, db, d, w;
  long idx, slsz, sz;
  //int measureTime = 0;

  if(cf == NULL) return false;
  if(f == NULL) return false;

  slsz = L*M;		// slice size
  sz = slsz*N;
  
  printf("Preprocessing phase..."); fflush(stdout);
  long idxz = 0;
  long idxy, idxx;
  long offsetx = 1;
  long offsety = L;
  long offsetz = slsz;

  for (idx = 0; idx < slsz*N; idx++) {
    if (cf[idx] > 0) {
      f[idx] = 99999;
    } else {
      f[idx] = 0;
    }
  }

  int maxdim = MAX(L,M);
  maxdim = MAX(maxdim,N);
  buff = new float[maxdim+10];

  // Using Algorithm 3 from Appendix 
  
  // Step 1  forward scan
  printf("Phase 1 FWD..."); fflush(stdout);

  idxz = 0;
  for (k = 0; k < N; k++) {
    idxy = idxz;
    for (j = 0; j < M; j++) {
      idxx = idxy;
      df = (float) L;
      for (i = 0; i < L; i++) {
	idx=idxx;
	//idx = k*slsz + j*L + i;
	df = (f[idx]!=0)? df + 1: 0;
	f[idx] = df*df;
	idxx+=offsetx;
      }
      idxy+=offsety;
    }
    idxz+=offsetz;
  }
 
  printf("BCK..."); fflush(stdout);
  //  Step 1 backward scan

  idxz = 0;
  for (k = 0; k < N; k++) {
    idxy = idxz;
    for (j = 0; j < M; j++) {
      idxx = idxy+L-1;
      db = (float) L;
      for (i = L-1; i >=0; i--) {
	idx=idxx;
        //idx = k*slsz + j*L + i;
	db = (f[idx]!=0)? db + 1 : 0;
	f[idx] = MIN(f[idx], db*db);
	idxx-=offsetx;
      }
      idxy+=offsety;
    }
    idxz+=offsetz;
  }

  printf("[DONE]\n");
  printf("Phase 2...");

  // Step 2
  idxz = 0;
  for (k = 0; k < N; k++) {
    idxx = idxz;
    for (i = 0; i < L; i++) {
      idxy = idxx;
      for (j =0; j < M; j++) {
	idx = idxy;
	  //k*slsz + j*L +i
        buff[j] = f[idx];
	idxy+=offsety;
      }
    
      idxy=idxx;
      for (j = 0; j < M; j++) {
        d = buff[j];
        if (d != 0) {
          int rmax, rstart, rend;
          rmax = (int) floor(sqrt(d)) + 1;
          rstart = MIN(rmax, (j-1));
          rend = MIN(rmax, (M-j));
          for (n = -rstart; n < rend; n++) {
	    if (j+n >= 0 && j+n < M) {
	      w = buff[j+n] + n*n;
	      if (w < d)  d = w;
	    }
          }
        }
        //idx = k*slsz + j*L +i;
	idx = idxy;
        f[idx] = d;
	idxy+=offsety;
      }
      idxx+=offsetx;
    }
    idxz+=offsetz;
    printf("."); fflush(stdout);
  }

  printf("[DONE]\n");

  // Step 3


    printf("Phase 3..."); fflush(stdout);

    for (j = 0; j < M; j++) {
      for (i = 0; i < L; i++) {
	for (k =0; k < N; k++) {
	  buff[k] = f[k*slsz + j*L +i];
	}
	for (k = 0; k < N; k++) {
	  d = buff[k];
	  if (d != 0) {
	    int rmax, rstart, rend;
	    rmax = (int) floor(sqrt(d)) + 1;
	    rstart = MIN(rmax, (k-1));
	    rend = MIN(rmax, (N-k));
	    for (n = -rstart; n < rend; n++) {
              if (k+n >= 0 && k+n < N) {		
                w = buff[k+n] + n*n;
                if (w < d)  d = w;
              }
	    }
	  }
	  idx = k*slsz + j*L +i;
	  f[idx] = d;
	}
      }
      printf("."); fflush(stdout);
    }
    printf("[DONE]\n");
  
    // extract square root from each entry
    for(idx=0; idx < L*M*N; idx++) {
      if(f[idx] != 0.00) {
	f[idx] = sqrt(f[idx]);
      }
    }

    delete[] buff;

    return true;
}




///////////////////////////////////////////////////////////////////////////////
// read distance field data from a given file
///////////////////////////////////////////////////////////////////////////////
bool ReadDistanceField(char *filename, int L, int M, int N, float **df) {
  FILE* fdf;
  size_t read;
  
  (*df) = NULL;

  if ((fdf = fopen(filename,"rb")) == NULL) {
    printf("\nCannot open input file %s\n", filename);
    return false;
  }
  
  if(((*df) = new float[L*M*N]) == NULL) {
    printf("\nError allocating memory for the volume. Not enough memory ?\n");
    return false;
  }
  
  read = fread((*df), sizeof(float), L*M*N, fdf);
  if ( read < ((size_t) L*M*N)) {
    printf("\n\
Read only %d values before the end of input file. Expected: %d.\n", 
	   read, L*M*N);
    delete [] (*df);
    (*df) = NULL;
    return false;
  }

  fclose(fdf);
  return true;
}


///////////////////////////////////////////////////////////////////////////////
// write distance field data to a given file
///////////////////////////////////////////////////////////////////////////////
bool SaveDistanceField(char *filename, int L, int M, int N, float *df) {
  FILE* fdf;
  size_t wrote;
  
  if ((fdf = fopen(filename,"wb")) == NULL) {
    printf("\nCannot open output file %s\n", filename);
    return false;
  }
  
  wrote = fwrite(df, sizeof(float), L*M*N, fdf);
  if ( wrote < ((size_t) L*M*N)) {
    printf("\n\
Wrote only %d values to output file. Expected: %d.\n", 
	   wrote, L*M*N);
    return false;
  }
  
  fclose(fdf);
  return true;
}

