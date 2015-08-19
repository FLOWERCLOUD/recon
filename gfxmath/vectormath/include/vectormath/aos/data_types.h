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
namespace aos {

struct Float;
struct PFloat;
struct Vec3;
struct Vec4;
struct Mat3;
struct Mat4;
struct Quat;
struct DQuat;

struct Float {
  __m128 xmm_data;

  inline Float() noexcept = default;
  inline Float(const Float&) noexcept = default;
  inline Float& operator=(const Float&) noexcept = default;

  explicit inline Float(__m128 a) noexcept;

  inline Float(float x) noexcept;
  explicit inline operator float() const noexcept;

  static inline Float load(const float* ptr) noexcept;
  inline void store(float* ptr) const noexcept;
};

struct PFloat {
  __m128 xmm_data;

  inline PFloat() noexcept = default;
  inline PFloat(const PFloat&) noexcept = default;
  inline PFloat& operator=(const PFloat&) noexcept = default;

  explicit inline PFloat(__m128 a) noexcept;
  inline PFloat(Float x) noexcept;
  inline PFloat(float x) noexcept;
  explicit inline operator float() const noexcept;
  inline operator Float() const noexcept;
};

struct Vec3 {
  __m128 xmm_data;

  inline Vec3() noexcept = default;
  inline Vec3(const Vec3&) noexcept = default;
  inline Vec3& operator=(const Vec3&) noexcept = default;

  explicit inline Vec3(__m128) noexcept;
  explicit inline Vec3(float, float, float) noexcept;

  static inline Vec3 zero() noexcept;
  static inline Vec3 proj(Vec4);
  static inline Vec3 cast(Vec4);

  static inline Vec3 load(const float*) noexcept;
  inline void store(float*) const noexcept;

  inline PFloat x() const noexcept;
  inline PFloat y() const noexcept;
  inline PFloat z() const noexcept;
};

struct Vec4 {
  __m128 xmm_data;

  inline Vec4() noexcept = default;
  inline Vec4(const Vec4&) noexcept = default;
  inline Vec4& operator=(const Vec4&) noexcept = default;

  explicit inline Vec4(__m128) noexcept;
  explicit inline Vec4(float, float, float, float) noexcept;
  explicit inline Vec4(Vec3, Float) noexcept;

  static inline Vec4 zero() noexcept;

  static inline Vec4 load(const float*) noexcept;
  inline void store(float*) const noexcept;

  inline PFloat x() const noexcept;
  inline PFloat y() const noexcept;
  inline PFloat z() const noexcept;
  inline PFloat w() const noexcept;
};

struct Mat3 {
  __m128 xmm_column0;
  __m128 xmm_column1;
  __m128 xmm_column2;

  inline Mat3() noexcept = default;
  inline Mat3(const Mat3&) noexcept = default;
  inline Mat3& operator=(const Mat3&) noexcept = default;

  explicit inline Mat3(__m128, __m128, __m128) noexcept;
  explicit inline Mat3(Vec3, Vec3, Vec3) noexcept;

  static inline Mat3 zero() noexcept;
  static inline Mat3 identity() noexcept;
  static inline Mat3 scaling(PFloat, PFloat, PFloat) noexcept;
  static inline Mat3 rotation(PFloat, Vec3);

  static inline Mat3 load(const float*) noexcept;
  inline void store(float*) const noexcept;

  inline Vec3 column0() const noexcept;
  inline Vec3 column1() const noexcept;
  inline Vec3 column2() const noexcept;
};

struct Mat4 {
  __m128 xmm_column0;
  __m128 xmm_column1;
  __m128 xmm_column2;
  __m128 xmm_column3;

  inline Mat4() noexcept = default;
  inline Mat4(const Mat4&) noexcept = default;
  inline Mat4& operator=(const Mat4&) noexcept = default;

  explicit inline Mat4(__m128, __m128, __m128, __m128) noexcept;
  explicit inline Mat4(Vec4, Vec4, Vec4, Vec4) noexcept;
  explicit inline Mat4(Mat3, Vec3) noexcept;

