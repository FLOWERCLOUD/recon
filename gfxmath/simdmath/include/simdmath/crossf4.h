/*
The source file is part of simdmath

Copyright (c) 2015 David Lin <davll.xc@gmail.com>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#ifndef ___SIMD_MATH_CROSSF4_H___
#define ___SIMD_MATH_CROSSF4_H___

#include <simdmath.h>

static SIMD_INLINE __m128
_crossf4 (__m128 vec0, __m128 vec1)
{
  __m128 tmp0, tmp1, tmp2, tmp3;
  tmp0 = _mm_shuffle_ps(vec0, vec0, _MM_SHUFFLE(3,0,2,1));
  tmp1 = _mm_shuffle_ps(vec1, vec1, _MM_SHUFFLE(3,1,0,2));
  tmp2 = _mm_shuffle_ps(vec0, vec0, _MM_SHUFFLE(3,1,0,2));
  tmp3 = _mm_shuffle_ps(vec1, vec1, _MM_SHUFFLE(3,0,2,1));
  return _mm_sub_ps(_mm_mul_ps(tmp0, tmp1), _mm_mul_ps(tmp2, tmp3));
}

#endif
