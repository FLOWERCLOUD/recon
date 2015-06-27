#pragma once

#include <vectormath.h>
#include <vectormath/aos/utils/point3.h>
#include <stdint.h>
#include <math.h>
#include <QImage>

namespace recon {

using vectormath::aos::vec3;
using vectormath::aos::vec4;
using vectormath::aos::utils::point3;

struct SampleWindow {
  bool valid;
  float red[121];
  float green[121];
  float blue[121];

  SampleWindow()
  : valid(false)
  {
  }

  SampleWindow(const QImage& image, vec3 xy)
  : SampleWindow()
  {
    set_bilinear(image, xy);
    //set_floor(image, xy);
  }

  inline void set_floor(const QImage& image, vec3 xy)
  {
    int width = image.width(), height = image.height();
    int px = (float)xy.x(), py = (float)xy.y();
    int px0 = px - 5, py0 = py - 5;

    this->valid = image.valid(px, py);

    for (int i = 0; i < 11; ++i) {
      for (int j = 0; j < 11; ++j) {
        int x = px0 + j, y = py0 + i;
        x = (x < 0 ? 0 : x);
        x = (x < width ? x : width-1);
        y = (y < 0 ? 0 : y);
        y = (y < height ? y : height-1);

        QRgb c = image.pixel(x, y);

        int index = (i) * 11 + (j);
        red[ index ] = qRed(c);
        green[ index ] = qGreen(c);
        blue[ index ] = qBlue(c);
      }
    }
  }

  static inline float bilinear(float fx, float fy, float v00, float v10, float v01, float v11)
  {
    float v_0 = (1.0f - fx) * v00 + fx * v10;
    float v_1 = (1.0f - fx) * v01 + fx * v11;
    return (1.0f - fy) * v_0 + fy * v_1;
  }

  inline void set_bilinear(const QImage& image, vec3 xy)
  {
    int width = image.width(), height = image.height();

    float ix, iy;
    float fx = modff((float)xy.x(), &ix);
    float fy = modff((float)xy.y(), &iy);

    int px = (int)ix, py = (int)iy;
    if (px >= 0 && py >= 0 && px < width && py < height) {
      uint8_t r[12][12];
      uint8_t g[12][12];
      uint8_t b[12][12];

      for (int i = 0; i < 12; ++i) {
        for (int j = 0; j < 12; ++j) {
          int x = px - 5 + j, y = py - 5 + i;
          x = (x < 0 ? 0 : x);
          x = (x < width ? x : width-1);
          y = (y < 0 ? 0 : y);
          y = (y < height ? y : height-1);
          QRgb c = image.pixel(x, y);
          r[i][j] = qRed(c);
          g[i][j] = qGreen(c);
          b[i][j] = qBlue(c);
        }
      }

      for (int i = 0; i < 11; ++i) {
        for (int j = 0; j < 11; ++j) {
          int index = i*11+j;
          red[index] = bilinear(fx, fy, r[i][j], r[i][j+1], r[i+1][j], r[i+1][j+1]);
          green[index] = bilinear(fx, fy, g[i][j], g[i][j+1], g[i+1][j], g[i+1][j+1]);
          blue[index] = bilinear(fx, fy, b[i][j], b[i][j+1], b[i+1][j], b[i+1][j+1]);
        }
      }
      valid = true;
    }
  }
};

static const mat3 RGB_TO_YUV = mat3{
  vec3{ 0.299f, -0.147f, 0.615f },
  vec3{ 0.587f, -0.289f, -0.515f },
  vec3{ 0.114f, 0.436f, -0.100f }
};

struct NormalizedCrossCorrelation {
  float value;

  NormalizedCrossCorrelation(const SampleWindow& wi, const SampleWindow& wj)
  : value((wi.valid && wj.valid) ? zncc(wi, wj) : -1.0f)
  {
  }

  static float zncc(const SampleWindow& wi, const SampleWindow& wj)
  {
    vec3 avg1 = vec3::zero(), avg2 = vec3::zero();
    for (int i = 0; i < 121; ++i)
      avg1 = avg1 + vec3(wi.red[i], wi.green[i], wi.blue[i]) / 121.0f;
    for (int i = 0; i < 121; ++i)
      avg2 = avg2 + vec3(wj.red[i], wj.green[i], wj.blue[i]) / 121.0f;

    vec3 s1 = vec3::zero(), s2 = vec3::zero();
    for (int i = 0; i < 121; ++i) {
      vec3 v1 = vec3(wi.red[i], wi.green[i], wi.blue[i]) - avg1;
      v1 = RGB_TO_YUV * v1;
      s1 = s1 + square(v1);
    }
    for (int i = 0; i < 121; ++i) {
      vec3 v2 = vec3(wj.red[i], wj.green[i], wj.blue[i]) - avg2;
      v2 = RGB_TO_YUV * v2;
      s2 = s2 + square(v2);
    }
    s1 = rsqrt(s1);
    s2 = rsqrt(s2);

    vec3 ncc = vec3::zero();
    for (int i = 0; i < 121; ++i) {
      vec3 v1 = vec3(wi.red[i], wi.green[i], wi.blue[i]) - avg1;
      vec3 v2 = vec3(wj.red[i], wj.green[i], wj.blue[i]) - avg2;
      v1 = RGB_TO_YUV * v1;
      v2 = RGB_TO_YUV * v2;
      ncc = ncc + mul(mul(v1, s1), mul(v2, s2));
    }

    float y = (float)ncc.x();
    float u = (float)ncc.y();
    float v = (float)ncc.z();

    if (isnan(y) || isnan(u) || isnan(v))
      return -1.0f;

    return 0.5f * y + 0.25f * u + 0.25f * v;
    //return y;
  }

  inline operator float() const
  {
    return value;
  }
};

}
