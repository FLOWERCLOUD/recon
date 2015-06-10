#pragma once

#include <math.h>

namespace vectormath {
namespace utils {

inline mat4 affine_inverse(mat4);
inline mat4 ortho_inverse(mat4);

inline mat4 affine_inverse(mat4 txfm)
{
  mat3 r = txfm.upper3x3();
  vec3 t = to_vec3(txfm.column3());
  mat3 ir = inverse(r);
  vec3 it = -(ir * t);
  return mat4{ ir, it };
}

inline mat4 ortho_inverse(mat4 txfm)
{
  mat3 r = txfm.upper3x3();
  vec3 t = to_vec3(txfm.column3());
  mat3 ir = transpose(r);
  vec3 it = -(ir * t);
  return mat4{ ir, it };
}

inline mat4 look_at(vec3 eye, vec3 at, vec3 up);
inline mat4 make_modelview(vec3 center, quat orientation);
inline mat3 make_normalmatrix(mat4 modelview);
inline mat4 perspective(float fovy, float aspect, float znear, float zfar);
inline mat4 frustum(float left, float right, float bottom, float top, float znear, float zfar);
inline mat4 orthographic(float left, float right, float bottom, float top, float znear, float zfar);

inline mat4 look_at(vec3 eye, vec3 at, vec3 up)
{
  vec3 xv, yv, zv;
  yv = normalize(up);
  zv = normalize(eye - at);
  xv = normalize(cross(yv, zv));
  yv = cross(zv, xv);

  return ortho_inverse(mat4{ mat3(xv, yv, zv), eye });
}

inline mat4 make_modelview(vec3 center, quat orientation)
{
  quat iorient = ~orientation;
  mat3 rot = (mat3)iorient;
  vec3 trans = -(rot * center);
  return mat4{ rot, trans };
}

inline mat3 make_normalmatrix(mat4 modelview)
{
  mat3 mat = modelview.upper3x3();
  return transpose(inverse(mat));
}

inline mat4 perspective(float fovy, float aspect, float znear, float zfar)
{
  float f = 1.0f / tanf(0.5f * fovy);
  float x = f / aspect;
  float y = f;
  float a = (zfar + znear) / (znear - zfar);
  float b = 2.0f * zfar * znear / (znear - zfar);

  return mat4{
    vec4{ x, 0.0f, 0.0f, 0.0f },
    vec4{ 0.0f, y, 0.0f, 0.0f },
    vec4{ 0.0f, 0.0f, a, -1.0f },
    vec4{ 0.0f, 0.0f, b, 0.0f }
  };
}

inline mat4 frustum(float left, float right, float bottom, float top, float znear, float zfar)
{
  float x = 2.0f * znear / (right - left);
  float y = 2.0f * znear / (top - bottom);
  float a = (right + left) / (right - left);
  float b = (top + bottom) / (top - bottom);
  float c = -(zfar + znear) / (zfar - znear);
  float d = -2.0f * zfar * znear / (zfar - znear);

  return mat4{
    vec4{ x, 0.0f, 0.0f, 0.0f },
    vec4{ 0.0f, y, 0.0f, 0.0f },
    vec4{ a, b, c, -1.0f },
    vec4{ 0.0f, 0.0f, d, 0.0f }
  };
}

inline mat4 orthographic(float left, float right, float bottom, float top, float znear, float zfar)
{
  float sx = 2.0f / (right - left);
  float sy = 2.0f / (top - bottom);
  float sz = -2.0f / (zfar - znear);
  float tx = -(right + left) / (right - left);
  float ty = -(top + bottom) / (top - bottom);
  float tz = -(zfar + znear) / (zfar - znear);

  return mat4{
    vec4{ sx, 0.0f, 0.0f, 0.0f },
    vec4{ 0.0f, sy, 0.0f, 0.0f },
    vec4{ 0.0f, 0.0f, sz, 0.0f },
    vec4{ tx, ty, tz, 1.0f }
  };
}

}
}
