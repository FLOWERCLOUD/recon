#ifndef NCD_SKEL_STACK_DEFINED
#define NCD_SKEL_STACK_DEFINED


#include <stdlib.h>
#include <stdio.h>

///////////////////////////////////////////////////////////////////////////////
// Stack implementation
///////////////////////////////////////////////////////////////////////////////

template<class T>
class Stack {
 public:
	Stack() {
		this->incr = 500;
		if((this->values = new T[this->incr]) == NULL) {
			printf("Stack: not enough memory !\n");
		}
		this->height = this->incr;
		this->top = -1;
	 }

  ~Stack();

  bool Push(T *val);
  bool Pop(T *val);
	bool IsEmpty();

 private:
  T *values;
  long height;
  long top;
  int incr;
};



template <class T>
Stack<T>::~Stack() {  
  if(this->values != NULL) {
    delete [] this->values;
  }
  
  this->values = NULL;
  this->height = 0;
  this->top = -1;
}


template <class T>
bool Stack<T>::Push(T *val) {
  if(val == NULL) {
    printf("Stack::Push: invalid parameters !\n");
    return false;
  }  
  
  this->top++;

  if(this->top >= this->height) {
    // need to increase size
    T *newVals;
    long i;

    // allocate new array
    if((newVals = new T[this->height + this->incr]) == NULL) {
      printf("Stack::Push: not enough memory !\n");
      return false;
    }

    // copy old values
    for(i=0; i < this->height; i++) {
      newVals[i] = this->values[i];
    }

    // delete old array
    delete [] this->values;

    // link new array
    this->values = newVals;
    this->height = this->height + this->incr;    
  }

  // copy the point at the top of the stack
  this->values[this->top] = *val;
  
  return true;
}


template <class T>
bool Stack<T>::Pop(T *val) {
  if(val == NULL) {
    printf("Stack::Pop: invalid parameters !\n");
    return false;
  }  

  if(this->top >= 0) {
    *val = this->values[this->top];
    this->top--;
    return true;
  }

  return false;
}

template <class T>
bool Stack<T>::IsEmpty() {
	if(this->top >= 0) return false;
	return true;
}

#endif // #define NCD_SKEL_STACK_DEFINED