  static inline Mat4 zero() noexcept;
  static inline Mat4 identity() noexcept;

  static inline Mat4 load(const float*) noexcept;
  inline void store(float*) const noexcept;

  inline Vec4 column0() const noexcept;
  inline Vec4 column1() const noexcept;
  inline Vec4 column2() const noexcept;
  inline Vec4 column3() const noexcept;

  inline Mat3 upper3x3() const noexcept;
};

struct Quat {
  __m128 xmm_data;

  inline Quat() noexcept = default;
  inline Quat(const Quat&) noexcept = default;
  inline Quat& operator=(const Quat&) noexcept = default;

  explicit inline Quat(__m128) noexcept;
  explicit inline Quat(float, float, float, float) noexcept;
  explicit inline Quat(Vec3, float) noexcept;

  static inline Quat zero() noexcept;
  static inline Quat identity() noexcept;
  static inline Quat rotation(PFloat, Vec3) noexcept;
  static inline Quat rotation(Vec3, Vec3) noexcept;
  static inline Quat rotation(Mat3) noexcept;

  static inline Quat load(const float*) noexcept;
  inline void store(float*) const noexcept;

  explicit inline operator Mat3() const noexcept;

  inline PFloat x() const noexcept;
  inline PFloat y() const noexcept;
  inline PFloat z() const noexcept;
  inline PFloat w() const noexcept;
  inline Vec3 v() const noexcept;
};

struct DQuat {
  __m128 xmm_real;
  __m128 xmm_dual;

  inline DQuat() noexcept = default;
  inline DQuat(const DQuat&) noexcept = default;
  inline DQuat& operator=(const DQuat&) noexcept = default;

  explicit inline DQuat(__m128, __m128) noexcept;
  explicit inline DQuat(Quat, Quat) noexcept;

  static inline DQuat identity() noexcept;
  static inline DQuat transform(Quat, Vec3) noexcept;

  explicit inline operator Mat4() const noexcept;

