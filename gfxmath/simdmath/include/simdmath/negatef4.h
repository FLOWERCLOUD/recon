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

#ifndef ___SIMD_MATH_NEGATEF4_H___
#define ___SIMD_MATH_NEGATEF4_H___

#include <simdmath.h>

#if defined(__SSE__)
#include <simdmath/sse/_vec_utils.h>

static SIMD_INLINE __m128
_negatef4 (__m128 x)
{
  return _mm_xor_ps(_MM_SIGNMASK_PS, x);
}

#elif defined(__ALTIVEC__)
#include <simdmath/ppu/_vec_utils.h>

static SIMD_INLINE __vector float
_negatef4 (__vector float x)
{
  return (__vector float)vec_xor((__vector unsigned int)x, __vec_splatsu4(0x80000000));
}

#elif defined(__SPU__)

static SIMD_INLINE __vector float
_negatef4 (__vector float x)
{
  return (vec_float4)spu_xor((vec_uint4)x, spu_splats(0x80000000));
}

#elif defined(__ARM_NEON)

static SIMD_INLINE float32x4_t
_negatef4(float32x4_t x)
{
  return vnegq_f32(x);
}

#else
#  error Not Implemented
#endif

#endif
