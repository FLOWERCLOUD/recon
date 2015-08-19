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

#ifndef ___SIMD_MATH_RECIPF4_H___
#define ___SIMD_MATH_RECIPF4_H___

#include <simdmath.h>

static SIMD_INLINE __m128
_recipf4 (__m128 x)
{
  // Reciprocal estimate and 1 Newton-Raphson iteration.
  // A constant of 1.0 + 1 ulp in the Newton-Raphson step results in exact
  // answers for powers of 2, and a slightly smaller relative error bound.

  const __m128 oneish = _mm_castsi128_ps(_mm_set1_epi32(0x3f800001));
  __m128 y0, tmp;

  y0 = _mm_rcp_ps( x );
  tmp = _mm_sub_ps( oneish, _mm_mul_ps( x, y0 ) );
  return _mm_add_ps( _mm_mul_ps( tmp, y0 ), y0 );
}

#endif
