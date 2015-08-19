/*  SIMD math library functions for both the PowerPC (PPU) and the SPU.
   Copyright (C) 2006, 2007 Sony Computer Entertainment Inc.
   All rights reserved.

   modified by David Lin <davll.xc@gmail.com> in 2015

   Redistribution and use in source and binary forms,
   with or without modification, are permitted provided that the
   following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the Sony Computer Entertainment Inc nor the names
      of its contributors may be used to endorse or promote products derived
      from this software without specific prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
   AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
   ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
   LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
   CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
   SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
   INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
   CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
   ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
   POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#ifndef ___SIMD_MATH_H___
#define ___SIMD_MATH_H___

#define __SIMD_MATH_HAVE_VECTOR_f4 0
#define __SIMD_MATH_HAVE_VECTOR_i4 0
#define __SIMD_MATH_HAVE_VECTOR_d2 0
#define __SIMD_MATH_HAVE_VECTOR_ll2 0

#if defined(_MSC_VER)
#  include <intrin.h>
#  if defined (_M_IX86) || defined (_M_X64)
#    if (_M_IX86_FP < 2) && defined(_M_IX86)
#      error SSE and SSE2 should be enabled at least
#    endif
#    define __SSE__
#    define __SSE2__
#    define __SSE3__
#    define __SSSE3__
#    if defined(__AVX__)
#      define __SSE4_1__
//#      define __FMA__
#    endif
#  elif defined(_M_PPCBE)
#    define __ALTIVEC__
#  elif defined(_M_ARM)
#    define __ARM_NEON
#  endif
#endif

#ifndef SIMD_INLINE
#  if defined(__GNUC__) || defined(__clang__)
#    define SIMD_INLINE __attribute__((__always_inline__)) __inline__
#  elif defined(_MSC_VER)
#    define SIMD_INLINE __forceinline
#  else
#    define SIMD_INLINE __inline
#  endif
#endif

#ifndef SIMD_API
#  if defined(_MSC_VER)
#    define SIMD_API __vectorcall
#  elif defined(__GNUC__) || defined(__clang__)
#    define SIMD_API __attribute__((pure))
#  else
#    define SIMD_API
#  endif
#endif

#if defined(__SPU__)
// Cell SPU
#  include <spu_intrinsics.h>
#  undef __SIMD_MATH_HAVE_VECTOR_f4
#  define __SIMD_MATH_HAVE_VECTOR_f4 1
#  undef __SIMD_MATH_HAVE_VECTOR_i4
#  define __SIMD_MATH_HAVE_VECTOR_i4 1
#  undef __SIMD_MATH_HAVE_VECTOR_d2
#  define __SIMD_MATH_HAVE_VECTOR_d2 1
#  undef __SIMD_MATH_HAVE_VECTOR_ll2
#  define __SIMD_MATH_HAVE_VECTOR_ll2 1

typedef __vector float float4;
typedef __vector double double2;
typedef __vector signed int int4;
typedef __vector unsigned int uint4;
typedef __vector signed long long long2;
typedef __vector unsigned long long ulong2;

#elif defined(__ALTIVEC__)
// Power Architecture
#  include <altivec.h>
#  undef __SIMD_MATH_HAVE_VECTOR_f4
#  define __SIMD_MATH_HAVE_VECTOR_f4 1
#  undef __SIMD_MATH_HAVE_VECTOR_i4
#  define __SIMD_MATH_HAVE_VECTOR_i4 1

typedef __vector float float4;
typedef __vector signed int int4;
typedef __vector unsigned int uint4;

#elif defined(__SSE__)
// Intel x86/x86-64
#  include <xmmintrin.h>
#  include <emmintrin.h>
#  include <immintrin.h>
#  undef __SIMD_MATH_HAVE_VECTOR_f4
#  define __SIMD_MATH_HAVE_VECTOR_f4 1

typedef __m128 float4;

#elif defined(__ARM_NEON)
// ARM Arch
#  include <arm_neon.h>
#  undef __SIMD_MATH_HAVE_VECTOR_f4
#  define __SIMD_MATH_HAVE_VECTOR_f4 1
#  if defined(_MSC_VER)
typedef __n128 float4;
#  else
typedef float32x4_t float4;
#  endif

#else
#  error "No functions defined"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* Types */

