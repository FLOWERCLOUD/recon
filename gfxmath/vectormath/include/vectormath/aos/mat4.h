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
#include <simdmath/copyyf4.h>

namespace vectormath {
namespace aos {

// =====================================================================
//
// 3x3 float matrix
//

inline Mat3::Mat3(__m128 c0, __m128 c1, __m128 c2) noexcept
: xmm_column0(c0), xmm_column1(c1), xmm_column2(c2)
{
}

inline Mat3::Mat3(Vec3 c0, Vec3 c1, Vec3 c2) noexcept
: Mat3(c0.xmm_data, c1.xmm_data, c2.xmm_data)
{
}

inline Mat3 Mat3::zero() noexcept
{
  __m128 a0 = _mm_setzero_ps();
  return Mat3{ a0, a0, a0 };
}

inline Mat3 Mat3::identity() noexcept
{
  __m128 a0, a1, a2;
  a0 = _mm_setr_ps(1.0f, 0.0f, 0.0f, 0.0f);
  a1 = _mm_setr_ps(0.0f, 1.0f, 0.0f, 0.0f);
  a2 = _mm_setr_ps(0.0f, 0.0f, 1.0f, 0.0f);
  return Mat3{ a0, a1, a2 };
}

inline Mat3 Mat3::scaling(PFloat sx, PFloat sy, PFloat sz) noexcept
{
  __m128 a0, a1, a2;
  a0 = _copyxf4(_mm_setzero_ps(), sx.xmm_data);
  a1 = _copyyf4(_mm_setzero_ps(), sy.xmm_data);
  a2 = _copyzf4(_mm_setzero_ps(), sz.xmm_data);
  return Mat3{ a0, a1, a2 };
}

inline Mat3 Mat3::rotation(PFloat radian, Vec3 unitvec)
{
  __m128 s, c;
  sincosf4(radian.xmm_data, &s, &c);

  __m128 axis = unitvec.xmm_data;
  __m128 xxxx = _mm_permute_ps(axis, _MM_SPLAT(0));
  __m128 yyyy = _mm_permute_ps(axis, _MM_SPLAT(1));
  __m128 zzzz = _mm_permute_ps(axis, _MM_SPLAT(2));
  __m128 oneminusc = _mm_sub_ps(_mm_set1_ps(1.0f), c);
  __m128 axis_s = _mm_mul_ps(axis, s);
  __m128 naxis_s = _negatef4(axis_s);

  __m128 tmp0, tmp1, tmp2;
  tmp0 = _mm_permute_ps(axis_s, _MM_SHUFFLE(0,0,2,0));
  tmp0 = _copyzf4(tmp0, _mm_permute_ps(naxis_s, _MM_SPLAT(1)));
  tmp1 = _mm_permute_ps(axis_s, _MM_SPLAT(0));
  tmp1 = _copyxf4(tmp1, _mm_permute_ps(naxis_s, _MM_SPLAT(2)));
  tmp2 = _mm_permute_ps(axis_s, _MM_SHUFFLE(0,0,0,1));
  tmp2 = _copyyf4(tmp2, _mm_permute_ps(naxis_s, _MM_SPLAT(0)));

  tmp0 = _copyxf4(tmp0, c);
  tmp1 = _copyyf4(tmp1, c);
  tmp2 = _copyzf4(tmp2, c);

  return Mat3{
    _maddf4(_mm_mul_ps(axis, xxxx), oneminusc, tmp0),
    _maddf4(_mm_mul_ps(axis, yyyy), oneminusc, tmp1),
    _maddf4(_mm_mul_ps(axis, zzzz), oneminusc, tmp2)
  };
}

inline Mat3 Mat3::load(const float* pxyz) noexcept
{
  __m128 x0y0z0x1 = _unaligned_loadf4(pxyz+0);
  __m128 y1z1x2y2 = _unaligned_loadf4(pxyz+4);
  __m128 z2 = _mm_load_ss(pxyz+8);
  __m128 a0, a1, a2;
  a0 = x0y0z0x1;
  a1 = _mm_shuffle_ps(x0y0z0x1, y1z1x2y2, _MM_SHUFFLE(1,0,3,3));
  a1 = _mm_permute_ps(a1, _MM_SHUFFLE(0,3,2,0));
  a2 = _mm_shuffle_ps(y1z1x2y2, z2, _MM_SHUFFLE(1,0,3,2));
  return Mat3{ a0, a1, a2 };
}

inline void Mat3::store(float* pxyz) const noexcept
{
  __m128 x0y0z0 = xmm_column0;
  __m128 x1y1z1 = xmm_column1;
  __m128 x2y2z2 = xmm_column2;
  __m128 y0z0x1y1, x0y0z0x1, y1z1x2y2, z2;
  y0z0x1y1 = _mm_shuffle_ps(x0y0z0, x1y1z1, _MM_SHUFFLE(1,0,2,1));
  x0y0z0x1 = _mm_shuffle_ps(x0y0z0, y0z0x1y1, _MM_SHUFFLE(2,1,1,0));
  y1z1x2y2 = _mm_shuffle_ps(x1y1z1, x2y2z2, _MM_SHUFFLE(1,0,2,1));
  z2 = _mm_permute_ps(x2y2z2, _MM_SHUFFLE(0,0,0,2));
  _mm_storeu_ps(pxyz+0, x0y0z0x1);
  _mm_storeu_ps(pxyz+4, y1z1x2y2);
  _mm_store_ss(pxyz+8, z2);
}

inline Mat3 operator-(Mat3 m) noexcept
{
  return Mat3{
    _negatef4(m.xmm_column0),
    _negatef4(m.xmm_column1),
    _negatef4(m.xmm_column2)
  };
}

inline Mat3 operator+(Mat3 m0, Mat3 m1) noexcept
{
  __m128 a0, a1, a2, b0, b1, b2;
  a0 = m0.xmm_column0;
  a1 = m0.xmm_column1;
  a2 = m0.xmm_column2;
  b0 = m1.xmm_column0;
  b1 = m1.xmm_column1;
  b2 = m1.xmm_column2;
  a0 = _mm_add_ps(a0, b0);
  a1 = _mm_add_ps(a1, b1);
  a2 = _mm_add_ps(a2, b2);
  return Mat3{ a0, a1, a2 };
}

inline Mat3 operator-(Mat3 m0, Mat3 m1) noexcept
{
  __m128 a0, a1, a2, b0, b1, b2;
  a0 = m0.xmm_column0;
  a1 = m0.xmm_column1;
  a2 = m0.xmm_column2;
  b0 = m1.xmm_column0;
  b1 = m1.xmm_column1;
  b2 = m1.xmm_column2;
  a0 = _mm_sub_ps(a0, b0);
  a1 = _mm_sub_ps(a1, b1);
  a2 = _mm_sub_ps(a2, b2);
  return Mat3{ a0, a1, a2 };
}

inline Mat3 operator*(Mat3 m, PFloat s) noexcept
{
  __m128 a0, a1, a2, b;
  a0 = m.xmm_column0;
  a1 = m.xmm_column1;
  a2 = m.xmm_column2;
  b = s.xmm_data;
  a0 = _mm_mul_ps(a0, b);
  a1 = _mm_mul_ps(a1, b);
  a2 = _mm_mul_ps(a2, b);
  return Mat3{ a0, a1, a2 };
}

inline Mat3 operator*(PFloat s, Mat3 m) noexcept
{
  return operator*(m, s);
}

inline Mat3 operator/(Mat3 m, PFloat s) noexcept
{
  __m128 a0, a1, a2, b;
  a0 = m.xmm_column0;
  a1 = m.xmm_column1;
  a2 = m.xmm_column2;
  b = s.xmm_data;
  a0 = _mm_div_ps(a0, b);
  a1 = _mm_div_ps(a1, b);
  a2 = _mm_div_ps(a2, b);
  return Mat3{ a0, a1, a2 };
}

inline Vec3 operator*(Mat3 m, Vec3 v) noexcept
{
  __m128 a0, a1, a2, b, result;
  a0 = m.xmm_column0;
  a1 = m.xmm_column1;
  a2 = m.xmm_column2;
  b = v.xmm_data;
  result = _mm_mul_ps(a0, _mm_permute_ps(b, _MM_SPLAT(0)));
  result = _maddf4(a1, _mm_permute_ps(b, _MM_SPLAT(1)), result);
  result = _maddf4(a2, _mm_permute_ps(b, _MM_SPLAT(2)), result);
  return Vec3{ result };
}

inline Mat3 operator*(Mat3 m0, Mat3 m1) noexcept
{
  __m128 c0, c1, c2;
  c0 = (m0 * Vec3{ m1.xmm_column0 }).xmm_data;
  c1 = (m0 * Vec3{ m1.xmm_column1 }).xmm_data;
  c2 = (m0 * Vec3{ m1.xmm_column2 }).xmm_data;
  return Mat3{ c0, c1, c2 };
}

inline Mat3 transpose(Mat3 m) noexcept
{
  __m128 a0, a1, a2, tmp0, tmp1, res0, res1, res2;
  a0 = m.xmm_column0;
  a1 = m.xmm_column1;
  a2 = m.xmm_column2;

  tmp0 = _mm_unpacklo_ps(a0, a2);
  tmp1 = _mm_unpackhi_ps(a0, a2);
  res0 = _mm_unpacklo_ps(tmp0, a1);
  res1 = _mm_permute_ps(tmp0, _MM_SHUFFLE(0,3,2,2));
  res1 = _copyyf4(res1, a1);
  res2 = _mm_permute_ps(tmp1, _MM_SHUFFLE(0,1,1,0));
  res2 = _copyyf4(res2, _mm_permute_ps(a1, _MM_SPLAT(2)));

  return Mat3{ res0, res1, res2 };
}

inline PFloat determinant(Mat3 m) noexcept
{
  return dot(m.column2(), cross(m.column0(), m.column1()));
}

inline Mat3 inverse(Mat3 m) noexcept
{
  __m128 tmp0, tmp1, tmp2, tmp3, tmp4, dot, invdet, inv0, inv1, inv2;
  tmp2 = _crossf4(m.xmm_column0, m.xmm_column1);
  tmp0 = _crossf4(m.xmm_column1, m.xmm_column2);
  tmp1 = _crossf4(m.xmm_column2, m.xmm_column0);
  dot = _dot3f4(tmp2, m.xmm_column2);
  invdet = recipf4(dot);

  tmp3 = _mm_unpacklo_ps(tmp0, tmp2);
  tmp4 = _mm_unpackhi_ps(tmp0, tmp2);

  inv0 = _mm_unpacklo_ps(tmp3, tmp1);
  inv1 = _mm_permute_ps(tmp3, _MM_SHUFFLE(0,3,2,2));
  inv1 = _copyyf4(inv1, tmp1);
  inv2 = _mm_permute_ps(tmp4, _MM_SHUFFLE(0,1,1,0));
  inv2 = _copyyf4(inv2, _mm_permute_ps(tmp1, _MM_SPLAT(2)));

  inv0 = _mm_mul_ps(inv0, invdet);
  inv1 = _mm_mul_ps(inv1, invdet);
  inv2 = _mm_mul_ps(inv2, invdet);

  return Mat3{ inv0, inv1, inv2 };
}

inline PFloat trace(Mat3 m) noexcept
{
  __m128 sum = _mm_permute_ps(m.xmm_column0, _MM_SPLAT(0));
  sum = _mm_add_ps(sum, _mm_permute_ps(m.xmm_column1, _MM_SPLAT(1)));
  sum = _mm_add_ps(sum, _mm_permute_ps(m.xmm_column2, _MM_SPLAT(2)));
  return PFloat{ sum };
}

inline Vec3 Mat3::column0() const noexcept
{
  return Vec3{ xmm_column0 };
}

inline Vec3 Mat3::column1() const noexcept
{
  return Vec3{ xmm_column1 };
}

inline Vec3 Mat3::column2() const noexcept
{
  return Vec3{ xmm_column2 };
}

// =====================================================================
//
// 4x4 float matrix
//

inline Mat4::Mat4(__m128 c0, __m128 c1, __m128 c2, __m128 c3) noexcept
: xmm_column0(c0), xmm_column1(c1), xmm_column2(c2), xmm_column3(c3)
{
}

inline Mat4::Mat4(Vec4 c0, Vec4 c1, Vec4 c2, Vec4 c3) noexcept
: Mat4(c0.xmm_data, c1.xmm_data, c2.xmm_data, c3.xmm_data)
{
}

inline Mat4::Mat4(Mat3 m, Vec3 t) noexcept
: Mat4(_combine3f4(m.xmm_column0, _mm_set_ss(0.0f)),
       _combine3f4(m.xmm_column1, _mm_set_ss(0.0f)),
       _combine3f4(m.xmm_column2, _mm_set_ss(0.0f)),
       _combine3f4(t.xmm_data, _mm_set_ss(1.0f)))
{
}

inline Mat4 Mat4::zero() noexcept
{
  __m128 a0 = _mm_setzero_ps();
  return Mat4{ a0, a0, a0, a0 };
}

inline Mat4 Mat4::identity() noexcept
{
  __m128 a0, a1, a2, a3;
  a0 = _mm_setr_ps(1.0f, 0.0f, 0.0f, 0.0f);
  a1 = _mm_setr_ps(0.0f, 1.0f, 0.0f, 0.0f);
  a2 = _mm_setr_ps(0.0f, 0.0f, 1.0f, 0.0f);
  a3 = _mm_setr_ps(0.0f, 0.0f, 0.0f, 1.0f);
  return Mat4{ a0, a1, a2, a3 };
}

inline Mat4 Mat4::load(const float* pxyzw) noexcept
{
  __m128 a0, a1, a2, a3;
  a0 = _unaligned_loadf4(pxyzw + 0);
  a1 = _unaligned_loadf4(pxyzw + 4);
  a2 = _unaligned_loadf4(pxyzw + 8);
  a3 = _unaligned_loadf4(pxyzw +12);
  return Mat4{ a0, a1, a2, a3 };
}

inline void Mat4::store(float* pxyzw) const noexcept
{
  _mm_storeu_ps(pxyzw+ 0, xmm_column0);
  _mm_storeu_ps(pxyzw+ 4, xmm_column1);
  _mm_storeu_ps(pxyzw+ 8, xmm_column2);
  _mm_storeu_ps(pxyzw+12, xmm_column3);
}

inline Mat4 operator-(Mat4 m) noexcept
{
  return Mat4{
    _negatef4(m.xmm_column0),
    _negatef4(m.xmm_column1),
    _negatef4(m.xmm_column2),
    _negatef4(m.xmm_column3)
  };
}

inline Mat4 operator+(Mat4 m0, Mat4 m1) noexcept
{
  __m128 a0, a1, a2, a3, b0, b1, b2, b3;
  a0 = m0.xmm_column0;
  a1 = m0.xmm_column1;
  a2 = m0.xmm_column2;
  a3 = m0.xmm_column3;
  b0 = m1.xmm_column0;
  b1 = m1.xmm_column1;
  b2 = m1.xmm_column2;
  b3 = m1.xmm_column3;
  a0 = _mm_add_ps(a0, b0);
  a1 = _mm_add_ps(a1, b1);
  a2 = _mm_add_ps(a2, b2);
  a3 = _mm_add_ps(a3, b3);
  return Mat4{ a0, a1, a2, a3 };
}

inline Mat4 operator-(Mat4 m0, Mat4 m1) noexcept
{
  __m128 a0, a1, a2, a3, b0, b1, b2, b3;
  a0 = m0.xmm_column0;
  a1 = m0.xmm_column1;
  a2 = m0.xmm_column2;
  a3 = m0.xmm_column3;
  b0 = m1.xmm_column0;
  b1 = m1.xmm_column1;
  b2 = m1.xmm_column2;
  b3 = m1.xmm_column3;
  a0 = _mm_sub_ps(a0, b0);
  a1 = _mm_sub_ps(a1, b1);
  a2 = _mm_sub_ps(a2, b2);
  a3 = _mm_sub_ps(a3, b3);
  return Mat4{ a0, a1, a2, a3 };
}

inline Mat4 operator*(Mat4 m, PFloat s) noexcept
{
  __m128 a0, a1, a2, a3, b;
  a0 = m.xmm_column0;
  a1 = m.xmm_column1;
  a2 = m.xmm_column2;
  a3 = m.xmm_column3;
  b = s.xmm_data;
  a0 = _mm_mul_ps(a0, b);
  a1 = _mm_mul_ps(a1, b);
  a2 = _mm_mul_ps(a2, b);
  a3 = _mm_mul_ps(a3, b);
  return Mat4{ a0, a1, a2, a3 };
}

inline Mat4 operator*(PFloat s, Mat4 m) noexcept
{
  return operator*(m, s);
}

inline Mat4 operator/(Mat4 m, PFloat s) noexcept
{
  __m128 a0, a1, a2, a3, b;
  a0 = m.xmm_column0;
  a1 = m.xmm_column1;
  a2 = m.xmm_column2;
  a3 = m.xmm_column3;
  b = s.xmm_data;
  a0 = _mm_div_ps(a0, b);
  a1 = _mm_div_ps(a1, b);
  a2 = _mm_div_ps(a2, b);
  a3 = _mm_div_ps(a3, b);
  return Mat4{ a0, a1, a2, a3 };
}

inline Vec4 operator*(Mat4 m, Vec4 v) noexcept
{
  __m128 a0, a1, a2, a3, b, result;
  a0 = m.xmm_column0;
  a1 = m.xmm_column1;
  a2 = m.xmm_column2;
  a3 = m.xmm_column3;
  b = v.xmm_data;
  result = _mm_mul_ps(a0, _mm_permute_ps(b, _MM_SPLAT(0)));
  result = _maddf4(a1, _mm_permute_ps(b, _MM_SPLAT(1)), result);
  result = _maddf4(a2, _mm_permute_ps(b, _MM_SPLAT(2)), result);
  result = _maddf4(a3, _mm_permute_ps(b, _MM_SPLAT(3)), result);
  return Vec4{ result };
}

inline Mat4 operator*(Mat4 m0, Mat4 m1) noexcept
{
  __m128 c0, c1, c2, c3;
  c0 = (m0 * Vec4{ m1.xmm_column0 }).xmm_data;
  c1 = (m0 * Vec4{ m1.xmm_column1 }).xmm_data;
  c2 = (m0 * Vec4{ m1.xmm_column2 }).xmm_data;
  c3 = (m0 * Vec4{ m1.xmm_column3 }).xmm_data;
  return Mat4{ c0, c1, c2, c3 };
}

inline Mat4 transpose(Mat4 m) noexcept
{
  __m128 a0, a1, a2, a3, tmp3, tmp2, tmp1, tmp0;

  a0 = m.xmm_column0;
  a1 = m.xmm_column1;
  a2 = m.xmm_column2;
  a3 = m.xmm_column3;

  tmp0 = _mm_unpacklo_ps(a0, a1);
  tmp2 = _mm_unpacklo_ps(a2, a3);
  tmp1 = _mm_unpackhi_ps(a0, a1);
  tmp3 = _mm_unpackhi_ps(a2, a3);

  a0 = _mm_movelh_ps(tmp0, tmp2);
  a1 = _mm_movehl_ps(tmp2, tmp0);
  a2 = _mm_movelh_ps(tmp1, tmp3);
  a3 = _mm_movehl_ps(tmp3, tmp1);

  return Mat4{ a0, a1, a2, a3 };
}

inline Float determinant(Mat4 mat) noexcept
{
  __m128 Va,Vb,Vc;
	__m128 r1,r2,r3,tt,tt2;
	__m128 sum,Det;

	__m128 _L1 = mat.xmm_column0;
	__m128 _L2 = mat.xmm_column1;
	__m128 _L3 = mat.xmm_column2;
	__m128 _L4 = mat.xmm_column3;
	// Calculating the minterms for the first line.

	// _mm_ror_ps is just a macro using _mm_shuffle_ps().
	tt = _L4; tt2 = _mm_ror_ps(_L3,1);
	Vc = _mm_mul_ps(tt2,_mm_ror_ps(tt,0)); // V3' dot V4
	Va = _mm_mul_ps(tt2,_mm_ror_ps(tt,2)); // V3' dot V4"
	Vb = _mm_mul_ps(tt2,_mm_ror_ps(tt,3)); // V3' dot V4^

	r1 = _mm_sub_ps(_mm_ror_ps(Va,1),_mm_ror_ps(Vc,2)); // V3" dot V4^ - V3^ dot V4"
	r2 = _mm_sub_ps(_mm_ror_ps(Vb,2),_mm_ror_ps(Vb,0)); // V3^ dot V4' - V3' dot V4^
	r3 = _mm_sub_ps(_mm_ror_ps(Va,0),_mm_ror_ps(Vc,1)); // V3' dot V4" - V3" dot V4'

	tt = _L2;
	Va = _mm_ror_ps(tt,1);    sum = _mm_mul_ps(Va,r1);
	Vb = _mm_ror_ps(tt,2);    sum = _mm_add_ps(sum,_mm_mul_ps(Vb,r2));
	Vc = _mm_ror_ps(tt,3);    sum = _mm_add_ps(sum,_mm_mul_ps(Vc,r3));

	// Calculating the determinant.
	Det = _mm_mul_ps(sum,_L1);
	Det = _mm_add_ps(Det,_mm_movehl_ps(Det,Det));

	// Calculating the minterms of the second line (using previous results).
	tt = _mm_ror_ps(_L1,1);   sum = _mm_mul_ps(tt,r1);
	tt = _mm_ror_ps(tt,1);    sum = _mm_add_ps(sum,_mm_mul_ps(tt,r2));
	tt = _mm_ror_ps(tt,1);    sum = _mm_add_ps(sum,_mm_mul_ps(tt,r3));

	// Testing the determinant.
	Det = _mm_sub_ss(Det, _mm_permute_ps(Det, 0x1));
  return Float{ Det };
}

inline Mat4 inverse(Mat4 mat) noexcept
{
  __m128 Va,Vb,Vc;
  __m128 r1,r2,r3,tt,tt2;
  __m128 sum,Det,RDet;
  __m128 trns0,trns1,trns2,trns3;

  __m128 _L1 = mat.xmm_column0;
  __m128 _L2 = mat.xmm_column1;
  __m128 _L3 = mat.xmm_column2;
  __m128 _L4 = mat.xmm_column3;
  // Calculating the minterms for the first line.

  // _mm_ror_ps is just a macro using _mm_shuffle_ps().
  tt = _L4; tt2 = _mm_ror_ps(_L3,1);
  Vc = _mm_mul_ps(tt2,_mm_ror_ps(tt,0)); // V3'dot V4
  Va = _mm_mul_ps(tt2,_mm_ror_ps(tt,2)); // V3'dot V4"
  Vb = _mm_mul_ps(tt2,_mm_ror_ps(tt,3)); // V3' dot V4^

  r1 = _mm_sub_ps(_mm_ror_ps(Va,1),_mm_ror_ps(Vc,2)); // V3" dot V4^ - V3^ dot V4"
  r2 = _mm_sub_ps(_mm_ror_ps(Vb,2),_mm_ror_ps(Vb,0)); // V3^ dot V4' - V3' dot V4^
  r3 = _mm_sub_ps(_mm_ror_ps(Va,0),_mm_ror_ps(Vc,1)); // V3' dot V4" - V3" dot V4'

  tt = _L2;
  Va = _mm_ror_ps(tt,1);    sum = _mm_mul_ps(Va,r1);
  Vb = _mm_ror_ps(tt,2);    sum = _mm_add_ps(sum,_mm_mul_ps(Vb,r2));
  Vc = _mm_ror_ps(tt,3);    sum = _mm_add_ps(sum,_mm_mul_ps(Vc,r3));

  // Calculating the determinant.
  Det = _mm_mul_ps(sum,_L1);
  Det = _mm_add_ps(Det,_mm_movehl_ps(Det,Det));

  const __m128 Sign_PNPN = _mm_castsi128_ps(_mm_setr_epi32(0x00000000, 0x80000000, 0x00000000, 0x80000000));
  const __m128 Sign_NPNP = _mm_castsi128_ps(_mm_setr_epi32(0x80000000, 0x00000000, 0x80000000, 0x00000000));

  __m128 mtL1 = _mm_xor_ps(sum,Sign_PNPN);

  // Calculating the minterms of the second line (using previous results).
  tt = _mm_ror_ps(_L1,1);   sum = _mm_mul_ps(tt,r1);
  tt = _mm_ror_ps(tt,1);    sum = _mm_add_ps(sum,_mm_mul_ps(tt,r2));
  tt = _mm_ror_ps(tt,1);    sum = _mm_add_ps(sum,_mm_mul_ps(tt,r3));
  __m128 mtL2 = _mm_xor_ps(sum,Sign_NPNP);

  // Testing the determinant.
  Det = _mm_sub_ss(Det,_mm_permute_ps(Det,0x1));

  // Calculating the minterms of the third line.
  tt = _mm_ror_ps(_L1,1);
  Va = _mm_mul_ps(tt,Vb);  // V1' dot V2"
  Vb = _mm_mul_ps(tt,Vc);  // V1' dot V2^
  Vc = _mm_mul_ps(tt,_L2); // V1' dot V2

  r1 = _mm_sub_ps(_mm_ror_ps(Va,1),_mm_ror_ps(Vc,2)); // V1" dot V2^ - V1^ dot V2"
  r2 = _mm_sub_ps(_mm_ror_ps(Vb,2),_mm_ror_ps(Vb,0)); // V1^ dot V2' - V1' dot V2^
  r3 = _mm_sub_ps(_mm_ror_ps(Va,0),_mm_ror_ps(Vc,1)); // V1' dot V2" - V1" dot V2'

  tt = _mm_ror_ps(_L4,1); sum = _mm_mul_ps(tt,r1);
  tt = _mm_ror_ps(tt,1);  sum = _mm_add_ps(sum,_mm_mul_ps(tt,r2));
  tt = _mm_ror_ps(tt,1);  sum = _mm_add_ps(sum,_mm_mul_ps(tt,r3));
  __m128 mtL3 = _mm_xor_ps(sum,Sign_PNPN);

  // Dividing is FASTER than rcp_nr! (Because rcp_nr causes many register-memory RWs).
  RDet = _mm_div_ss(_mm_set_ss(1.0f), Det);
  RDet = _mm_permute_ps(RDet, 0x00);

  // Devide the first 12 minterms with the determinant.
  mtL1 = _mm_mul_ps(mtL1, RDet);
  mtL2 = _mm_mul_ps(mtL2, RDet);
  mtL3 = _mm_mul_ps(mtL3, RDet);

  // Calculate the minterms of the forth line and devide by the determinant.
  tt = _mm_ror_ps(_L3,1);   sum = _mm_mul_ps(tt,r1);
  tt = _mm_ror_ps(tt,1);    sum = _mm_add_ps(sum,_mm_mul_ps(tt,r2));
  tt = _mm_ror_ps(tt,1);    sum = _mm_add_ps(sum,_mm_mul_ps(tt,r3));
  __m128 mtL4 = _mm_xor_ps(sum,Sign_NPNP);
  mtL4 = _mm_mul_ps(mtL4, RDet);

  // Now we just have to transpose the minterms matrix.
  trns0 = _mm_unpacklo_ps(mtL1,mtL2);
  trns1 = _mm_unpacklo_ps(mtL3,mtL4);
  trns2 = _mm_unpackhi_ps(mtL1,mtL2);
  trns3 = _mm_unpackhi_ps(mtL3,mtL4);
  _L1 = _mm_movelh_ps(trns0,trns1);
  _L2 = _mm_movehl_ps(trns1,trns0);
  _L3 = _mm_movelh_ps(trns2,trns3);
  _L4 = _mm_movehl_ps(trns3,trns2);

  return Mat4{ _L1, _L2, _L3, _L4 };
}

inline PFloat trace(Mat4 m) noexcept
{
  __m128 sum = _mm_permute_ps(m.xmm_column0, _MM_SPLAT(0));
  sum = _mm_add_ps(sum, _mm_permute_ps(m.xmm_column1, _MM_SPLAT(1)));
  sum = _mm_add_ps(sum, _mm_permute_ps(m.xmm_column2, _MM_SPLAT(2)));
  sum = _mm_add_ps(sum, _mm_permute_ps(m.xmm_column3, _MM_SPLAT(3)));
  return PFloat{ sum };
}

inline Vec4 Mat4::column0() const noexcept
{
  return Vec4{ xmm_column0 };
}

inline Vec4 Mat4::column1() const noexcept
{
  return Vec4{ xmm_column1 };
}

inline Vec4 Mat4::column2() const noexcept
{
  return Vec4{ xmm_column2 };
}

inline Vec4 Mat4::column3() const noexcept
{
  return Vec4{ xmm_column3 };
}

inline Mat3 Mat4::upper3x3() const noexcept
{
  return Mat3{ xmm_column0, xmm_column1, xmm_column2 };
}

}
}
