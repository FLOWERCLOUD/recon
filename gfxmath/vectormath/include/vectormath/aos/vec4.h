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
#include <simdmath/dot3f4.h>
#include <simdmath/dot4f4.h>
#include <simdmath/crossf4.h>
#include <simdmath/negatef4.h>
#include <simdmath/fabsf4.h>
#include <simdmath/fmaxf4.h>
#include <simdmath/fminf4.h>
#include <simdmath/combine3f4.h>
#include <simdmath/copyxf4.h>
#include <simdmath/copyyf4.h>
#include <simdmath/copyzf4.h>
#include <simdmath/copywf4.h>

namespace vectormath {
namespace aos {

// =====================================================================
//
// 3-Component Float Vector
//

inline Vec3::Vec3(__m128 a) noexcept
: xmm_data(a)
{
}

inline Vec3::Vec3(float x, float y, float z) noexcept
: Vec3(_mm_setr_ps(x, y, z, 0.0f))
{
}

inline Vec3 Vec3::zero() noexcept
{
  return Vec3{ _mm_setzero_ps() };
}

inline Vec3 Vec3::proj(Vec4 v)
{
  __m128 xyzw = v.xmm_data;
  __m128 wwww = _mm_shuffle_ps(xyzw, xyzw, _MM_SHUFFLE(3,3,3,3));
  return Vec3{ _mm_div_ps(xyzw, wwww) };
}

inline Vec3 Vec3::cast(Vec4 v)
{
  return Vec3{ v.xmm_data };
}

inline Vec3 Vec3::load(const float* pxyz) noexcept
{
  __m128 x = _mm_load_ss(pxyz+0);
  __m128 y = _mm_load_ss(pxyz+1);
  __m128 z = _mm_load_ss(pxyz+2);
  __m128 xy = _mm_movelh_ps(x, y);
  __m128 xyz = _mm_shuffle_ps(xy, z, _MM_SHUFFLE(2,0,2,0));
  return Vec3{ xyz };
}

inline void Vec3::store(float* pxyz) const noexcept
{
  __m128 xyz = xmm_data;
  __m128 x = _mm_permute_ps(xyz, _MM_SPLAT(0));
  __m128 y = _mm_permute_ps(xyz, _MM_SPLAT(1));
  __m128 z = _mm_permute_ps(xyz, _MM_SPLAT(2));
  _mm_store_ss(pxyz+0, x);
  _mm_store_ss(pxyz+1, y);
  _mm_store_ss(pxyz+2, z);
}

inline PFloat Vec3::x() const noexcept
{
  __m128 a0 = xmm_data;
  a0 = _mm_permute_ps(a0, _MM_SPLAT(0));
  return PFloat{ a0 };
}

inline PFloat Vec3::y() const noexcept
{
  __m128 a0 = xmm_data;
  a0 = _mm_permute_ps(a0, _MM_SPLAT(1));
  return PFloat{ a0 };
}

inline PFloat Vec3::z() const noexcept
{
  __m128 a0 = xmm_data;
  a0 = _mm_permute_ps(a0, _MM_SPLAT(2));
  return PFloat{ a0 };
}

inline Vec3 operator-(Vec3 v) noexcept
{
  return Vec3{ _negatef4(v.xmm_data) };
}

inline Vec3 operator+(Vec3 v0, Vec3 v1) noexcept
{
  __m128 a0 = v0.xmm_data, a1 = v1.xmm_data;
  return Vec3{ _mm_add_ps(a0, a1) };
}

inline Vec3 operator-(Vec3 v0, Vec3 v1) noexcept
{
  __m128 a0 = v0.xmm_data, a1 = v1.xmm_data;
  return Vec3{ _mm_sub_ps(a0, a1) };
}

inline Vec3 operator*(Vec3 v0, PFloat s) noexcept
{
  __m128 a0 = v0.xmm_data, a1 = s.xmm_data;
  return Vec3{ _mm_mul_ps(a0, a1) };
}

inline Vec3 operator*(PFloat s, Vec3 v1) noexcept
{
  return operator*(v1, s);
}

inline Vec3 operator/(Vec3 v0, PFloat s) noexcept
{
  __m128 a0 = v0.xmm_data, a1 = s.xmm_data;
  return Vec3{ _mm_div_ps(a0, a1) };
}

inline Vec3 abs(Vec3 v) noexcept
{
  return Vec3{ _fabsf4(v.xmm_data) };
}

inline PFloat dot(Vec3 v0, Vec3 v1) noexcept
{
  __m128 a0 = v0.xmm_data, a1 = v1.xmm_data;
  a0 = _dot3f4(a0, a1);
  return PFloat{ a0 };
}

inline Vec3 cross(Vec3 v0, Vec3 v1) noexcept
{
  __m128 a0 = v0.xmm_data, a1 = v1.xmm_data;
  a0 = _crossf4(a0, a1);
  return Vec3{ a0 };
}

inline Vec3 normalize(Vec3 v) noexcept
{
  __m128 a0 = v.xmm_data;
  __m128 rlen = rsqrtf4(_dot3f4(a0, a0));
  a0 = _mm_mul_ps(a0, rlen);
  return Vec3{ a0 };
}

inline Float length(Vec3 v) noexcept
{
  __m128 a0 = v.xmm_data;
  a0 = _mm_sqrt_ss(_dot3f4(a0, a0));
  return Float{ a0 };
}

inline Vec3 lerp(PFloat t, Vec3 v0, Vec3 v1) noexcept
{
  return v0 + t * (v1 - v0);
}

inline Vec3 min(Vec3 v0, Vec3 v1) noexcept
{
  return Vec3{ _fminf4(v0.xmm_data, v1.xmm_data) };
}

inline Vec3 max(Vec3 v0, Vec3 v1) noexcept
{
  return Vec3{ _fmaxf4(v0.xmm_data, v1.xmm_data) };
}

inline Vec3 copy_x(Vec3 v0, Vec3 v1) noexcept
{
  return Vec3{ _copyxf4(v0.xmm_data, v1.xmm_data) };
}

inline Vec3 copy_y(Vec3 v0, Vec3 v1) noexcept
{
  return Vec3{ _copyyf4(v0.xmm_data, v1.xmm_data) };
}

inline Vec3 copy_z(Vec3 v0, Vec3 v1) noexcept
{
  return Vec3{ _copyzf4(v0.xmm_data, v1.xmm_data) };
}

inline Vec3 clamp(Vec3 v, PFloat smin, PFloat smax) noexcept
{
  __m128 a = v.xmm_data;
  a = _fmaxf4(a, smin.xmm_data);
  a = _fminf4(a, smax.xmm_data);
  return Vec3{ a };
}

inline Vec3 clamp(Vec3 v, Vec3 vmin, Vec3 vmax) noexcept
{
  __m128 a = v.xmm_data;
  a = _fmaxf4(a, vmin.xmm_data);
  a = _fminf4(a, vmax.xmm_data);
  return Vec3{ a };
}

inline Vec3 square(Vec3 v) noexcept
{
  return mul(v, v);
}

inline Vec3 sqrt(Vec3 v) noexcept
{
  return Vec3{ sqrtf4(v.xmm_data) };
}

inline Vec3 rsqrt(Vec3 v) noexcept
{
  return Vec3{ rsqrtf4(v.xmm_data) };
}

inline Vec3 mul(Vec3 v0, Vec3 v1) noexcept
{
  return Vec3{ _mm_mul_ps(v0.xmm_data, v1.xmm_data) };
}

// =====================================================================
//
// 4-Component Float Vector
//

inline Vec4::Vec4(__m128 a) noexcept
: xmm_data(a)
{
}

inline Vec4::Vec4(float x, float y, float z, float w) noexcept
: Vec4(_mm_setr_ps(x, y, z, w))
{
}

inline Vec4::Vec4(Vec3 v, Float w) noexcept
: Vec4(_combine3f4(v.xmm_data, w.xmm_data))
{
}

inline Vec4 Vec4::zero() noexcept
{
  return Vec4{ _mm_setzero_ps() };
}

inline Vec4 Vec4::load(const float* pxyzw) noexcept
{
  return Vec4{ _unaligned_loadf4(pxyzw) };
}

inline void Vec4::store(float* pxyzw) const noexcept
{
  _mm_storeu_ps(pxyzw, xmm_data);
}

inline PFloat Vec4::x() const noexcept
{
  __m128 a0 = xmm_data;
  a0 = _mm_permute_ps(a0, _MM_SPLAT(0));
  return PFloat{ a0 };
}

inline PFloat Vec4::y() const noexcept
{
  __m128 a0 = xmm_data;
  a0 = _mm_permute_ps(a0, _MM_SPLAT(1));
  return PFloat{ a0 };
}

inline PFloat Vec4::z() const noexcept
{
  __m128 a0 = xmm_data;
  a0 = _mm_permute_ps(a0, _MM_SPLAT(2));
  return PFloat{ a0 };
}

inline PFloat Vec4::w() const noexcept
{
  __m128 a0 = xmm_data;
  a0 = _mm_permute_ps(a0, _MM_SPLAT(3));
  return PFloat{ a0 };
}

inline Vec4 operator-(Vec4 v) noexcept
{
  return Vec4{ _negatef4(v.xmm_data) };
}

inline Vec4 operator+(Vec4 v0, Vec4 v1) noexcept
{
  __m128 a0 = v0.xmm_data, a1 = v1.xmm_data;
  return Vec4{ _mm_add_ps(a0, a1) };
}

inline Vec4 operator-(Vec4 v0, Vec4 v1) noexcept
{
  __m128 a0 = v0.xmm_data, a1 = v1.xmm_data;
  return Vec4{ _mm_sub_ps(a0, a1) };
}

inline Vec4 operator*(Vec4 v0, PFloat s) noexcept
{
  __m128 a0 = v0.xmm_data, a1 = s.xmm_data;
  return Vec4{ _mm_mul_ps(a0, a1) };
}

inline Vec4 operator*(PFloat s, Vec4 v1) noexcept
{
  return operator*(v1, s);
}

inline Vec4 operator/(Vec4 v0, PFloat s) noexcept
{
  __m128 a0 = v0.xmm_data, a1 = s.xmm_data;
  return Vec4{ _mm_div_ps(a0, a1) };
}

inline Vec4 abs(Vec4 v) noexcept
{
  return Vec4{ _fabsf4(v.xmm_data) };
}

inline PFloat dot(Vec4 v0, Vec4 v1) noexcept
{
  __m128 a0 = v0.xmm_data, a1 = v1.xmm_data;
  a0 = _dot4f4(a0, a1);
  return PFloat{ a0 };
}

inline Vec4 normalize(Vec4 v) noexcept
{
  __m128 a0 = v.xmm_data;
  __m128 rlen = rsqrtf4(_dot4f4(a0, a0));
  a0 = _mm_mul_ps(a0, rlen);
  return Vec4{ a0 };
}

inline Float length(Vec4 v) noexcept
{
  __m128 a0 = v.xmm_data;
  a0 = _mm_sqrt_ss(_dot4f4(a0, a0));
  return Float{ a0 };
}

inline Vec4 lerp(PFloat t, Vec4 v0, Vec4 v1) noexcept
{
  return v0 + t * (v1 - v0);
}

}
}
