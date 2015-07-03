#pragma once

#include <vectormath.h>
#include <vectormath/aos/utils/ray3.h>
#include <QSize>
#include <math.h>

namespace recon {

using vectormath::aos::utils::Vec3;
using vectormath::aos::utils::Point3;
using vectormath::aos::utils::Mat4;
using vectormath::aos::utils::Ray3;

struct Epipolar {
  int width;
  int height;
  Mat4 txfm;
  Ray3 ray;

  float e_x;
  float e_y;
  float e_w;
  float d_x;
  float d_y;
  float d_w;
  float d_k;
  float e_px;
  float e_py;

  Epipolar(int w, int h, Mat4 m, Ray3 r)
  : width(w), height(h), txfm(m), ray(r)
  {
    Vec4 e = transform(txfm, r.start);
    Vec4 d = txfm * Vec4(r.diff, 0.0f);

    e_x = (float)e.x(), e_y = (float)e.y(), e_w = (float)e.w();
    d_x = (float)d.x(), d_y = (float)d.y(), d_w = (float)d.w();
    float a = (float)(d.x() - d.w() * e.x() / e.w());
    float b = (float)(d.y() - d.w() * e.y() / e.w());
    d_k = sqrtf(a * a + b * b);
    e_px = e_x / e_w;
    e_py = e_y / e_w;
  }

  inline float depth(float x, float y) const
  {
    float pdx = x - e_px, pdy = y - e_py;
    float len = sqrtf(pdx * pdx + pdy * pdy);
    float dp = pdx * d_x + pdy * d_y;
    float depth = (e_w * len) / (d_k - d_w * len);
    return copysignf(depth, dp);
  }

  template<bool fixed_inside = false, typename F>
  void walk(F f) const
  {
    Vec3 p0 = Vec3::proj(transform(txfm, ray[-1.0f]));
    Vec3 p1 = Vec3::proj(transform(txfm, ray[0.0f]));
    Vec3 p2 = Vec3::proj(transform(txfm, ray[1.0f]));
    p1 = copy_z(p1, Vec3::zero());
    p2 = copy_z(p2, Vec3::zero());
    Vec3 d = normalize(p2 - p1);
    float dx = (float)d.x(), dy = (float)d.y();
    float adx = fabsf(dx), ady = fabsf(dy);

    if (adx >= ady) { // Along X
      float m = (dy / dx);
      float b = (float)p0.y() - (float)p0.x() * m;
      float x0 = (float)p0.x();
      float x1 = (float)p2.x();
      if (x0 > x1)
        std::swap(x0, x1);
      auto invoke = [m,b,f,x0,x1,this](float x)
      {
        bool inside = (x0 <= x && x <= x1);
        if (!fixed_inside || inside) {
          float y = m * x + b;
          f(x, y, depth(x, y), inside);
        }
      };
      for (int x = 0, w = width; x < w; ++x) {
        invoke((float)x);
        //invoke((float)x + 0.25f);
        invoke((float)x + 0.5f);
        //invoke((float)x + 0.75f);
      }
    } else { // Along Y
      float m = (float)(dx / dy);
      float b = (float)p0.x() - (float)p0.y() * m;
      float y0 = (float)p0.y();
      float y1 = (float)p2.y();
      if (y0 > y1)
        std::swap(y0, y1);
      auto invoke = [m,b,f,y0,y1,this](float y)
      {
        bool inside = (y0 <= y && y <= y1);
        if (!fixed_inside || inside) {
          float x = m * y + b;
          f(x, y, depth(x, y), inside);
        }
      };
      for (int y = 0, h = height; y < h; ++y) {
        invoke((float)y);
        //invoke((float)y + 0.25f);
        invoke((float)y + 0.5f);
        //invoke((float)y + 0.75f);
      }
    }
  }
};

}