  inline Quat real() const noexcept;
  inline Quat dual() const noexcept;
};

// MARK: Float
inline Float operator-(Float) noexcept;
inline Float operator+(Float, Float) noexcept;
inline Float operator-(Float, Float) noexcept;
inline Float operator*(Float, Float) noexcept;
inline Float operator/(Float, Float) noexcept;
inline Float abs(Float) noexcept;

// MARK: PFloat
inline PFloat operator-(PFloat) noexcept;
inline PFloat operator+(PFloat, PFloat) noexcept;
inline PFloat operator-(PFloat, PFloat) noexcept;
inline PFloat operator*(PFloat, PFloat) noexcept;
inline PFloat operator/(PFloat, PFloat) noexcept;
inline PFloat abs(PFloat) noexcept;

// MARK: Vec3
inline Vec3 operator-(Vec3) noexcept;
inline Vec3 operator+(Vec3, Vec3) noexcept;
inline Vec3 operator-(Vec3, Vec3) noexcept;
inline Vec3 operator*(Vec3, PFloat) noexcept;
inline Vec3 operator*(PFloat, Vec3) noexcept;
inline Vec3 operator/(Vec3, PFloat) noexcept;
inline Vec3 abs(Vec3) noexcept;
inline PFloat dot(Vec3, Vec3) noexcept;
inline Vec3 cross(Vec3, Vec3) noexcept;
inline Vec3 normalize(Vec3) noexcept;
inline Float length(Vec3) noexcept;
inline Vec3 lerp(PFloat, Vec3, Vec3) noexcept;
inline Vec3 min(Vec3, Vec3) noexcept;
inline Vec3 max(Vec3, Vec3) noexcept;
inline Vec3 copy_x(Vec3, Vec3) noexcept;
inline Vec3 copy_y(Vec3, Vec3) noexcept;
inline Vec3 copy_z(Vec3, Vec3) noexcept;
inline Vec3 clamp(Vec3, PFloat, PFloat) noexcept;
inline Vec3 clamp(Vec3, Vec3, Vec3) noexcept;
inline Vec3 square(Vec3) noexcept;
inline Vec3 sqrt(Vec3) noexcept;
inline Vec3 rsqrt(Vec3) noexcept;
inline Vec3 mul(Vec3, Vec3) noexcept;

// MARK: Vec4
inline Vec4 operator-(Vec4) noexcept;
inline Vec4 operator+(Vec4, Vec4) noexcept;
inline Vec4 operator-(Vec4, Vec4) noexcept;
inline Vec4 operator*(Vec4, PFloat) noexcept;
inline Vec4 operator*(PFloat, Vec4) noexcept;
inline Vec4 operator/(Vec4, PFloat) noexcept;
inline Vec4 abs(Vec4) noexcept;
inline PFloat dot(Vec4, Vec4) noexcept;
inline Vec4 normalize(Vec4) noexcept;
inline Float length(Vec4) noexcept;
inline Vec4 lerp(PFloat, Vec4, Vec4) noexcept;

// MARK: Mat3
inline Mat3 operator-(Mat3) noexcept;
inline Mat3 operator+(Mat3, Mat3) noexcept;
inline Mat3 operator-(Mat3, Mat3) noexcept;
inline Mat3 operator*(Mat3, PFloat) noexcept;
inline Mat3 operator*(PFloat, Mat3) noexcept;
inline Mat3 operator/(Mat3, PFloat) noexcept;
inline Vec3 operator*(Mat3, Vec3) noexcept;
inline Mat3 operator*(Mat3, Mat3) noexcept;
inline Mat3 transpose(Mat3) noexcept;
inline PFloat determinant(Mat3) noexcept;
inline Mat3 inverse(Mat3) noexcept;
inline PFloat trace(Mat3) noexcept;

// MARK: Mat4
inline Mat4 operator-(Mat4) noexcept;
inline Mat4 operator+(Mat4, Mat4) noexcept;
inline Mat4 operator-(Mat4, Mat4) noexcept;
inline Mat4 operator*(Mat4, PFloat) noexcept;
inline Mat4 operator*(PFloat, Mat4) noexcept;
inline Mat4 operator/(Mat4, PFloat) noexcept;
inline Vec4 operator*(Mat4, Vec4) noexcept;
inline Mat4 operator*(Mat4, Mat4) noexcept;
inline Mat4 transpose(Mat4) noexcept;
inline Float determinant(Mat4) noexcept;
inline Mat4 inverse(Mat4) noexcept;
inline PFloat trace(Mat4) noexcept;

// MARK: Quat
inline Quat operator-(Quat) noexcept;
inline Quat operator~(Quat) noexcept;
inline Quat operator+(Quat, Quat) noexcept;
inline Quat operator-(Quat, Quat) noexcept;
inline Quat operator*(Quat, PFloat) noexcept;
inline Quat operator*(PFloat, Quat) noexcept;
inline Quat operator/(Quat, PFloat) noexcept;
inline Quat operator*(Quat, Quat) noexcept;
inline Vec3 rotate(Quat, Vec3) noexcept;
inline Quat normalize(Quat) noexcept;
inline Quat lerp(PFloat, Quat, Quat) noexcept;
inline Quat slerp(Float, Quat, Quat) noexcept;
inline Quat squad(Float, Quat, Quat, Quat, Quat) noexcept;

// MARK: DQuat
inline DQuat operator-(DQuat) noexcept;
inline DQuat operator+(DQuat, DQuat) noexcept;
inline DQuat operator-(DQuat, DQuat) noexcept;
inline DQuat operator*(DQuat, PFloat) noexcept;
inline DQuat operator*(PFloat, DQuat) noexcept;
inline DQuat operator/(DQuat, PFloat) noexcept;
inline DQuat operator*(DQuat, DQuat) noexcept;
inline Vec3 transform(DQuat, Vec3) noexcept;
inline DQuat lerp(PFloat, DQuat, DQuat) noexcept;

}
}