#if __SIMD_MATH_HAVE_VECTOR_i4
typedef struct divi4_s {
  int4 quot;
  int4 rem;
} divi4_t;
#endif

#if __SIMD_MATH_HAVE_VECTOR_i4
typedef struct divu4_s {
  uint4 quot;
  uint4 rem;
} divu4_t;
#endif

#if __SIMD_MATH_HAVE_VECTOR_ll2
typedef struct lldivi2_s {
  long2 quot;
  long2 rem;
} lldivi2_t;
#endif

#if __SIMD_MATH_HAVE_VECTOR_ll2
typedef struct lldivu2_s {
  ulong2 quot;
  ulong2 rem;
} lldivu2_t;
#endif

#if __SIMD_MATH_HAVE_VECTOR_f4 && __SIMD_MATH_HAVE_VECTOR_ll2
typedef struct llroundf4_s {
	long2 vll[2];
} llroundf4_t;
#endif

/* integer divide */

#if __SIMD_MATH_HAVE_VECTOR_i4
divi4_t divi4 (int4, int4);
#endif

#if __SIMD_MATH_HAVE_VECTOR_i4
divu4_t divu4 (uint4, uint4);
#endif

#if __SIMD_MATH_HAVE_VECTOR_ll2
lldivi2_t lldivi2 (long2, long2);
#endif

#if __SIMD_MATH_HAVE_VECTOR_ll2
lldivu2_t lldivu2 (ulong2, ulong2);
#endif

/* abs value */
#if __SIMD_MATH_HAVE_VECTOR_f4
float4 fabsf4 (float4);
#endif

#if __SIMD_MATH_HAVE_VECTOR_d2
double2 fabsd2 (double2);
#endif

#if __SIMD_MATH_HAVE_VECTOR_i4
int4 absi4 (int4);
#endif

#if __SIMD_MATH_HAVE_VECTOR_ll2
long2 llabsi2 (long2);
#endif

/* negate value */
#if __SIMD_MATH_HAVE_VECTOR_f4
float4 negatef4 (float4);
#endif

#if __SIMD_MATH_HAVE_VECTOR_d2
double2 negated2 (double2);
#endif

#if __SIMD_MATH_HAVE_VECTOR_i4
int4 negatei4 (int4);
#endif

#if __SIMD_MATH_HAVE_VECTOR_ll2
long2 negatell2 (long2);
#endif

/* trunc */
#if __SIMD_MATH_HAVE_VECTOR_f4
float4 truncf4 (float4);
#endif

#if __SIMD_MATH_HAVE_VECTOR_d2
double2 trund2 (double2);
#endif

/* floor */
#if __SIMD_MATH_HAVE_VECTOR_f4
float4 floorf4 (float4);
#endif

#if __SIMD_MATH_HAVE_VECTOR_d2
double2 floord2 (double2);
#endif

/* ceil */
#if __SIMD_MATH_HAVE_VECTOR_f4
float4 ceilf4 (float4);
#endif

#if __SIMD_MATH_HAVE_VECTOR_d2
double2 ceild2 (double2);
#endif

/* exp */
#if __SIMD_MATH_HAVE_VECTOR_f4
float4 expf4 (float4);
#endif

#if __SIMD_MATH_HAVE_VECTOR_d2
double2 expd2 (double2);
#endif

/* exp */
#if __SIMD_MATH_HAVE_VECTOR_f4
float4 exp2f4 (float4);
#endif

#if __SIMD_MATH_HAVE_VECTOR_d2
double2 exp2d2 (double2);
#endif

/* expm1 */
#if __SIMD_MATH_HAVE_VECTOR_f4
float4 expm1f4 (float4);
#endif

#if __SIMD_MATH_HAVE_VECTOR_d2
double2 expm1d2 (double2);
#endif

/* log */
#if __SIMD_MATH_HAVE_VECTOR_f4
float4 logf4 (float4);
#endif

#if __SIMD_MATH_HAVE_VECTOR_d2
double2 logd2 (double2);
#endif

