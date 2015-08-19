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

#ifndef ___SIMD_MATH_TRUNCF4_H___
#define ___SIMD_MATH_TRUNCF4_H___

#include <simdmath.h>
#include <simdmath/sse/_vec_utils.h>
#include <simdmath/fabsf4.h>

static SIMD_INLINE __m128
_truncf4 (__m128 x)
{
#ifdef __SSE4_1__ /* SSE 4.1 */

  return _mm_round_ps( x, 0x3 );

#else /* SSE2 */

  const __m128 xa = _fabsf4(x);
  const __m128 inrange = _mm_cmpgt_ps(_mm_castsi128_ps(_mm_set1_epi32(0x4b000000)), xa);

  const __m128i xi = _mm_cvttps_epi32(x);

  return _selectf4(x, _mm_cvtepi32_ps(xi), inrange);

#endif
}

#endif
