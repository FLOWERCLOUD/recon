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

#ifndef ___SIMD_MATH_CEILF4_H___
#define ___SIMD_MATH_CEILF4_H___

#include <simdmath.h>
#include <simdmath/sse/_vec_utils.h>
#include <simdmath/fabsf4.h>

static SIMD_INLINE __m128
_ceilf4 (__m128 x)
{
#ifdef __SSE4_1__ /* SSE 4.1 */

  return _mm_round_ps( x, 0x2 );

#else /* SSE2 */

  const __m128 xa = _fabsf4(x);
  const __m128 inrange = _mm_cmpgt_ps(_mm_castsi128_ps(_mm_set1_epi32(0x4b000000)), xa);

  const __m128i xi = _mm_cvttps_epi32(x);
  const __m128i xi1 = _mm_add_epi32(xi, _mm_set1_epi32(1));

  const __m128 truncated0 = _selectf4(x, _mm_cvtepi32_ps(xi), inrange);
  const __m128 truncated1 = _selectf4(x, _mm_cvtepi32_ps(xi1), inrange);

  /* If truncated value is greater than input, subtract one. */

  return _selectf4(truncated0, truncated1, _mm_cmpgt_ps(x, truncated0));

#endif
}

#endif
/*
 .const
 .align 4
 one:	.long	0x3f800000
 absmask:.long	0x7fffffff

 .text
 .align 4
 .globl _ceilf
 _ceilf:
 movd		%xmm0,			%eax
 andl		absmask(%rip),	%eax
 movd		absmask(%rip),	%xmm1
 cmpl		$0x4b000000,	%eax
 andnps		%xmm0,			%xmm1
 jae			1f

 cvttps2dq	%xmm0,			%xmm2
 movdqa		%xmm0,			%xmm3
 psrad		$31,			%xmm0	// (x < 0) ? -1 : 0
 cvtdq2ps	%xmm2,			%xmm2
 pcmpgtd		%xmm2,			%xmm3	// (x >i trunc(x)) ? -1 : 0
 andnps		%xmm3,			%xmm0	// (x > trunc(x)) ? -1 : 0
 andps		one(%rip),		%xmm0	// (x > trunc(x)) ? 1.0 : 0.0
 addss		%xmm2,			%xmm0	// (x > trunc(x)) ? trunc(x) + 1.0 : trunc(x)
 orps		%xmm1,			%xmm0	// ceil(x)
 1:	ret

 // fast path using SSE 4.1
 roundss		$0x2,	%xmm0,	%xmm0
 movss		%xmm0,		  4(%esp)
 flds	  4(%esp)
 ret

 */
