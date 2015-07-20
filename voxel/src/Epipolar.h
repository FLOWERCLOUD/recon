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
    return copy_z(Vec3::proj(ve + d * vd), Vec3(d,d,d));
  }

  inline float solve_depth(Vec3 xy) const
  {
    Vec3 delta = copy_z(xy - Vec3::proj(ve), Vec3::zero());
    //float len = (float)length(delta);
    float len = sqrtf((float)dot(delta, delta));
    float dp = (float)dot(delta, Vec3::proj(vd));
    float e_w = (float)ve.w();
    float d_w = (float)vd.w();
    float t = (e_w * len) / (d_k - d_w * len);
    return copysignf(t, dp);
  }

  //
  // F: void func(Vec3 pt0, Vec3 pt1)
  //    where pt0, pt1 are 2D points with z = depth
  //
  template<bool GLOBAL = true, typename F>
  void per_pixel(F f, float drange = 1.0f) const
  {
    // end points
    Vec3 ep0 = lerp2D(-drange), ep1 = lerp2D(drange);
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
        //f(x0, y0, x1, y1);
      };
      //
      if (GLOBAL) {
        for (int ix = 0, w = width; ix < w; ++ix) {
          float x = ix;
          invoke(x, x+0.5f);
          invoke(x+0.5f, x+1.0f);
          //invoke(x, x + 1.0f);
        }
      } else {
        float ex0 = (float)ep0.x();
        float ex1 = (float)ep1.x();
        if (ex0 > ex1)
          std::swap(ex0, ex1);
        ex0 = floorf(ex0), ex1 = ceilf(ex1);
        //printf("ex %f %f\n", ex0, ex1);
        for (int ix = ex0, ix2 = ex1; ix <= ix2; ++ix) {
          float x = ix;
          invoke(x, x+0.5f);
          invoke(x+0.5f, x+1.0f);
          //invoke(x, x + 1.0f);
        }
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
        //f(x0, y0, x1, y1);
      };
      //
      if (GLOBAL) {
        for (int iy = 0, h = height; iy < h; ++iy) {
          float y = iy;
          invoke(y, y+0.5f);
          invoke(y+0.5f, y+1.0f);
          //invoke(y, y + 1.0f);
        }
      } else {
        float ey0 = (float)ep0.y();
        float ey1 = (float)ep1.y();
        if (ey0 > ey1)
          std::swap(ey0, ey1);
        ey0 = floorf(ey0), ey1 = ceilf(ey1);
        for (int iy = ey0, iy2 = ey1; iy <= iy2; ++iy) {
          float y = iy;
          invoke(y, y+0.5f);
          invoke(y+0.5f, y+1.0f);
          //invoke(y, y + 1.0f);
        }
      }
    }
  }
};

}
