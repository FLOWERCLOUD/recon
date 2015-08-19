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

#ifndef ___SIMD_MATH_SINCOSF4_H___
#define ___SIMD_MATH_SINCOSF4_H___

#include <simdmath.h>
#include <simdmath/_sincos.h>
#include <simdmath/sse/_vec_utils.h>
#include <simdmath/negatef4.h>
#include <simdmath/copysignf4.h>
#include <simdmath/ceilf4.h>
#include <simdmath/fabsf4.h>

//
//     Computes both the sine and cosine of the all four slots of x
//     by using a polynomial approximation.
//
static SIMD_INLINE void
_sincosf4 (__m128 x, __m128 *s, __m128 *c)
{
  __m128 xl, xl2, xl3, ts, tc;
  __m128i q, offsetSin, offsetCos;

  // Range reduction using : xl = angle * TwoOverPi;
  //
  xl = _mm_mul_ps(x, _mm_set1_ps(0.63661977236f));

  // Find the quadrant the angle falls in
  // using:  q = (int) (ceil(abs(xl))*sign(xl))
  //
  xl = _mm_add_ps(xl, _copysignf4(_mm_set1_ps(0.5f), xl));
  q = _mm_cvttps_epi32(xl);

  // Compute the offset based on the quadrant that the angle falls in.
  // Add 1 to the offset for the cosine.
  //
  offsetSin = _mm_and_si128(q, _mm_set1_epi32(0x3));
  offsetCos = _mm_add_epi32(offsetSin, _mm_set1_epi32(1));

  // Remainder in range [-pi/4..pi/4]
  //
  __m128 qf = _mm_cvtepi32_ps(q);
  __m128 p1 = _nmsubf4( qf, _mm_set1_ps(__SINCOSF_KC1), x );
  xl = _nmsubf4( qf, _mm_set1_ps(__SINCOSF_KC2), p1 );

  // Compute x^2 and x^3
  //
  xl2 = _mm_mul_ps(xl, xl);
  xl3 = _mm_mul_ps(xl2, xl);

  // Compute both the sin and cos of the angles
  // using a polynomial expression:
  //   cx = 1.0f + xl2 * ((C0 * xl2 + C1) * xl2 + C2), and
  //   sx = xl + xl3 * ((S0 * xl2 + S1) * xl2 + S2)
  //
  __m128 cx, sx;

  cx = _maddf4(_mm_set1_ps(__SINCOSF_CC0),xl2,_mm_set1_ps(__SINCOSF_CC1));
  sx = _maddf4(_mm_set1_ps(__SINCOSF_SC0),xl2,_mm_set1_ps(__SINCOSF_SC1));

  cx = _maddf4(cx,xl2,_mm_set1_ps(__SINCOSF_CC2));
  sx = _maddf4(sx,xl2,_mm_set1_ps(__SINCOSF_SC2));

  cx = _maddf4(cx,xl2,_mm_set1_ps(1.0f));
  sx = _maddf4(sx,xl3,xl);

  // Use the cosine when the offset is odd and the sin
  // when the offset is even
  //
  __m128i sinMask, cosMask;

  sinMask = _mm_and_si128(offsetSin, _mm_set1_epi32(0x1));
  sinMask = _mm_cmpeq_epi32(sinMask, _mm_setzero_si128());

  cosMask = _mm_and_si128(offsetCos, _mm_set1_epi32(0x1));
  cosMask = _mm_cmpeq_epi32(cosMask, _mm_setzero_si128());

  ts = _blendf4(cx, sx, _mm_castsi128_ps(sinMask));
  tc = _blendf4(cx, sx, _mm_castsi128_ps(cosMask));

  // Flip the sign of the result when (offset mod 4) = 1 or 2
  //
  sinMask = _mm_and_si128(offsetSin, _mm_set1_epi32(0x2));
  sinMask = _mm_cmpeq_epi32(sinMask, _mm_setzero_si128());

  cosMask = _mm_and_si128(offsetCos, _mm_set1_epi32(0x2));
  cosMask = _mm_cmpeq_epi32(cosMask, _mm_setzero_si128());

  ts = _blendf4(_negatef4(ts), ts, _mm_castsi128_ps(sinMask));
  tc = _blendf4(_negatef4(tc), tc, _mm_castsi128_ps(cosMask));

  *s = ts;
  *c = tc;
}

#endif
