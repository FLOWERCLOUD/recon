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

#ifndef ___SIMD_MATH__VEC_UTILS_H___
#define ___SIMD_MATH__VEC_UTILS_H___

#include <xmmintrin.h>
#include <stdint.h>

//#define _imask1f4(m) (_mm_castsi128_ps(_mm_set1_epi32(m)))
//#define _imaskf4(a,b,c,d) (_mm_castsi128_ps(_mm_setr_epi32((a),(b),(c),(d))))

#define _MM_COPYSIGNF_MASK 0x80000000
#define _MM_SIGNMASK_PS (_mm_castsi128_ps(_mm_set1_epi32(0x80000000)))

// bit-wise selection
static SIMD_INLINE __m128
_selectf4(__m128 x, __m128 y, __m128 s)
{
  return _mm_or_ps(_mm_andnot_ps(s, x), _mm_and_ps(s, y));
}

// component-wise selection
static SIMD_INLINE __m128
_blendf4(__m128 x, __m128 y, __m128 s)
{
#ifdef __SSE4_1__
  return _mm_blendv_ps(x, y, s);
#else
  return _selectf4(x, y, s);
#endif
}

static SIMD_INLINE __m128
_maddf4 (__m128 x, __m128 y, __m128 z)
{
#ifdef __FMA__
  return _mm_fmadd_ps(x, y, z);
#else
  return _mm_add_ps(_mm_mul_ps(x, y), z);
#endif
}

static SIMD_INLINE __m128
_nmsubf4 (__m128 x, __m128 y, __m128 z)
{
#ifdef __FMA__
  return _mm_fnmsub_ps(x, y, z);
#else
  return _mm_sub_ps(z, _mm_mul_ps(x, y));
#endif
}

static SIMD_INLINE __m128
_unaligned_loadf4(const float* p)
{
#if defined(__SSE3__)
  return _mm_castsi128_ps(_mm_lddqu_si128((const __m128i*)((void*)p)));
#else
  return _mm_loadu_ps(p);
#endif
}

#ifndef __AVX__
#  ifdef _mm_permute_ps // workaround for GNU C Compilers
#  undef _mm_permute_ps
#  endif
#  define _mm_permute_ps(v,i) _mm_shuffle_ps((v), (v), (i))
#endif

#define _MM_SPLAT(e) _MM_SHUFFLE((e), (e), (e), (e))

#define _MM_ROR(i) _MM_SHUFFLE((i+3)%4, (i+2)%4, (i+1)%4, (i+0)%4)
#define _MM_ROL(i) _MM_SHUFFLE((7-i)%4, (6-i)%4, (5-i)%4, (4-i)%4)

#define _mm_ror_ps(v,i) (((i)%4) ? (_mm_permute_ps((v),_MM_ROR(i))) : (v))
#define _mm_rol_ps(v,i) (((i)%4) ? (_mm_permute_ps((v),_MM_ROL(i))) : (v))
//#define vec_sld(vec,vec2,x) _mm_ror_ps(vec, ((x)/4))

#endif