/* log10 */
#if __SIMD_MATH_HAVE_VECTOR_f4
float4 log10f4 (float4);
#endif

#if __SIMD_MATH_HAVE_VECTOR_d2
double2 log10d2 (double2);
#endif

/* log1p */
#if __SIMD_MATH_HAVE_VECTOR_f4
float4 log1pf4 (float4);
#endif

#if __SIMD_MATH_HAVE_VECTOR_d2
double2 log1pd2 (double2);
#endif

/* pow */
#if __SIMD_MATH_HAVE_VECTOR_f4
float4 powf4 (float4, float4);
#endif

/* fma */
#if __SIMD_MATH_HAVE_VECTOR_f4
float4 fmaf4 (float4, float4, float4);
#endif

#if __SIMD_MATH_HAVE_VECTOR_d2
double2 fmad2 (double2, double2, double2);
#endif

/* fmax */
#if __SIMD_MATH_HAVE_VECTOR_f4
float4 fmaxf4 (float4, float4);
#endif

#if __SIMD_MATH_HAVE_VECTOR_d2
double2 fmaxd2 (double2, double2);
#endif

/* fmin */
#if __SIMD_MATH_HAVE_VECTOR_f4
float4 fminf4 (float4, float4);
#endif

#if __SIMD_MATH_HAVE_VECTOR_d2
double2 fmind2 (double2, double2);
#endif

/* fdim */
#if __SIMD_MATH_HAVE_VECTOR_f4
float4 fdimf4 (float4, float4);
#endif

#if __SIMD_MATH_HAVE_VECTOR_d2
double2 fdimd2 (double2, double2);
#endif


/* fmod */
#if __SIMD_MATH_HAVE_VECTOR_f4
float4 fmodf4 (float4, float4);
#endif

#if __SIMD_MATH_HAVE_VECTOR_d2
double2 fmodd2 (double2, double2);
#endif

/* log2 */
#if __SIMD_MATH_HAVE_VECTOR_f4
float4 log2f4 (float4);
#endif

#if __SIMD_MATH_HAVE_VECTOR_d2
double2 log2d2 (double2);
#endif

/* logb */
#if __SIMD_MATH_HAVE_VECTOR_f4
float4 logbf4 (float4);
#endif

#if __SIMD_MATH_HAVE_VECTOR_d2
double2 logbd2 (double2);
#endif

/* ilogb */
#if __SIMD_MATH_HAVE_VECTOR_f4 && __SIMD_MATH_HAVE_VECTOR_i4
int4 ilogbf4 (float4);
#endif

#if __SIMD_MATH_HAVE_VECTOR_d2 && __SIMD_MATH_HAVE_VECTOR_ll2
long2 ilogbd2 (double2);
#endif

/* modf */
#if __SIMD_MATH_HAVE_VECTOR_f4
float4 modff4 (float4, float4 *);
#endif

#if __SIMD_MATH_HAVE_VECTOR_d2
double2 modfd2 (double2, double2 *);
#endif

/* sqrt */
#if __SIMD_MATH_HAVE_VECTOR_f4
float4 sqrtf4 (float4);
#endif

#if __SIMD_MATH_HAVE_VECTOR_d2
double2 sqrtd2 (double2);
#endif

/* hypot */
#if __SIMD_MATH_HAVE_VECTOR_f4
float4 hypotf4 (float4, float4);
#endif

#if __SIMD_MATH_HAVE_VECTOR_d2
double2 hypotd2 (double2, double2);
#endif

/* cbrtf4 */
#if __SIMD_MATH_HAVE_VECTOR_f4
float4 cbrtf4 (float4);
#endif

#if __SIMD_MATH_HAVE_VECTOR_d2
double2 cbrtd2 (double2);
#endif

/* sin */
#if __SIMD_MATH_HAVE_VECTOR_f4
float4 sinf4 (float4);
#endif

#if __SIMD_MATH_HAVE_VECTOR_d2
double2 sind2 (double2);
#endif


/* asin */
#if __SIMD_MATH_HAVE_VECTOR_f4
float4 asinf4 (float4);
#endif

