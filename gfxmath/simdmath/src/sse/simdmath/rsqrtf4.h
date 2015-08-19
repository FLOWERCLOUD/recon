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

#ifndef ___SIMD_MATH_RSQRTF4_H___
#define ___SIMD_MATH_RSQRTF4_H___

#include <simdmath.h>

static SIMD_INLINE __m128
_rsqrtf4 (__m128 x)
{
  // Newton-Raphson method
  const __m128 half = _mm_set1_ps(0.5f);
  const __m128 three = _mm_set1_ps(3.0f);

  __m128 approx = _mm_rsqrt_ps( x );
  __m128 muls = _mm_mul_ps( _mm_mul_ps(x, approx), approx );
  return _mm_mul_ps( _mm_mul_ps(half, approx), _mm_sub_ps(three, muls) );

  /*
  // Reciprocal square root estimate and 1 Newton-Raphson iteration.

  const __m128 zero = _mm_set1_ps(0.0f);
  const __m128 half = _mm_set1_ps(0.5f);
  const __m128 one = _mm_set1_ps(1.0f);
  __m128 y0, y0x, y0half, tmp;

  y0 = _mm_rsqrt_ps( x );
  y0x = _mm_add_ps( _mm_mul_ps( y0, x ), zero );
  y0half = _mm_add_ps( _mm_mul_ps( y0, half ), zero );
  tmp = _mm_sub_ps( one, _mm_mul_ps( y0, y0x ) );
  return _mm_add_ps( _mm_mul_ps( tmp, y0half ), y0 );
  */
}

#endif
