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

#include <simdmath.h>
#if defined(__SSE__)
#  include <simdmath/sse/_vec_utils.h>
#endif
#include <simdmath/negatef4.h>
#include <simdmath/fabsf4.h>
#ifdef VECTORMATH_DEBUG
#  include <assert.h>
#  include <math.h>
#endif

namespace vectormath {
namespace aos {

/*

 XMM REGISTER:

 127    95     63     31     0
 | r[3] | r[2] | r[1] | r[0] | = { r[0], r[1], r[2], r[3] } (IN MEMORY)
 |  z   |  y   |  x   |  w   | = { w, x, y, z }

 Here Float-type operations only use r[0]

 EX:

 v1 = { x, y, z, w } = | w z y x |
 v2 = { a, b, c, d } = | d c b a |
 v3 = _mm_shuffle_ps(v1, v2, _MM_SHUFFLE(0,3,1,2))
 => v3 = | a d y z | = { z, y, d, a }

 */

// =====================================================================
//
// 1-component Float Scalar (stored in SIMD registers)
//

inline Float::Float(__m128 a) noexcept
: xmm_data(a)
{
}

inline Float::Float(float x) noexcept
: Float(_mm_set_ss(x))
{
}

inline Float::operator float() const noexcept
{
  return _mm_cvtss_f32(xmm_data);
}

inline Float Float::load(const float* ptr) noexcept
{
  return Float{ _mm_load_ss(ptr) };
}

inline void Float::store(float* ptr) const noexcept
{
  _mm_store_ss(ptr, xmm_data);
}

inline Float operator-(Float s) noexcept
{
  return Float{ _negatef4(s.xmm_data) };
}

inline Float operator+(Float a, Float b) noexcept
{
  return Float{ _mm_add_ss(a.xmm_data, b.xmm_data) };
}

inline Float operator-(Float a, Float b) noexcept
{
  return Float{ _mm_sub_ss(a.xmm_data, b.xmm_data) };
}

inline Float operator*(Float a, Float b) noexcept
{
  return Float{ _mm_mul_ss(a.xmm_data, b.xmm_data) };
}

inline Float operator/(Float a, Float b) noexcept
{
  return Float{ _mm_div_ss(a.xmm_data, b.xmm_data) };
}

inline Float abs(Float s) noexcept
{
  return Float{ _fabsf4(s.xmm_data) };
}

// =====================================================================
//
// 4-component Parallel Float Scalar (stored in SIMD registers)
//

inline PFloat::PFloat(__m128 a) noexcept
: xmm_data(a)
{
#ifdef VECTORMATH_DEBUG
  float f[4];
  _mm_storeu_ps(f, xmm_data);
  assert(f[0] == f[1]);
  assert(f[1] == f[2]);
  assert(f[2] == f[3]);
#endif
}

inline PFloat::PFloat(Float s) noexcept
: PFloat(_mm_permute_ps(s.xmm_data, _MM_SPLAT(0)))
{
}

inline PFloat::PFloat(float x) noexcept
: xmm_data(_mm_set1_ps(x))
{
}

inline PFloat::operator float() const noexcept
{
  return _mm_cvtss_f32(xmm_data);
}

inline PFloat::operator Float() const noexcept
{
  return Float{ xmm_data };
}

inline PFloat operator-(PFloat s) noexcept
{
  return PFloat{ _negatef4(s.xmm_data) };
}

inline PFloat operator+(PFloat a, PFloat b) noexcept
{
  return PFloat{ _mm_add_ps(a.xmm_data, b.xmm_data) };
}

inline PFloat operator-(PFloat a, PFloat b) noexcept
{
  return PFloat{ _mm_sub_ps(a.xmm_data, b.xmm_data) };
}

inline PFloat operator*(PFloat a, PFloat b) noexcept
{
  return PFloat{ _mm_mul_ps(a.xmm_data, b.xmm_data) };
}

inline PFloat operator/(PFloat a, PFloat b) noexcept
{
  return PFloat{ _mm_div_ps(a.xmm_data, b.xmm_data) };
}

inline PFloat abs(PFloat s) noexcept
{
  return PFloat{ _fabsf4(s.xmm_data) };
}

}
}