#if __SIMD_MATH_HAVE_VECTOR_d2
double2 asind2 (double2);
#endif



/* divide */
#if __SIMD_MATH_HAVE_VECTOR_f4
float4 divf4 (float4, float4);
#endif

#if __SIMD_MATH_HAVE_VECTOR_d2
double2 divd2 (double2, double2);
#endif

/* remainder */
#if __SIMD_MATH_HAVE_VECTOR_f4
float4 remainderf4 (float4, float4);
#endif

#if __SIMD_MATH_HAVE_VECTOR_d2
double2 remainderd2 (double2, double2);
#endif

#if __SIMD_MATH_HAVE_VECTOR_f4 && __SIMD_MATH_HAVE_VECTOR_i4
float4 remquof4(float4 x, float4 y, int4 *quo);
#endif

#if __SIMD_MATH_HAVE_VECTOR_d2 && __SIMD_MATH_HAVE_VECTOR_ll2
double2 remquod2(double2 x, double2 y, long2 *quo);
#endif

/* copysign */
#if __SIMD_MATH_HAVE_VECTOR_f4
float4 copysignf4 (float4, float4);
#endif

#if __SIMD_MATH_HAVE_VECTOR_d2
double2 copysignd2 (double2, double2);
#endif

/* cos */
#if __SIMD_MATH_HAVE_VECTOR_f4
float4 cosf4 (float4);
#endif

#if __SIMD_MATH_HAVE_VECTOR_d2
double2 cosd2 (double2);
#endif

/* acos */
#if __SIMD_MATH_HAVE_VECTOR_f4
float4 acosf4 (float4);
#endif

#if __SIMD_MATH_HAVE_VECTOR_d2
double2 acosd2 (double2);
#endif

/* atan */
#if __SIMD_MATH_HAVE_VECTOR_f4
float4 atanf4 (float4);
#endif

#if __SIMD_MATH_HAVE_VECTOR_d2
double2 atand2 (double2);
#endif

/* atan2 */
#if __SIMD_MATH_HAVE_VECTOR_f4
float4 atan2f4 (float4, float4);
#endif

#if __SIMD_MATH_HAVE_VECTOR_d2
double2 atan2d2 (double2, double2);
#endif


/* tan */
#if __SIMD_MATH_HAVE_VECTOR_f4
float4 tanf4 (float4);
#endif

#if __SIMD_MATH_HAVE_VECTOR_d2
double2 tand2 (double2);
#endif

/* sincos */
#if __SIMD_MATH_HAVE_VECTOR_f4
void sincosf4 (float4, float4 *, float4 *);
#endif

#if __SIMD_MATH_HAVE_VECTOR_d2
void sincosd2 (double2, double2 *, double2 *);
#endif



/* recip */
#if __SIMD_MATH_HAVE_VECTOR_f4
float4 recipf4 (float4);
#endif

#if __SIMD_MATH_HAVE_VECTOR_d2
double2 recipd2 (double2);
#endif


/* rsqrt */
#if __SIMD_MATH_HAVE_VECTOR_f4
float4 rsqrtf4 (float4);
#endif

#if __SIMD_MATH_HAVE_VECTOR_d2
double2 rsqrtd2 (double2);
#endif


/* frexp */
#if __SIMD_MATH_HAVE_VECTOR_f4 && __SIMD_MATH_HAVE_VECTOR_i4
float4 frexpf4 (float4, int4 *);
#endif

#if __SIMD_MATH_HAVE_VECTOR_d2 && __SIMD_MATH_HAVE_VECTOR_ll2
double2 frexpd2 (double2, long2 *);
#endif

/* ldexp */
#if __SIMD_MATH_HAVE_VECTOR_f4 && __SIMD_MATH_HAVE_VECTOR_i4
float4 ldexpf4 (float4, int4 );
#endif

#if __SIMD_MATH_HAVE_VECTOR_d2 && __SIMD_MATH_HAVE_VECTOR_ll2
double2 ldexpd2 (double2, long2 );
#endif

#if __SIMD_MATH_HAVE_VECTOR_f4 && __SIMD_MATH_HAVE_VECTOR_i4
float4 scalbnf4(float4 x, int4 n);
#endif

