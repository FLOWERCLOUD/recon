#ifndef NCD_DYNAMIC_ARRAY_H_INCLUDED
#define NCD_DYNAMIC_ARRAY_H_INCLUDED

///////////////////////////////////////////////////////////////////////////////
// This file contains two classes representing dynamic arrays that grow as
//   needed when new data is appended.
// DynamicPointerArray - is a class that manages a dynamic array of pointers to
//   external data. It does not allocate/free memory for the external data
// DynamicArray - is a class that manages a dynamic array of some data
//   it makes copies of the data that is appended. Internally, it uses a
//   DynamicPointerArray
// If you need to manage pointers to some data that is already created, use
//   DynamicPointerArray to avoid making copies of the data. Also use this
//   class if your data consists of pointers. In this case, using DynamicArray
//   would mean allocating memory to copy each of the pointers and then
//   managing an array of pointers to these pointers - pretty inneficient :).
///////////////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////////
// DynamicPointerArray - a dynamic array of pointers to some data that exists
//   outside this class.
// !! It is not the responsability of this class to allocate or free memory for
// the external data. The class only manages an array of pointers to that data.
// !! Be careful to free the memory before deleting pointers from this array
// in case you don't have other copies.
///////////////////////////////////////////////////////////////////////////////
template <class T>
class DynamicPointerArray {
 public:
  DynamicPointerArray(int initSize = 1);
  // destructor -
  //  - will not free the memory pointed to by the stored pointers
  ~DynamicPointerArray();

  // append -
  // does not make a copy of the object - only stores the pointer to it
  inline bool Append(T *obj);

  // GetElem -
  // returns the stored pointer in position pos
  inline T* GetElem(int pos);
  inline const T* GetElem(int pos) const;

  // operator []
  // alias for GetElem
  inline T* operator[](int pos);
  inline const T* operator[](int pos) const;

  // set the pointer at position pos to another value
  // does not free the memory pointed to by the previous value
  bool Set(int pos, T* pObj);

  // remove pointer at position pos.
  // does not free the memory pointed to by this pointer
  inline bool Remove(int pos);

  // removelastelem -
  // removes the last element in the array
  // does not free the memory pointed to by that pointer
  inline bool RemoveLastElem();

  // getLastElem()
  // returns the last pointer in the array
  inline T* GetLastElem();
  inline const T* GetLastElem() const;

  // get number of elements already in the array
  inline int GetNrElem() const { return this->nrElem;}

  // resize the array to exactly fit it's useful contents
  inline bool Fit();

  // grow the array to a user defined size
  bool GrowTo(int newSize);

  // deletes all elements but keeps the array at the current size
  inline bool DeleteAll();

  // deletes all elements are resizes the array to default size: 1
  inline bool Reset();

  // operator =
  DynamicPointerArray<T>& operator=(const DynamicPointerArray<T>& otherObj);

 private:
  inline int GetSize() const {return this->crtSize;}
  bool InitDynamicPointerArray(int initSize);
  inline bool Grow();


 private:
  int crtSize;
  int nrElem;
  T **pArray;
};


///////////////////////////////////////////////////////////////////////////////
// DynamicArray - Auto-expandable array
//  -- doubles it's size when a new elem is appended into a full array
// Makes a copy of any data that is appended in it
///////////////////////////////////////////////////////////////////////////////

template <class T>
class DynamicArray {
 public:
  DynamicArray(int initSize = 1);
  ~DynamicArray();

  inline bool Append(const T &obj);
  inline T& GetElem(int pos);
  inline const T& GetElem(int pos) const;
  inline T& operator[](int pos);
  inline const T& operator[](int pos) const;
  inline bool RemoveLastElem();
  inline bool Remove(int pos);
  inline int GetNrElem() const { return this->pArray.GetNrElem(); }
  inline bool Fit();

    // grow the array to a user defined size
  bool GrowTo(int newSize);

  // deletes all elements but keeps the array at the current size
  inline bool DeleteAll();
  // deletes all elements are resizes the array to default size: 1
  inline bool Reset();

  // operator =
  DynamicArray<T>& operator=(const DynamicArray<T>& otherObj);

 private:
  DynamicPointerArray<T> pArray;
};



///////////////////////////////////////////////////////////////////////////////
// Implementation - DynamicArray
///////////////////////////////////////////////////////////////////////////////


template <class T>
DynamicArray<T>::DynamicArray(int initSize /*= 1*/) {
  // set initial size of the pointer array
  this->pArray.GrowTo(initSize);
}


template <class T>
DynamicArray<T>::~DynamicArray() {

  // free the allocated memory
  for(int i = 0; i < this->pArray.GetNrElem(); i++) {
    if(this->pArray[i] != NULL) {
      delete this->pArray[i];
      this->pArray.Set(i, NULL);
    }
  }
}


