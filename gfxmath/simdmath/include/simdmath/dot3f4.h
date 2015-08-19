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

#ifndef ___SIMD_MATH_DOT3F4_H___
#define ___SIMD_MATH_DOT3F4_H___

#include <simdmath.h>

static SIMD_INLINE __m128
_dot3f4 (__m128 x, __m128 y)
{
#if defined(__SSE4_1__)
  return _mm_dp_ps(x, y, 0x7f);
#elif defined(__SSE3__)
  __m128 t = _mm_mul_ps(x, y);
  __m128 t1 = _mm_hadd_ps(t, t);
  __m128 t2 = _mm_add_ps(t1, t);
  return _mm_shuffle_ps(t2, t2, _MM_SHUFFLE(2,2,2,2));
#else
  __m128 t = _mm_mul_ps(x, y);
  __m128 tx = _mm_shuffle_ps(t, t, _MM_SHUFFLE(0,0,0,0));
  __m128 ty = _mm_shuffle_ps(t, t, _MM_SHUFFLE(1,1,1,1));
  __m128 tz = _mm_shuffle_ps(t, t, _MM_SHUFFLE(2,2,2,2));
  return _mm_add_ps(tx, _mm_add_ps(ty, tz));
#endif
}

#endif
