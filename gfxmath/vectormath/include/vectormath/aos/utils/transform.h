#pragma once

#include <math.h>

namespace vectormath {
namespace aos {
namespace utils {

using namespace aos;

inline Mat4 affine_inverse(Mat4);
inline Mat4 ortho_inverse(Mat4);
inline Mat4 look_at(Vec3 eye, Vec3 at, Vec3 up);
inline Mat4 make_modelview(Vec3 center, Quat orientation);
inline Mat3 make_normalmatrix(Mat4 modelview);
inline Mat3 make_euler(float roll, float pitch, float yaw);
inline Mat4 perspective(float fovy, float aspect, float znear, float zfar);
inline Mat4 frustum(float left, float right, float bottom, float top, float znear, float zfar);
inline Mat4 orthographic(float left, float right, float bottom, float top, float znear, float zfar);

inline Mat4 affine_inverse(Mat4 txfm)
{
  Mat3 r = txfm.upper3x3();
  Vec3 t = Vec3::cast(txfm.column3());
  Mat3 ir = inverse(r);
  Vec3 it = -(ir * t);
  return Mat4{ ir, it };
}

inline Mat4 ortho_inverse(Mat4 txfm)
{
  Mat3 r = txfm.upper3x3();
  Vec3 t = Vec3::cast(txfm.column3());
  Mat3 ir = transpose(r);
  Vec3 it = -(ir * t);
  return Mat4{ ir, it };
}

inline Mat4 look_at(Vec3 eye, Vec3 at, Vec3 up)
{
  Vec3 xv, yv, zv;
  yv = normalize(up);
  zv = normalize(eye - at);
  xv = normalize(cross(yv, zv));
  yv = cross(zv, xv);
  return ortho_inverse(Mat4{ Mat3(xv, yv, zv), eye });
}

inline Mat4 make_modelview(Vec3 center, Quat orientation)
{
  Quat iorient = ~orientation;
  Mat3 rot = (Mat3)iorient;
  Vec3 trans = -(rot * center);
  return Mat4{ rot, trans };
}

inline Mat3 make_normalmatrix(Mat4 modelview)
{
  Mat3 mat = modelview.upper3x3();
  return transpose(inverse(mat));
}

inline Mat3 make_euler(float roll, float pitch, float yaw)
{
  float ci = cosf(roll), cj = cosf(pitch), ch = cosf(yaw);
  float si = sinf(roll), sj = sinf(pitch), sh = sinf(yaw);
  float cc = ci * ch, cs = ci * sh, sc = si * ch, ss = si * sh;

  return Mat3{
    Vec3{ cj * ch, cj * sh, -sj },
    Vec3{ sj * sc - cs, sj * ss + cc, cj * si },
    Vec3{ sj * cc + ss, sj * cs - cc, cj * ci }
  };
}

inline Mat4 perspective(float fovy, float aspect, float znear, float zfar)
{
  float f = 1.0f / tanf(0.5f * fovy);
  float x = f / aspect;
  float y = f;
  float a = (zfar + znear) / (znear - zfar);
  float b = 2.0f * zfar * znear / (znear - zfar);

  return Mat4{
    Vec4{ x, 0.0f, 0.0f, 0.0f },
    Vec4{ 0.0f, y, 0.0f, 0.0f },
    Vec4{ 0.0f, 0.0f, a, -1.0f },
    Vec4{ 0.0f, 0.0f, b, 0.0f }
  };
}

inline Mat4 frustum(float left, float right, float bottom, float top, float znear, float zfar)
{
  float x = 2.0f * znear / (right - left);
  float y = 2.0f * znear / (top - bottom);
  float a = (right + left) / (right - left);
  float b = (top + bottom) / (top - bottom);
  float c = -(zfar + znear) / (zfar - znear);
  float d = -2.0f * zfar * znear / (zfar - znear);

  return Mat4{
    Vec4{ x, 0.0f, 0.0f, 0.0f },
    Vec4{ 0.0f, y, 0.0f, 0.0f },
    Vec4{ a, b, c, -1.0f },
    Vec4{ 0.0f, 0.0f, d, 0.0f }
  };
}

inline Mat4 orthographic(float left, float right, float bottom, float top, float znear, float zfar)
{
  float sx = 2.0f / (right - left);
  float sy = 2.0f / (top - bottom);
  float sz = -2.0f / (zfar - znear);
  float tx = -(right + left) / (right - left);
  float ty = -(top + bottom) / (top - bottom);
  float tz = -(zfar + znear) / (zfar - znear);

  return Mat4{
    Vec4{ sx, 0.0f, 0.0f, 0.0f },
    Vec4{ 0.0f, sy, 0.0f, 0.0f },
    Vec4{ 0.0f, 0.0f, sz, 0.0f },
    Vec4{ tx, ty, tz, 1.0f }
  };
}

}
}
}