template <class T>
inline bool DynamicArray<T>::Append(const T &obj) {
  //
  // first, allocate memory and make a copy of the object
  T *newObj;
  if((newObj = new T) == NULL) {
    printf("DynamicArray::Append - Not enough memory !!\n");
    return false;
  }
  // copy argument into newObj
  (*newObj) = obj;

  //
  // add pointer to pointer array
  return this->pArray.Append(newObj);
}


template <class T>
inline T& DynamicArray<T>::GetElem(int pos) {
  // if the index is out of bounds, this will crash, but there is
  // nothing I can do about it
  return *(this->pArray[pos]);
}

template <class T>
inline const T& DynamicArray<T>::GetElem(int pos) const {
  // if the index is out of bounds, this will crash, but there is
  // nothing I can do about it
  return *(this->pArray[pos]);
}


template <class T>
T& DynamicArray<T>::operator[](int pos) {
  return this->GetElem(pos);
}

template <class T>
const T& DynamicArray<T>::operator[](int pos) const {
  return this->GetElem(pos);
}


template <class T>
inline bool DynamicArray<T>::Fit() {
  return this->pArray.Fit();
}


template <class T>
inline bool DynamicArray<T>::Remove(int pos) {
  // retrieve the last pointer
  T *obj = this->pArray.GetElem(pos);

  // free the memory that it points to
  if(obj != NULL) {
    delete obj;
  }

  // remove pointer from the pointers array
  return this->pArray.Remove(pos);
}


template <class T>
inline bool DynamicArray<T>::RemoveLastElem() {
  // retrieve the last pointer
  T *obj = this->pArray.GetLastElem();

  // free the memory that it points to
  if(obj != NULL) {
    delete obj;
  }

  // remove pointer from the pointers array
  return this->pArray.RemoveLastElem();
}


template <class T>
inline bool DynamicArray<T>::DeleteAll() {
  // free the allocated memory
  for(int i = 0; i < this->pArray.GetNrElem(); i++) {
    if(this->pArray[i] != NULL) {
      delete this->pArray[i];
    }
  }
  // clean up the pointer array
  return this->pArray.DeleteAll();
}


template <class T>
inline bool DynamicArray<T>::Reset() {
  this->DeleteAll();
  // clean up the pointer array
  return this->pArray.Reset();
}


template <class T>
DynamicArray<T>& DynamicArray<T>::operator=(const DynamicArray<T> &otherObj) {
  // delete previous array
  this->Reset();

  // allocate new array
  this->pArray.GrowTo(otherObj.GetNrElem());

  // copy all elements from the other object
  for(int i=0; i < otherObj.GetNrElem(); i++) {
    this->Append(otherObj[i]);
  }

  return *this;
}


template <class T>
bool DynamicArray<T>::GrowTo(int newSize) {
  int i, nrToDelete;
  if(newSize < this->pArray.GetNrElem()) {
    printf("WARNING: DynamicArray::GrowTo(...): new size is less than current number of elements. Data will be lost !\n");
    nrToDelete = this->pArray.GetNrElem() - newSize;
    for(i=0; i < nrToDelete; i++) {
      if(!this->RemoveLastElem()) return false;
    }
  }
  return true;
}


///////////////////////////////////////////////////////////////////////////////
// Implementation of DynamicPointerArray
///////////////////////////////////////////////////////////////////////////////
template <class T>
DynamicPointerArray<T>::DynamicPointerArray(int initSize /*= 1*/) {
  this->InitDynamicPointerArray(initSize);
}


template <class T>
DynamicPointerArray<T>::~DynamicPointerArray() {

  if(this->pArray != NULL) {
    //
    // ! It is not my responsability to free the memory these pointers point to
    //
    delete [] this->pArray;
    this->pArray = NULL;
  }

  this->crtSize = 0;
  this->nrElem = 0;
}


template <class T>
bool DynamicPointerArray<T>::InitDynamicPointerArray(int initSize)
{
  this->nrElem = 0;
  this->crtSize = 0;
  this->pArray = NULL;

  if(initSize <= 0) initSize = 1;

  if((this->pArray = new T*[initSize]) == NULL) {
    printf("DynamicPointerArray::INIT - Not enough memory !!\n");
    return false;
  }
  memset(this->pArray, 0, sizeof(T*)*initSize);

  this->crtSize = initSize;
  // printf("Init ok\n");
  return true;
}


template <class T>
inline bool DynamicPointerArray<T>::Append(T *pObj) {
  // I will not make a copy of the object
  // only store the pointer to the object

  // printf("Append  ");
  if(this->nrElem >= this->crtSize) {
    if(!this->Grow()) {
      return false;
    }
  }

  this->pArray[this->nrElem] = pObj;
  this->nrElem++;

  return true;
}


template <class T>
inline T* DynamicPointerArray<T>::GetElem(int pos) {
  if((pos < 0) || (pos >= this->nrElem)) {
    printf("DynamicPointerArray:GetElem(...): index out of bounds when requesting element %d as number of elements is %d.\n",
	   pos, this->nrElem);
    return NULL;
  }

  return this->pArray[pos];
}