#if __SIMD_MATH_HAVE_VECTOR_d2 && __SIMD_MATH_HAVE_VECTOR_ll2
double2 scalbllnd2 (double2, long2 );
#endif


/* isnan */
#if __SIMD_MATH_HAVE_VECTOR_f4 && __SIMD_MATH_HAVE_VECTOR_i4
uint4 isnanf4 (float4);
#endif

#if __SIMD_MATH_HAVE_VECTOR_d2 && __SIMD_MATH_HAVE_VECTOR_ll2
ulong2 isnand2 (double2);
#endif

/* isinf */
#if __SIMD_MATH_HAVE_VECTOR_f4 && __SIMD_MATH_HAVE_VECTOR_i4
uint4 isinff4 (float4);
#endif

#if __SIMD_MATH_HAVE_VECTOR_d2 && __SIMD_MATH_HAVE_VECTOR_ll2
ulong2 isinfd2 (double2);
#endif

/* isfinite */
#if __SIMD_MATH_HAVE_VECTOR_f4 && __SIMD_MATH_HAVE_VECTOR_i4
uint4 isfinitef4 (float4);
#endif

#if __SIMD_MATH_HAVE_VECTOR_d2 && __SIMD_MATH_HAVE_VECTOR_ll2
ulong2 isfinited2 (double2);
#endif

/* isnormal */
#if __SIMD_MATH_HAVE_VECTOR_f4 && __SIMD_MATH_HAVE_VECTOR_i4
uint4 isnormalf4 (float4);
#endif

#if __SIMD_MATH_HAVE_VECTOR_d2 && __SIMD_MATH_HAVE_VECTOR_ll2
ulong2 isnormald2 (double2);
#endif

/* isunordered */
#if __SIMD_MATH_HAVE_VECTOR_f4 && __SIMD_MATH_HAVE_VECTOR_i4
uint4 isunorderedf4 (float4, float4);
#endif

#if __SIMD_MATH_HAVE_VECTOR_d2 && __SIMD_MATH_HAVE_VECTOR_ll2
ulong2 isunorderedd2 (double2, double2);
#endif

/* is0denorm */
#if __SIMD_MATH_HAVE_VECTOR_f4 && __SIMD_MATH_HAVE_VECTOR_i4
uint4 is0denormf4 (float4);
#endif

#if __SIMD_MATH_HAVE_VECTOR_d2 && __SIMD_MATH_HAVE_VECTOR_ll2
ulong2 is0denormd2 (double2);
#endif

/* signbit */
#if __SIMD_MATH_HAVE_VECTOR_f4 && __SIMD_MATH_HAVE_VECTOR_i4
uint4 signbitf4 (float4);
#endif

#if __SIMD_MATH_HAVE_VECTOR_d2 && __SIMD_MATH_HAVE_VECTOR_ll2
ulong2 signbitd2 (double2);
#endif

/* isequal */
#if __SIMD_MATH_HAVE_VECTOR_f4 && __SIMD_MATH_HAVE_VECTOR_i4
uint4 isequalf4 (float4, float4);
#endif

#if __SIMD_MATH_HAVE_VECTOR_d2 && __SIMD_MATH_HAVE_VECTOR_ll2
ulong2 isequald2 (double2, double2);
#endif

/* islessgreater */
#if __SIMD_MATH_HAVE_VECTOR_f4 && __SIMD_MATH_HAVE_VECTOR_i4
uint4 islessgreaterf4 (float4, float4);
#endif

#if __SIMD_MATH_HAVE_VECTOR_d2 && __SIMD_MATH_HAVE_VECTOR_ll2
ulong2 islessgreaterd2 (double2, double2);
#endif

/* isless */
#if __SIMD_MATH_HAVE_VECTOR_f4 && __SIMD_MATH_HAVE_VECTOR_i4
uint4 islessf4 (float4, float4);
#endif

#if __SIMD_MATH_HAVE_VECTOR_d2 && __SIMD_MATH_HAVE_VECTOR_ll2
ulong2 islessd2 (double2, double2);
#endif

