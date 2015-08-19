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

namespace vectormath {
namespace soa {

#if defined(__AVX__)
  typedef __m256 floatvec;
  const int FLOATVEC_SIZE = 8;
#elif defined(__SSE__)
  typedef __m128 floatvec;
  const int FLOATVEC_SIZE = 4;
#else
#  error SIMD is not supported
#endif

/* floatvec is a primitive SIMD type contains packed float
 * FLOATVEC_SIZE is the number of float elements that a floatvec contains
 *
 * On x86 with SSE for example, floatvec is __m128 and FLOATVEC_SIZE = 4
 *
 */

static_assert(sizeof(floatvec) == sizeof(float) * FLOATVEC_SIZE, "");

/* 3-component vector type
 *
 * It contains XXXXYYYYZZZZ on x86 with SSE
 *
 */
struct Vec3 {
  floatvec x;
  floatvec y;
  floatvec z;
};

/* 4-component vector type
 *
 * It contains XXXXYYYYZZZZWWWW on x86 with SSE
 *
 */
struct Vec4 {
  floatvec x;
  floatvec y;
  floatvec z;
  floatvec w;
};

/* Load Vec3 from aligned memory address (AoS to SoA)
 *
 * On x86 with SSE for example, pxyz must contains XYZXYZXYZXYZ
 * => XXXXYYYYZZZZ in Vec3
 *
 * Note that pxyz must be aligned as floatvec
 */
inline void load(Vec3& v, const float* pxyz) noexcept;

/* Load Vec4 from aligned memory address (AoS to SoA)
 *
 * On x86 with SSE for example, pxyzw must contains XYZWXYZWXYZWXYZW
 * => XXXXYYYYZZZZWWWW in Vec4
 *
 * Note that pxyzw must be aligned as floatvec
 */
inline void load(Vec4& v, const float* pxyzw) noexcept;

/* Store Vec3 to alignemd memory address (SoA to AoS)
 *
 * On x86 with SSE for example, pxyz to store XYZXYZXYZXYZ
 *
 * Note that pxyz must be aligned as floatvec
 */
inline void store(float* pxyz, const Vec3& v) noexcept;

/* Store Vec4 to alignemd memory address (SoA to AoS)
 *
 * On x86 with SSE for example, pxyzw to store XYZWXYZWXYZWXYZW
 *
 * Note that pxyzw must be aligned as floatvec
 */
inline void store(float* pxyzw, const Vec4& v) noexcept;

}
}