template <class T>
inline const T* DynamicPointerArray<T>::GetElem(int pos) const {
  if((pos < 0) || (pos >= this->nrElem)) {
    printf("DynamicPointerArray:GetElem(...): index out of bounds when requesting element %d as number of elements is %d.\n",
	   pos, this->nrElem);
    return NULL;
  }

  return this->pArray[pos];
}

template <class T>
inline T* DynamicPointerArray<T>::GetLastElem() {
  return this->GetElem(this->nrElem - 1);
}

template <class T>
inline const T* DynamicPointerArray<T>::GetLastElem() const {
  return this->GetElem(this->nrElem - 1);
}


template <class T>
T* DynamicPointerArray<T>::operator[](int pos) {
  return this->GetElem(pos);
}

template <class T>
const T* DynamicPointerArray<T>::operator[](int pos) const {
  return this->GetElem(pos);
}


template <class T>
bool DynamicPointerArray<T>::Set(int pos, T *pObj) {
  if((pos < 0) || (pos >= this->nrElem)) {
    printf("DynamicPointerArray:Set(...): index out of bounds !\n");
    return false;
  }

  // it is not my responsability to free the memoty pointed to by the previous
  // value
  this->pArray[pos] = pObj;

  return true;
}



template <class T>
bool DynamicPointerArray<T>::GrowTo(int newSize) {
  int i;
  int min_ns_ne;
  T** newArray = NULL;

  if(newSize <= 0) {
    printf("DynamicPointerArray::GrowTo(...) - new size is <= 0 !!\n");
    return false;
  }
  if(newSize < this->nrElem) {
    printf("WARNING: DynamicPointerArray::GrowTo(...): new size is < current number of elements. Data will be lost !\n");
  }

  // allocate new array
  if((newArray = new T*[newSize]) == NULL) {
    printf("DynamicPointerArray:GrowTo(...) - Not enough memory !\n");
    return false;
  }
  memset(newArray, 0, sizeof(T*)*newSize);

  // copy old values
  if(this->nrElem < newSize) {
    min_ns_ne = this->nrElem;
  }
  else {
    min_ns_ne = newSize;
  }

  for(i=0; i < min_ns_ne; i++) {
    newArray[i] = this->pArray[i];
  }

  // if the array shrunk,
  // I do not delete the memory pointed to by the pointers that will be lost.
  // Not my responsability

  // delete old array
  delete [] this->pArray;

  // link the new one
  this->pArray = newArray;

  // adjust current size
  this->crtSize = newSize;

  // adjust the current nr of elem
  this->nrElem = min_ns_ne;

  return true;
}


template <class T>
inline bool DynamicPointerArray<T>::Grow() {
  return this->GrowTo(this->crtSize * 2);
}



template <class T>
inline bool DynamicPointerArray<T>::Fit() {
  return this->GrowTo(this->nrElem);
}


template <class T>
inline bool DynamicPointerArray<T>::Remove(int pos) {
  if((pos < 0) || (pos >= this->nrElem)) {
    return false;
  }

  //
  // It is not my responsability to free the memory pointed to by that pointer
  //
  this->pArray[pos] = NULL;

  // move the following elements, one position to the left
  for(int i=pos; i < (this->nrElem - 1); i++) {
    this->pArray[i] = this->pArray[i+1];
  }
  // clear the last element
  if(this->nrElem > 0) {
    this->pArray[this->nrElem - 1] = NULL;
  }

  // adjust the number of elements
  this->nrElem--;

  return true;
}


template <class T>
inline bool DynamicPointerArray<T>::RemoveLastElem() {
  if(this->nrElem <= 0) {
    return false;
  }

  //
  // It is not my responsability to free the memory pointed to by that pointer
  //

  // adjust the number of elements
  this->nrElem--;

  return true;
}


template <class T>
inline bool DynamicPointerArray<T>::DeleteAll() {
  for(int i=0; i < this->nrElem; i++) {
    // It is not my responsability to free the memory pointed to the pointers
    this->pArray[i] = NULL;
  }
  this->nrElem = 0;
  return true;
}


template <class T>
inline bool DynamicPointerArray<T>::Reset() {
  this->DeleteAll();
  return this->GrowTo(1);
}


template <class T>
DynamicPointerArray<T>& DynamicPointerArray<T>::operator=(const DynamicPointerArray<T> &otherObj) {
  // delete previous array
  this->Reset();

  // allocate new array
  this->GrowTo(otherObj.GetNrElem());

  // copy all elements from the other object
  for(int i=0; i < otherObj.GetNrElem(); i++) {
    this->Append(otherObj[i]);
  }

  return *this;
}

#endif // NCD_DYNAMIC_ARRAY_H_INCLUDED
