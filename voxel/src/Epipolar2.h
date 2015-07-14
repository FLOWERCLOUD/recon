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

  float d_k;
  Vec4 ve;
  Vec4 vd;

  Epipolar(int w, int h, Mat4 txfm, Ray3 ray)
  : width(w), height(h)
  {
    Vec4 e = transform(txfm, ray.start);
    Vec4 d = txfm * Vec4(ray.diff, 0.0f);
    float a = (float)(d.x() - d.w() * e.x() / e.w());
    float b = (float)(d.y() - d.w() * e.y() / e.w());
    d_k = sqrtf(a * a + b * b);
    ve = e;
    vd = d;
  }

  inline Vec3 lerp2D(float d) const
  {
    return Vec3::proj(ve + d * vd);
  }

  inline float solve_depth(Vec3 xy) const
  {
    Vec3 delta = copy_z(xy - Vec3::proj(ve), Vec3::zero());
    float len = (float)length(delta);
    float dp = (float)dot(delta, Vec3::cast(vd));
    float e_w = (float)ve.w();
    float d_w = (float)vd.w();
    float t = (e_w * len) / (d_k - d_w * len);
    return copysignf(t, dp);
  }

  template<bool GLOBAL = true, typename F>
  void per_pixel(F f, float arange = 1.0f) const
  {
    // end points
    Vec3 ep0 = lerp2D(-arange), ep1 = lerp2D(arange);
    float dx = (float)((ep1 - ep0).x());
    float dy = (float)((ep1 - ep0).y());
    if (fabsf(dx) >= fabsf(dy)) { // along X
      float m = (dy / dx);
      float b = (float)ep0.y() - (float)ep0.x() * m;
      auto invoke = [m,b,f,this](float x0, float x1)
      {
        float y0 = m * x0 + b;
        float y1 = m * x1 + b;
        Vec3 p0 = Vec3(x0,y0, solve_depth(Vec3(x0,y0,0.0)));
        Vec3 p1 = Vec3(x1,y1, solve_depth(Vec3(x1,y1,0.0)));
        f(p0, p1);
      };
      //
      if (GLOBAL) {
        for (int x = 0, w = width; x < w; ++x)
          invoke((float)x, (float)(x+1));
      } else {
        float ex0 = (float)ep0.x();
        float ex1 = (float)ep1.x();
        if (ex0 > ex1)
          std::swap(ex0, ex1);
        ex0 = floorf(ex0), ex1 = ceilf(ex1);
        for (int x = ex0; x < ex1; ++x)
          invoke((float)x, (float)(x+1));
      }
    } else { // along Y
      float m = (dx / dy);
      float b = (float)ep0.x() - (float)ep0.y() * m;
      auto invoke = [m,b,f,this](float y0, float y1)
      {
        float x0 = m * y0 + b;
        float x1 = m * y1 + b;
        Vec3 p0 = Vec3(x0,y0, solve_depth(Vec3(x0,y0,0.0)));
        Vec3 p1 = Vec3(x1,y1, solve_depth(Vec3(x1,y1,0.0)));
        f(p0, p1);
      };
      //
      if (GLOBAL) {
        for (int y = 0, h = height; y < h; ++h)
          invoke((float)y, (float)(y+1));
      } else {
        float ey0 = (float)ep0.y();
        float ey1 = (float)ep1.y();
        if (ey0 > ey1)
          std::swap(ey0, ey1);
        ey0 = floorf(ey0), ey1 = ceilf(ey1);
        for (int y = ey0; y < ey1; ++y)
          invoke((float)y, (float)(y+1));
      }
    }
  }
};

}
