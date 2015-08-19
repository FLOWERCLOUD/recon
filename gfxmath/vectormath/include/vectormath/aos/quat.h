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
#include <simdmath/copyxf4.h>
#include <simdmath/copyyf4.h>
#include <simdmath/copyzf4.h>
#include <simdmath/copywf4.h>

namespace vectormath {
namespace aos {

// =====================================================================
//
// float Quaternion
//

inline Quat::Quat(__m128 a) noexcept
: xmm_data(a)
{
}

inline Quat::Quat(float x, float y, float z, float w) noexcept
: Quat(_mm_setr_ps(x,y,z,w))
{
}

inline Quat::Quat(Vec3 v, float w) noexcept
: Quat(_combine3f4(v.xmm_data, _mm_set_ss(w)))
{
}

inline Quat Quat::zero() noexcept
{
  return Quat{ _mm_setzero_ps() };
}

inline Quat Quat::identity() noexcept
{
  return Quat{ _mm_setr_ps(0.0f, 0.0f, 0.0f, 1.0f) };
}

inline Quat Quat::rotation(PFloat radian, Vec3 unitvec) noexcept
{
  __m128 s, c, th, tmp;
  th = _mm_mul_ps(radian.xmm_data, _mm_set1_ps(0.5f)); // angle/2
  sincosf4(th, &s, &c); // compute cos(angle/2) and sin(angle/2)
  tmp = _mm_mul_ps(unitvec.xmm_data, s); // axis * sin(angle/2)
  return Quat{ _combine3f4(tmp, c) };
}

inline Quat Quat::rotation(Vec3 unitvec0, Vec3 unitvec1) noexcept
{
  __m128 v0 = unitvec0.xmm_data, v1 = unitvec1.xmm_data;
  __m128 xv = _crossf4(v0, v1); // axis * sin(angle)
  __m128 c = _dot3f4(v0, v1); // cos(angle)

  // compute cos(angle/2) = sqrt((1+cos)/2)
  //         sin(angle/2) = sqrt((1-cos)/2)
  //      => sin(angle/2) / sin(angle) = 1/sqrt(2+2cos)
  __m128 t0, t1;
  t0 = _mm_add_ps(_mm_add_ps(c, c), _mm_set1_ps(2.0f));
  t1 = rsqrtf4(t0); // sin(angle/2) / sin(angle)
  t0 = _mm_mul_ps(_mm_mul_ps(t0, t1), _mm_set1_ps(0.5f)); // cos(angle/2)

  return Quat{ _combine3f4(_mm_mul_ps(xv, t1), t0) };
}

inline Quat Quat::load(const float* pxyzw) noexcept
{
  return Quat{ _unaligned_loadf4(pxyzw) };
}

inline void Quat::store(float* pxyzw) const noexcept
{
  _mm_storeu_ps(pxyzw, xmm_data);
}

inline PFloat Quat::x() const noexcept
{
  __m128 a0 = xmm_data;
  a0 = _mm_permute_ps(a0, _MM_SPLAT(0));
  return PFloat{ a0 };
}

inline PFloat Quat::y() const noexcept
{
  __m128 a0 = xmm_data;
  a0 = _mm_permute_ps(a0, _MM_SPLAT(1));
  return PFloat{ a0 };
}

inline PFloat Quat::z() const noexcept
{
  __m128 a0 = xmm_data;
  a0 = _mm_permute_ps(a0, _MM_SPLAT(2));
  return PFloat{ a0 };
}

inline PFloat Quat::w() const noexcept
{
  __m128 a0 = xmm_data;
  a0 = _mm_permute_ps(a0, _MM_SPLAT(3));
  return PFloat{ a0 };
}

inline Vec3 Quat::v() const noexcept
{
  return Vec3{ xmm_data };
}

inline Quat operator-(Quat q) noexcept
{
  return Quat{ _negatef4(q.xmm_data) };
}

inline Quat operator~(Quat q) noexcept
{
  __m128 mask = _mm_setr_ps(-0.0f, -0.0f, -0.0f, 0.0f);
  __m128 a0 = q.xmm_data;
  a0 = _mm_xor_ps(mask, a0);
  return Quat{ a0 };
}

inline Quat operator+(Quat q0, Quat q1) noexcept
{
  __m128 a0 = q0.xmm_data, a1 = q1.xmm_data;
  return Quat{ _mm_add_ps(a0, a1) };
}

inline Quat operator-(Quat q0, Quat q1) noexcept
{
  __m128 a0 = q0.xmm_data, a1 = q1.xmm_data;
  return Quat{ _mm_sub_ps(a0, a1) };
}

inline Quat operator*(Quat q0, PFloat s) noexcept
{
  __m128 a0 = q0.xmm_data, a1 = s.xmm_data;
  return Quat{ _mm_mul_ps(a0, a1) };
}

inline Quat operator*(PFloat s, Quat q1) noexcept
{
  return operator*(q1, s);
}

inline Quat operator/(Quat q0, PFloat s) noexcept
{
  __m128 a0 = q0.xmm_data, a1 = s.xmm_data;
  return Quat{ _mm_div_ps(a0, a1) };
}

inline Quat operator*(Quat q0, Quat q1) noexcept
{
  // let w0 = q0.w, v0 = q0.xyz, w1 = q1.w, v1 = q1.xyz
  __m128 a0 = q0.xmm_data, a1 = q1.xmm_data;
  __m128 w0, w1, qw, qv;
  // result.w = w0 * w1 + dot(v0, v1)
  w0 = _mm_permute_ps(a0, _MM_SPLAT(3));
  w1 = _mm_permute_ps(a1, _MM_SPLAT(3));
  qw = _mm_sub_ps(_mm_mul_ps(w0, w1), _dot3f4(a0, a1));
  // result.v = w0 * v1 + w1 * v0 + cross(v0, v1)
  qv = _crossf4(a0, a1);
  qv = _mm_add_ps(_mm_mul_ps(w0, a1), qv);
  qv = _mm_add_ps(_mm_mul_ps(w1, a0), qv);
  return Quat{ _combine3f4(qv, qw) };
}

inline Vec3 rotate(Quat q, Vec3 v) noexcept
{
  // let p = (xyz=v, w=0)
  // p' = q * p * inv(q)
  //    = q * p * ~q when q is an unit Quaternion
  // =>
  //   tmpXYZ = q.w * v.xyz + cross(q.xyz, v.xyz)
  //   tmpW = dot(q.xyz, v.xyz)
  //   outXYZ = tmpW * q.xyz + tmpXYZ * q.w - cross(tmpXYZ, q.xyz)

  __m128 qdata = q.xmm_data, vdata = v.xmm_data;
  __m128 qwwww = _mm_permute_ps(qdata, _MM_SPLAT(3));

  __m128 tmp0, tmp1, tmpXYZ, tmpW;
  tmp0 = _mm_mul_ps(qwwww, vdata);
  tmp1 = _crossf4(qdata, vdata);
  tmpXYZ = _mm_add_ps(tmp0, tmp1);
  tmpW = _dot3f4(qdata, vdata);

  __m128 result;
  result = _crossf4(qdata, tmpXYZ);
  result = _maddf4(tmpW, qdata, result);
  result = _maddf4(tmpXYZ, qwwww, result);

  return Vec3{ result };
}

inline Quat normalize(Quat q) noexcept
{
  __m128 a0 = q.xmm_data;
  __m128 rlen = rsqrtf4(_dot4f4(a0, a0));
  a0 = _mm_mul_ps(a0, rlen);
  return Quat{ a0 };
}

inline Quat lerp(PFloat t, Quat q0, Quat q1) noexcept
{
  return q0 + t * (q1 - q0);
}

inline Quat slerp(Float t, Quat q0, Quat q1) noexcept
{
  // TODO: clean up code
  const float _VECTORMATH_SLERP_TOL = 0.999f;
  __m128 start;
  __m128 scales, scale0, scale1, cosAngle, angle, tttt, oneMinusT, angles, sines;
  __m128 smask;

  cosAngle = _dot4f4( q0.xmm_data, q1.xmm_data );
  smask = _mm_cmpgt_ps( _mm_setzero_ps(), cosAngle );
  cosAngle = _blendf4( cosAngle, _negatef4( cosAngle ), smask );
  start = _blendf4( q0.xmm_data, _negatef4( q0.xmm_data ), smask );
  smask = _mm_cmpgt_ps( _mm_set1_ps(_VECTORMATH_SLERP_TOL), cosAngle );
  angle = acosf4( cosAngle );
  tttt = _mm_permute_ps(t.xmm_data, _MM_SPLAT(0));
  oneMinusT = _mm_sub_ps( _mm_set1_ps(1.0f), tttt );
  angles = _mm_unpacklo_ps( _mm_set1_ps(1.0f), tttt );
  angles = _mm_unpacklo_ps( angles, oneMinusT );
  angles = _maddf4( angles, angle, _mm_setzero_ps() );
  sines = sinf4( angles );
  scales = _mm_div_ps( sines, _mm_permute_ps(sines, _MM_SPLAT(0)) );
  scale0 = _blendf4( oneMinusT, _mm_permute_ps(scales, _MM_SPLAT(1)), smask );
  scale1 = _blendf4( tttt, _mm_permute_ps(scales, _MM_SPLAT(2)), smask );
  return Quat{ _maddf4( start, scale0, _mm_mul_ps( q1.xmm_data, scale1 ) ) };
}

inline Quat squad(Float t, Quat q0, Quat q1, Quat q2, Quat q3) noexcept
{
  Float t2 = (t + t) * (Float(1.0f) - t);
  return slerp(t2, slerp(t, q0, q3), slerp(t, q1, q2));
}

inline Quat::operator Mat3() const noexcept
{
  __m128 xyzw_2, wwww, yzxw, zxyw, yzxw_2, zxyw_2;
  __m128 tmp0, tmp1, tmp2, tmp3, tmp4, tmp5;

  // objective:
  // (1 - 2yy - 2zz), (2xy - 2zw), (2xz + 2yw)
  // (2xy + 2zw), (1 - 2xx - 2yy), (2yz - 2xw)
  // (2xz - 2yw), (2yz + 2xw), (1 - 2xx - 2yy)

  xyzw_2 = _mm_add_ps(xmm_data, xmm_data);
  wwww = _mm_permute_ps(xmm_data, _MM_SHUFFLE(3,3,3,3));
  yzxw = _mm_permute_ps(xmm_data, _MM_SHUFFLE(3,0,2,1));
  zxyw = _mm_permute_ps(xmm_data, _MM_SHUFFLE(3,1,0,2));
  yzxw_2 = _mm_permute_ps(xyzw_2, _MM_SHUFFLE(3,0,2,1));
  zxyw_2 = _mm_permute_ps(xyzw_2, _MM_SHUFFLE(3,1,0,2));

  // tmp0 = 2yw, 2zw, 2xw, 2ww
  // tmp1 = 1 - 2yy, 1 - 2zz, 1 - 2xx, 1 - 2ww
  // tmp2 = 2xy, 2yz, 2xz, 2ww
  tmp0 = _mm_mul_ps(yzxw_2, wwww);
  tmp1 = _mm_sub_ps(_mm_set1_ps(1.0f), _mm_mul_ps(yzxw, yzxw_2));
  tmp2 = _mm_mul_ps(yzxw, xyzw_2);
  // tmp0 = 2yw + 2zx, 2zw + 2xy, 2xw + 2yz, 2ww + 2ww
  // tmp1 = 1 - 2yy - 2zz, 1 - 2zz - 2xx, 1 - 2xx - 2yy, 1 - 2ww - 2ww
  // tmp2 = 2xy - 2zw, 2yz - 2xw, 2xz - 2yw, 2ww - 2ww
  tmp0 = _mm_add_ps(_mm_mul_ps(zxyw, xyzw_2), tmp0);
  tmp1 = _mm_sub_ps(tmp1, _mm_mul_ps(zxyw, zxyw_2));
  tmp2 = _mm_sub_ps(tmp2, _mm_mul_ps(zxyw_2, wwww));

  tmp3 = _copyxf4(tmp0, tmp1);
  tmp4 = _copyxf4(tmp1, tmp2);
  tmp5 = _copyxf4(tmp2, tmp0);
  return Mat3 {
    _copyzf4(tmp3, tmp2),
    _copyzf4(tmp4, tmp0),
    _copyzf4(tmp5, tmp1)
  };
}

inline Quat Quat::rotation(Mat3 rot) noexcept
{
  // case 1: if trace(m) > 0
  //   t = 1.0 + m[0,0] + m[1,1] + m[2,2] = 1.0 + trace(m)
  //   s = 0.5 / sqrt(t)
  //   w = s * t
  //   z = s * (m[0,1] - m[1,0])
  //   y = s * (m[2,0] - m[0,2])
  //   x = s * (m[1,2] - m[2,1])
  // case 2: elif rot[0,0] > rot[1,1] and rot[2,2]
  //   t = 1.0 + m[0,0] - m[1,1] - m[2,2]
  //   s = 0.5 / sqrt(t)
  //   x = s * t
  //   y = s * (m[0,1] + m[1,0])
  //   z = s * (m[2,0] + m[0,2])
  //   w = s * (m[1,2] - m[2,1])
  // case 3: elif rot[1,1] > rot[2,2]
  //   t = 1.0 - m[0,0] + m[1,1] - m[2,2]
  //   s = 0.5 / sqrt(t)
  //   y = s * t
  //   x = s * (m[0,1] + m[1,0])
  //   w = s * (m[2,0] - m[0,2])
  //   z = s * (m[1,2] + m[2,1])
  // case 4: else
  //   t = 1.0 - m[0,0] - m[1,1] + m[2,2]
  //   s = 0.5 / sqrt(t)
  //   z = s * t
  //   w = s * (m[0,1] - m[1,0])
  //   x = s * (m[2,0] + m[0,2])
  //   y = s * (m[1,2] + m[2,1])

  __m128 c0 = rot.xmm_column0;
  __m128 c1 = rot.xmm_column1;
  __m128 c2 = rot.xmm_column2;
  __m128 d0120 = _mm_permute_ps(_copyzf4(_copyyf4(c0, c1), c2), _MM_SHUFFLE(0,2,1,0));
  __m128 d1201 = _mm_permute_ps(d0120, _MM_SHUFFLE(1,0,2,1));
  __m128 d2012 = _mm_permute_ps(d0120, _MM_SHUFFLE(2,1,0,2));
  __m128 dsum = _mm_add_ps(_mm_add_ps(d0120, d1201), d2012);
  __m128 ddiff = _mm_sub_ps(_mm_sub_ps(d0120, d1201), d2012);
  __m128 t = _mm_add_ps(_copywf4(ddiff, dsum), _mm_set1_ps(1.0f));
  __m128 s = _mm_mul_ps(rsqrtf4(t), _mm_set1_ps(0.5f)); // case 2, 3, 4, 1

  __m128 m20_01_12, m10_21_02;
  m20_01_12 = _copyzf4(_copyyf4(c2, c0), c1);
  m10_21_02 = _copyzf4(_copyyf4(c1, c2), c0);

  __m128 m01_20_12, m10_02_21;
  m01_20_12 = _mm_permute_ps(m20_01_12, _MM_SHUFFLE(3,2,0,1));
  m10_02_21 = _mm_permute_ps(m10_21_02, _MM_SHUFFLE(3,1,2,0));

  __m128 sum, diff;
  sum = _mm_add_ps(m01_20_12, m10_02_21);
  diff = _mm_sub_ps(m01_20_12, m10_02_21);
  sum = _copywf4(sum, t);
  diff = _copywf4(diff, t);

  __m128 res1, res2, res3, res4;

  // case 1: zyxw = (diff,diff,diff,t)
  res1 = _mm_permute_ps(diff, _MM_SHUFFLE(3,0,1,2));
  res1 = _mm_mul_ps(res1, _mm_permute_ps(s, _MM_SPLAT(3)));

  // case 2: yzwx = (sum,sum,diff,t)
  res2 = _copyzf4(sum, diff);
  res2 = _mm_permute_ps(res2, _MM_SHUFFLE(2,1,0,3));
  res2 = _mm_mul_ps(res2, _mm_permute_ps(s, _MM_SPLAT(0)));

  // case 3: xwzy = (sum,diff,sum,t)
  res3 = _copyyf4(sum, diff);
  res3 = _mm_permute_ps(res3, _MM_SHUFFLE(1,2,3,0));
  res3 = _mm_mul_ps(res3, _mm_permute_ps(s, _MM_SPLAT(1)));

  // case 4: wxyz = (diff,sum,sum,t)
  res4 = _copyxf4(sum, diff);
  res4 = _mm_permute_ps(res4, _MM_SHUFFLE(0,3,2,1));
  res4 = _mm_mul_ps(res4, _mm_permute_ps(s, _MM_SPLAT(2)));

  // determine case and compute final result
  __m128 d0 = _mm_permute_ps(c0, _MM_SPLAT(0));
  __m128 d1 = _mm_permute_ps(c1, _MM_SPLAT(1));
  __m128 d2 = _mm_permute_ps(c2, _MM_SPLAT(2));
  __m128 result;
  result = _blendf4(res2, res3, _mm_cmpgt_ps(d1, d0));
  result = _blendf4(result, res4, _mm_and_ps(_mm_cmpgt_ps(d2, d0), _mm_cmpgt_ps(d2, d1)));
  result = _blendf4(result, res1, _mm_cmpgt_ps(dsum, _mm_setzero_ps()));

  return Quat{ result };
}

// =====================================================================
//
// float dual Quaternion
//

inline DQuat::DQuat(__m128 qr, __m128 qe) noexcept
: xmm_real(qr), xmm_dual(qe)
{
}

inline DQuat::DQuat(Quat qr, Quat qe) noexcept
: DQuat(qr.xmm_data, qe.xmm_data)
{
}

inline DQuat DQuat::identity() noexcept
{
  return DQuat{ Quat::identity(), Quat::zero() };
}

inline DQuat DQuat::transform(Quat rot, Vec3 trans) noexcept
{
  return DQuat{ rot, Quat(trans * 0.5f, 0.0f) * rot };
}

inline DQuat::operator Mat4() const noexcept
{
  Quat real = Quat(xmm_real);
  Quat dual = Quat(xmm_dual);
  Mat3 rot = (Mat3)real;
  Quat halftrans = dual * (~real);
#ifdef VECTORMATH_DEBUG
  assert(fabsf((float)halftrans.w()) < 0.000001f);
#endif
  Vec3 trans = halftrans.v() + halftrans.v();

  return Mat4{ rot, trans };
}

inline Quat DQuat::real() const noexcept
{
  return Quat{ xmm_real };
}

inline Quat DQuat::dual() const noexcept
{
  return Quat{ xmm_dual };
}

inline DQuat operator-(DQuat dq0) noexcept
{
  __m128 a0 = dq0.xmm_real, a1 = dq0.xmm_dual;
  return DQuat{ _negatef4(a0), _negatef4(a1) };
}

inline DQuat operator+(DQuat dq0, DQuat dq1) noexcept
{
  __m128 a0 = dq0.xmm_real, a1 = dq0.xmm_dual,
         b0 = dq1.xmm_real, b1 = dq1.xmm_dual;
  return DQuat{ _mm_add_ps(a0, b0), _mm_add_ps(a1, b1) };
}

inline DQuat operator-(DQuat dq0, DQuat dq1) noexcept
{
  __m128 a0 = dq0.xmm_real, a1 = dq0.xmm_dual,
         b0 = dq1.xmm_real, b1 = dq1.xmm_dual;
  return DQuat{ _mm_sub_ps(a0, b0), _mm_sub_ps(a1, b1) };
}

inline DQuat operator*(DQuat dq0, PFloat s) noexcept
{
  __m128 a0 = dq0.xmm_real, a1 = dq0.xmm_dual,
         b = s.xmm_data;
  return DQuat{ _mm_mul_ps(a0, b), _mm_mul_ps(a1, b) };
}

inline DQuat operator*(PFloat s, DQuat dq0) noexcept
{
  return operator*(dq0, s);
}

inline DQuat operator/(DQuat dq0, PFloat s) noexcept
{
  __m128 a0 = dq0.xmm_real, a1 = dq0.xmm_dual,
         b = s.xmm_data;
  return DQuat{ _mm_div_ps(a0, b), _mm_div_ps(a1, b) };
}

inline DQuat operator*(DQuat dq0, DQuat dq1) noexcept
{
  // result.real = dq0.real * dq1.real
  // result.dual = dq0.real * dq1.dual + dq0.dual * dq1.real
  //
  __m128 r0 = dq0.xmm_real, r1 = dq1.xmm_real,
         d0 = dq0.xmm_dual, d1 = dq1.xmm_dual;
  __m128 r0w, r1w, d0w, d1w, outrw, outdw, outrv, outdv;

  r0w = _mm_permute_ps(r0, _MM_SPLAT(3));
  r1w = _mm_permute_ps(r1, _MM_SPLAT(3));
  d0w = _mm_permute_ps(d0, _MM_SPLAT(3));
  d1w = _mm_permute_ps(d1, _MM_SPLAT(3));

  outrw = _mm_sub_ps(_mm_mul_ps(r0w, r1w), _dot3f4(r0, r1));
  outdw = _mm_add_ps(_mm_sub_ps(_mm_mul_ps(r0w, d1w), _dot3f4(r0, d1)),
                     _mm_sub_ps(_mm_mul_ps(d0w, r1w), _dot3f4(d0, r1)));

  outrv = _crossf4(r0, r1);
  outrv = _maddf4(r0w, r1, outrv);
  outrv = _maddf4(r0, r1w, outrv);

  outdv = _mm_add_ps(_crossf4(r0, d1), _crossf4(d0, r1));
  outdv = _maddf4(r0w, d1, outdv);
  outdv = _maddf4(r0, d1w, outdv);
  outdv = _maddf4(d0w, r1, outdv);
  outdv = _maddf4(d0, r1w, outdv);

  return DQuat{ _combine3f4(outrv, outrw), _combine3f4(outdv, outdw) };
}

inline Vec3 transform(DQuat txfm, Vec3 vec) noexcept
{
  // let v = Quat{ vec, w = 0 }
  // txfm * (1+e(v)) * ~txfm
  // = (real + e(dual)) * (1+e(v)) * (~real - e(~dual))
  // = real * (-~dual) + real * v * ~real + dual * ~real
  // = real * v * ~real + 2 * dual * ~real (because dual.w = 0)
  //
  Quat real = Quat(txfm.xmm_real);
  Quat dual = Quat(txfm.xmm_dual);
  Quat halftrans = dual * ~real;
#ifdef VECTORMATH_DEBUG
  assert(fabsf((float)halftrans.w()) < 0.000001f);
#endif
  Vec3 trans = halftrans.v() + halftrans.v();
  return rotate(real, vec) + trans;
}

inline DQuat lerp(PFloat t, DQuat dq0, DQuat dq1) noexcept
{
  return dq0 + t * (dq1 - dq0);
}

}
}
