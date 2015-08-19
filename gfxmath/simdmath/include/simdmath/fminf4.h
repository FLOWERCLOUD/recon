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

#ifndef ___SIMD_MATH_FMINF4_H___
#define ___SIMD_MATH_FMINF4_H___

#include <simdmath.h>

#if defined(__SSE__)

static SIMD_INLINE __m128
_fminf4(__m128 x, __m128 y)
{
  return _mm_min_ps(x, y);
}

#elif defined(__ALTIVEC__)

static SIMD_INLINE __vector float
_fminf4 (__vector float x, __vector float y)
{
  return vec_min( x, y );
}

#elif defined(__SPU__)

static SIMD_INLINE __vector float
_fminf4 (__vector float x, __vector float y)
{
  return spu_sel( x, y, spu_cmpgt( x, y ) );
}

#elif defined(__ARM_NEON)

static SIMD_INLINE float32x4_t
_fminf4(float32x4_t x, float32x4_t y)
{
  return vminq_f32(x, y);
}

#else
#  error Not Implemented
#endif

#endif
