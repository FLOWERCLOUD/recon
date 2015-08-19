/*
The source file is part of vectormath

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

#pragma once

#include "data_types.h"
#include <simdmath.h>

namespace vectormath {
namespace soa {

inline void load(Vec3& v, const float* pxyz) noexcept
{
#if defined(__AVX__)
  const __m128* m = (const __m128*)pxyz;
  __m256 m03, m14, m25, xy, yz;

  m03 = _mm256_castps128_ps256(m[0]); // load lower halves
  m14 = _mm256_castps128_ps256(m[1]);
  m25 = _mm256_castps128_ps256(m[2]);
  m03 = _mm256_insertf128_ps(m03,m[3],1); // load upper halves
  m14 = _mm256_insertf128_ps(m14,m[4],1);
  m25 = _mm256_insertf128_ps(m25,m[5],1);

  xy = _mm256_shuffle_ps(m14, m25, _MM_SHUFFLE(2,1,3,2)); // upper x's and y's
  yz = _mm256_shuffle_ps(m03, m14, _MM_SHUFFLE(1,0,2,1)); // lower y's and z's

  v.x = _mm256_shuffle_ps(m03, xy, _MM_SHUFFLE(2,0,3,0));
  v.y = _mm256_shuffle_ps(yz, xy, _MM_SHUFFLE(3,1,2,0));
  v.z = _mm256_shuffle_ps(yz, m25, _MM_SHUFFLE(3,0,3,1));

#elif defined(__SSE__)
  __m128 x0y0z0x1 = _mm_load_ps(pxyz+0);
  __m128 y1z1x2y2 = _mm_load_ps(pxyz+4);
  __m128 z2x3y3z3 = _mm_load_ps(pxyz+8);
  __m128 x2y2x3y3 = _mm_shuffle_ps(y1z1x2y2,z2x3y3z3,_MM_SHUFFLE(2,1,3,2));
  __m128 y0z0y1z1 = _mm_shuffle_ps(x0y0z0x1,y1z1x2y2,_MM_SHUFFLE(1,0,2,1));
  v.x = _mm_shuffle_ps(x0y0z0x1,x2y2x3y3,_MM_SHUFFLE(2,0,3,0));
  v.y = _mm_shuffle_ps(y0z0y1z1,x2y2x3y3,_MM_SHUFFLE(3,1,2,0));
  v.z = _mm_shuffle_ps(y0z0y1z1,z2x3y3z3,_MM_SHUFFLE(3,0,3,1));

#endif
}

inline void load(Vec4& v, const float* pxyzw) noexcept
{
#if defined(__AVX__)
  const __m128* m = (const __m128*)pxyzw;

  __m256 a0, a1, a2, a3;
  a0 = _mm256_castps128_ps256(m[0]);
  a1 = _mm256_castps128_ps256(m[1]);
  a2 = _mm256_castps128_ps256(m[2]);
  a3 = _mm256_castps128_ps256(m[3]);
  a0 = _mm256_insertf128_ps(a0,m[4],1);
  a1 = _mm256_insertf128_ps(a1,m[5],1);
  a2 = _mm256_insertf128_ps(a2,m[6],1);
  a3 = _mm256_insertf128_ps(a3,m[7],1);

  __m256 tmp0, tmp1, tmp2, tmp3;
  tmp0 = _mm256_unpacklo_ps(a0, a2);
  tmp1 = _mm256_unpacklo_ps(a1, a3);
  tmp2 = _mm256_unpackhi_ps(a0, a2);
  tmp3 = _mm256_unpackhi_ps(a1, a3);

  v.x = _mm256_unpacklo_ps(tmp0, tmp1);
  v.y = _mm256_unpackhi_ps(tmp0, tmp1);
  v.z = _mm256_unpacklo_ps(tmp2, tmp3);
  v.w = _mm256_unpackhi_ps(tmp2, tmp3);

#elif defined(__SSE__)
  __m128 a0 = _mm_load_ps(pxyzw+ 0);
  __m128 a1 = _mm_load_ps(pxyzw+ 4);
  __m128 a2 = _mm_load_ps(pxyzw+ 8);
  __m128 a3 = _mm_load_ps(pxyzw+12);
  __m128 tmp0, tmp1, tmp2, tmp3;

  tmp0 = _mm_unpacklo_ps(a0, a2);
  tmp1 = _mm_unpacklo_ps(a1, a3);
  tmp2 = _mm_unpackhi_ps(a0, a2);
  tmp3 = _mm_unpackhi_ps(a1, a3);

  v.x = _mm_unpacklo_ps(tmp0, tmp1);
  v.y = _mm_unpackhi_ps(tmp0, tmp1);
  v.z = _mm_unpacklo_ps(tmp2, tmp3);
  v.w = _mm_unpackhi_ps(tmp2, tmp3);

#endif
}

inline void store(float* pxyz, const Vec3& v) noexcept
{
#if defined(__AVX__)
  __m128* m = (__m128*)pxyz;
  __m256 x = v.x;
  __m256 y = v.y;
  __m256 z = v.z;

  __m256 rxy = _mm256_shuffle_ps(x,y, _MM_SHUFFLE(2,0,2,0));
  __m256 ryz = _mm256_shuffle_ps(y,z, _MM_SHUFFLE(3,1,3,1));
  __m256 rzx = _mm256_shuffle_ps(z,x, _MM_SHUFFLE(3,1,2,0));
  __m256 r03 = _mm256_shuffle_ps(rxy, rzx, _MM_SHUFFLE(2,0,2,0));
  __m256 r14 = _mm256_shuffle_ps(ryz, rxy, _MM_SHUFFLE(3,1,2,0));
  __m256 r25 = _mm256_shuffle_ps(rzx, ryz, _MM_SHUFFLE(3,1,3,1));

  m[0] = _mm256_castps256_ps128(r03);
  m[1] = _mm256_castps256_ps128(r14);
  m[2] = _mm256_castps256_ps128(r25);
  m[3] = _mm256_extractf128_ps(r03, 1);
  m[4] = _mm256_extractf128_ps(r14, 1);
  m[5] = _mm256_extractf128_ps(r25, 1);

#elif defined(__SSE__)
  __m128 x = v.x;
  __m128 y = v.y;
  __m128 z = v.z;

  __m128 x0x2y0y2 = _mm_shuffle_ps(x,y, _MM_SHUFFLE(2,0,2,0));
  __m128 y1y3z1z3 = _mm_shuffle_ps(y,z, _MM_SHUFFLE(3,1,3,1));
  __m128 z0z2x1x3 = _mm_shuffle_ps(z,x, _MM_SHUFFLE(3,1,2,0));
  __m128 rx0y0z0x1= _mm_shuffle_ps(x0x2y0y2,z0z2x1x3, _MM_SHUFFLE(2,0,2,0));
  __m128 ry1z1x2y2= _mm_shuffle_ps(y1y3z1z3,x0x2y0y2, _MM_SHUFFLE(3,1,2,0));
  __m128 rz2x3y3z3= _mm_shuffle_ps(z0z2x1x3,y1y3z1z3, _MM_SHUFFLE(3,1,3,1));

  _mm_store_ps(pxyz+0, rx0y0z0x1);
  _mm_store_ps(pxyz+4, ry1z1x2y2);
  _mm_store_ps(pxyz+8, rz2x3y3z3);

#endif
}

inline void store(float* pxyzw, const Vec4& v) noexcept
{
#if defined(__AVX__)
  __m128* m = (__m128*)pxyzw;
  __m256 a0 = v.x;
  __m256 a1 = v.y;
  __m256 a2 = v.z;
  __m256 a3 = v.w;

  __m256 tmp0, tmp1, tmp2, tmp3;
  tmp0 = _mm256_unpacklo_ps(a0, a2);
  tmp1 = _mm256_unpacklo_ps(a1, a3);
  tmp2 = _mm256_unpackhi_ps(a0, a2);
  tmp3 = _mm256_unpackhi_ps(a1, a3);

  a0 = _mm256_unpacklo_ps(tmp0, tmp1);
  a1 = _mm256_unpackhi_ps(tmp0, tmp1);
  a2 = _mm256_unpacklo_ps(tmp2, tmp3);
  a3 = _mm256_unpackhi_ps(tmp2, tmp3);

  m[0] = _mm256_castps256_ps128(a0);
  m[1] = _mm256_castps256_ps128(a1);
  m[2] = _mm256_castps256_ps128(a2);
  m[3] = _mm256_castps256_ps128(a3);
  m[4] = _mm256_extractf128_ps(a0, 1);
  m[5] = _mm256_extractf128_ps(a1, 1);
  m[6] = _mm256_extractf128_ps(a2, 1);
  m[7] = _mm256_extractf128_ps(a3, 1);

#elif defined(__SSE__)
  __m128 a0 = v.x;
  __m128 a1 = v.y;
  __m128 a2 = v.z;
  __m128 a3 = v.w;
  __m128 tmp0, tmp1, tmp2, tmp3;

  tmp0 = _mm_unpacklo_ps(a0, a2);
  tmp1 = _mm_unpacklo_ps(a1, a3);
  tmp2 = _mm_unpackhi_ps(a0, a2);
  tmp3 = _mm_unpackhi_ps(a1, a3);

  _mm_store_ps(pxyzw+ 0, _mm_unpacklo_ps(tmp0, tmp1));
  _mm_store_ps(pxyzw+ 4, _mm_unpackhi_ps(tmp0, tmp1));
  _mm_store_ps(pxyzw+ 8, _mm_unpacklo_ps(tmp2, tmp3));
  _mm_store_ps(pxyzw+12, _mm_unpackhi_ps(tmp2, tmp3));

#endif
}

}
}
