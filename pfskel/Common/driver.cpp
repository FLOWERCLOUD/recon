#include <stdio.h>
#include "common.h"
#include "dynamicArray.h"

#define NROPTS 2

#define L  87
#define M  33
#define N  56

typedef struct {
  int a;
  float b;
  double c;
  char abc[5];
} abc;

int main(int argc, char *argv[]) {
  
  DynamicArray<abc> arr(7);
  
  abc elem;
  int i, j;
  
  for(i=0; i < 100; i++) {
    elem.a = i;
    elem.b = i;
    elem.c = i;
    for(j=0; j < 5; j++) {
      elem.abc[j] = i+j;
    }
    
    arr.Append(elem);
  }

  for(i=0; i < 10; i++) {
    arr.RemoveLastElem();
  }
  
  arr.Fit();
  
  // print
  for(i=0; i < arr.GetNrElem(); i++) {
    printf(" Elem(%d): [%d %f %lf (",
	   i, arr[i].a, arr[i].b, arr[i].c);
    
    for(j=0; j < 5; j++) {
      printf(" %d", arr[i].abc[j]);
    }
    printf(")]\n");
    
  }

  printf("COPY:\n");
  
  DynamicArray<abc> copy;
  copy = arr;
  arr.Reset();


  for(i=0; i < copy.GetNrElem(); i++) {
    printf(" Elem(%d): [%d %f %lf (",
	   i, copy[i].a, copy[i].b, copy[i].c);
    
    for(j=0; j < 5; j++) {
      printf(" %d", copy[i].abc[j]);
    }
    printf(")]\n");
    
  }

  printf("REMOVE: element 17 from copy\n");
  copy.Remove(17);
  
  // print copy
  for(i=0; i < copy.GetNrElem(); i++) {
    printf(" Elem(%d): [%d %f %lf (",
	   i, copy[i].a, copy[i].b, copy[i].c);
    
    for(j=0; j < 5; j++) {
      printf(" %d", copy[i].abc[j]);
    }
    printf(")]\n");
    
  }
  
  printf("REMOVE: element 0 from copy\n");
  copy.Remove(0);
  
  // print copy
  for(i=0; i < copy.GetNrElem(); i++) {
    printf(" Elem(%d): [%d %f %lf (",
	   i, copy[i].a, copy[i].b, copy[i].c);
    
    for(j=0; j < 5; j++) {
      printf(" %d", copy[i].abc[j]);
    }
    printf(")]\n");
    
  }

  printf("REMOVE: last element from copy\n");
  copy.Remove(copy.GetNrElem() - 1);
  
  // print copy
  for(i=0; i < copy.GetNrElem(); i++) {
    printf(" Elem(%d): [%d %f %lf (",
	   i, copy[i].a, copy[i].b, copy[i].c);
    
    for(j=0; j < 5; j++) {
      printf(" %d", copy[i].abc[j]);
    }
    printf(")]\n");
    
  }

  return 0;
}