/* isgreater */
#if __SIMD_MATH_HAVE_VECTOR_f4 && __SIMD_MATH_HAVE_VECTOR_i4
uint4 isgreaterf4 (float4, float4);
#endif

#if __SIMD_MATH_HAVE_VECTOR_d2 && __SIMD_MATH_HAVE_VECTOR_ll2
ulong2 isgreaterd2 (double2, double2);
#endif

/* islessequal */
#if __SIMD_MATH_HAVE_VECTOR_f4 && __SIMD_MATH_HAVE_VECTOR_i4
uint4 islessequalf4 (float4, float4);
#endif

#if __SIMD_MATH_HAVE_VECTOR_d2 && __SIMD_MATH_HAVE_VECTOR_ll2
ulong2 islessequald2 (double2, double2);
#endif

/* isgreaterequal */
#if __SIMD_MATH_HAVE_VECTOR_f4 && __SIMD_MATH_HAVE_VECTOR_i4
uint4 isgreaterequalf4 (float4, float4);
#endif

#if __SIMD_MATH_HAVE_VECTOR_d2 && __SIMD_MATH_HAVE_VECTOR_ll2
ulong2 isgreaterequald2 (double2, double2);
#endif

/* fpclassify */
#if __SIMD_MATH_HAVE_VECTOR_f4 && __SIMD_MATH_HAVE_VECTOR_i4
int4 fpclassifyf4 (float4);
#endif

#if __SIMD_MATH_HAVE_VECTOR_d2 && __SIMD_MATH_HAVE_VECTOR_ll2
long2 fpclassifyd2 (double2);
#endif

/* round */
#if __SIMD_MATH_HAVE_VECTOR_d2 && __SIMD_MATH_HAVE_VECTOR_ll2
long2 llroundd2 (double2);
#endif

#if __SIMD_MATH_HAVE_VECTOR_f4 && __SIMD_MATH_HAVE_VECTOR_ll2
llroundf4_t llroundf4 (float4);
#endif

#if __SIMD_MATH_HAVE_VECTOR_f4 && __SIMD_MATH_HAVE_VECTOR_ll2
llroundf4_t llrintf4 (float4);
#endif

#if __SIMD_MATH_HAVE_VECTOR_d2 && __SIMD_MATH_HAVE_VECTOR_ll2
long2 llrintd2 (double2);
#endif

#if __SIMD_MATH_HAVE_VECTOR_f4
float4 roundf4(float4);
#endif

#if __SIMD_MATH_HAVE_VECTOR_f4 && __SIMD_MATH_HAVE_VECTOR_i4
int4  iroundf4(float4);
#endif

#if __SIMD_MATH_HAVE_VECTOR_f4
float4 rintf4(float4);
#endif

#if __SIMD_MATH_HAVE_VECTOR_f4 && __SIMD_MATH_HAVE_VECTOR_i4
int4  irintf4(float4);
#endif

#if __SIMD_MATH_HAVE_VECTOR_d2
double2 nextafterd2 (double2, double2);
#endif

#if __SIMD_MATH_HAVE_VECTOR_f4
float4 nextafterf4(float4, float4);
#endif

#if __SIMD_MATH_HAVE_VECTOR_d2
double2 nearbyintd2 (double2);
#endif

#if __SIMD_MATH_HAVE_VECTOR_f4
float4 nearbyintf4(float4);
#endif

#if __SIMD_MATH_HAVE_VECTOR_d2
double2 truncd2 (double2);
#endif

#if __SIMD_MATH_HAVE_VECTOR_d2
double2 roundd2 (double2);
#endif

#if __SIMD_MATH_HAVE_VECTOR_d2
double2 rintd2 (double2);
#endif

#if __SIMD_MATH_HAVE_VECTOR_d2
double2 ceild2(double2);
#endif

#if __SIMD_MATH_HAVE_VECTOR_d2
double2 floord2(double2);
#endif

#if __SIMD_MATH_HAVE_VECTOR_d2
double2 fmodd2(double2, double2);
#endif

#if __SIMD_MATH_HAVE_VECTOR_d2
double2 remainderd2(double2, double2);
#endif

#ifdef __cplusplus
}
#endif

#endif
