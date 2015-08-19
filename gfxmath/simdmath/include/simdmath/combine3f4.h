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

#ifndef ___SIMD_MATH_COMBINE3F4_H___
#define ___SIMD_MATH_COMBINE3F4_H___

#include <simdmath.h>
#include <simdmath/sse/_vec_utils.h>

static SIMD_INLINE __m128
_combine3f4(__m128 xyz, __m128 w)
{
  //__m128 zzww = _mm_shuffle_ps(xyz_, w___, _MM_SHUFFLE(0,0,2,2));
  //__m128 xyzw = _mm_shuffle_ps(xyz_, zzww, _MM_SHUFFLE(2,0,1,0));
#ifdef __SSE4_1__
  return _mm_insert_ps(xyz, w, 0x30);
#else
  __m128 w_z, xyzw;
  w_z = _mm_shuffle_ps(w, xyz, _MM_SHUFFLE(0,2,1,0));
  xyzw = _mm_shuffle_ps(xyz, w_z, _MM_SHUFFLE(0,2,1,0));
  return xyzw;
#endif
}

#endif
