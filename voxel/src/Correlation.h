#pragma once

#include <vectormath.h>
#include <vectormath/aos/utils/point3.h>
#include <stdint.h>
#include <math.h>
#include <QImage>

namespace recon {

using vectormath::aos::Vec3;
using vectormath::aos::Vec4;
using vectormath::aos::utils::Point3;

struct SampleWindow {
  bool valid;
  Vec3 color[121];

  SampleWindow()
  : valid(false)
  {
  }

  SampleWindow(const QImage& image, Vec3 xy)
  : SampleWindow()
  {
    set_bilinear_rgb32(image, xy);
    //set_bilinear(image, xy);
    //set_floor(image, xy);

    // normalize pixel value to 0.0 - 1.0
    for (int i = 0; i < 121; ++i) {
      color[i] = color[i] / 255.0f;
    }
  }

  inline Vec3 operator[](int i) const
  {
    Q_ASSERT(i >= 0 && i < 121);
    return color[i];
  }

  inline void set_floor(const QImage& image, Vec3 xy)
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
        color[i*11+j] = Vec3((float)qRed(c), (float)qGreen(c), (float)qBlue(c));
      }
    }
  }

  static inline float bilinear(float fx, float fy, float v00, float v10, float v01, float v11)
  {
    float v_0 = (1.0f - fx) * v00 + fx * v10;
    float v_1 = (1.0f - fx) * v01 + fx * v11;
    return (1.0f - fy) * v_0 + fy * v_1;
  }

  inline void set_bilinear(const QImage& image, Vec3 xy)
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
          float cr = bilinear(fx, fy, r[i][j], r[i][j+1], r[i+1][j], r[i+1][j+1]);
          float cg = bilinear(fx, fy, g[i][j], g[i][j+1], g[i+1][j], g[i+1][j+1]);
          float cb = bilinear(fx, fy, b[i][j], b[i][j+1], b[i+1][j], b[i+1][j+1]);
          color[index] = Vec3(cr, cg, cb);
        }
      }
      valid = true;
    }
  }

  inline void set_bilinear_rgb32(const QImage& image, Vec3 xy)
  {
    Q_ASSERT(image.format() == QImage::Format_RGB32);

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
        int y = py - 5 + i;
        y = (y < 0 ? 0 : y);
        y = (y < height ? y : height-1);
        const QRgb* row = (const QRgb*)image.constScanLine(y);

        for (int j = 0; j < 12; ++j) {
          int x = px - 5 + j;
          x = (x < 0 ? 0 : x);
          x = (x < width ? x : width-1);
          QRgb c = row[x];
          r[i][j] = qRed(c);
          g[i][j] = qGreen(c);
          b[i][j] = qBlue(c);
        }
      }

      for (int i = 0; i < 11; ++i) {
        for (int j = 0; j < 11; ++j) {
          int index = i*11+j;
          float cr = bilinear(fx, fy, r[i][j], r[i][j+1], r[i+1][j], r[i+1][j+1]);
          float cg = bilinear(fx, fy, g[i][j], g[i][j+1], g[i+1][j], g[i+1][j+1]);
          float cb = bilinear(fx, fy, b[i][j], b[i][j+1], b[i+1][j], b[i+1][j+1]);
          color[index] = Vec3(cr, cg, cb);
        }
      }
      valid = true;
    }
  }
};

static const Mat3 RGB_TO_YUV = Mat3{
  Vec3{ 0.299f, -0.147f, 0.615f },
  Vec3{ 0.587f, -0.289f, -0.515f },
  Vec3{ 0.114f, 0.436f, -0.100f }
};

static const Vec3 RGB_TO_GRAY = Vec3( 0.299f, 0.587f, 0.114f );

struct NormalizedCrossCorrelation {
  float value;

  NormalizedCrossCorrelation(const SampleWindow& wi, const SampleWindow& wj)
  : value((wi.valid && wj.valid) ? zncc(wi, wj) : -1.0f)
  {
  }

  static float zncc(const SampleWindow& wi, const SampleWindow& wj)
  {
    // convert RGB to GRAY
    float yi[121], yj[121];
    for (int i = 0; i < 121; ++i) {
      yi[i] = (float)dot(RGB_TO_GRAY, wi[i]);
      yj[i] = (float)dot(RGB_TO_GRAY, wj[i]);
    }

    // compute mean
    float ai = 0.0f, aj = 0.0f;
    for (int i = 0; i < 121; ++i) {
      ai += yi[i] / 121.0f;
      aj += yj[i] / 121.0f;
    }

    // compute variance
    float si = 0.0f, sj = 0.0f;
    for (int i = 0; i < 121; ++i) {
      float di = yi[i] - ai;
      float dj = yj[i] - aj;
      si += di * di;
      sj += dj * dj;
    }
    if (si == 0.000001f || sj == 0.000001f)
      return -1.0f;
    //si = sqrtf(si);
    //sj = sqrtf(sj);
    float denom = sqrtf(si * sj);

    // compute dot product of two normalized vector
    float ncc = 0.0f;
    for (int i = 0; i < 121; ++i) {
      float di = yi[i] - ai;
      float dj = yj[i] - aj;
      //ncc += (di / si) * (dj / sj);
      ncc += di * dj / denom;
    }

    //if (isnan(ncc))
    //  return -1.0f;
    return ncc;
  }

  /*static float zncc_sse(const SampleWindow& wi, const SampleWindow& wj)
  {
    // convert RGB to GRAY
    __m128 yi[121], yj[121];
    for (int i = 0; i < 121; ++i)
      yi[i] = dot(RGB_TO_GRAY, wi[i]).xmm_data;
    for (int i = 0; i < 121; ++i)
      yj[i] = dot(RGB_TO_GRAY, wj[i]).xmm_data;

    // compute mean
    __m128 ai = _mm_setzero_ps(), aj = _mm_setzero_ps();
    for (int i = 0; i < 121; ++i)
      ai = _mm_add_ss(ai, yi[i]);
    for (int i = 0; i < 121; ++i)
      ai = _mm_add_ss(ai, yi[i]);
    __m128 denom = _mm_set_ss(121.0f);
    ai = _mm_div_ss(ai, denom);
    aj = _mm_div_ss(aj, denom);

    // compute variance
    __m128 si = _mm_setzero_ps(), sj = _mm_setzero_ps();
    for (int i = 0; i < 121; ++i) {
      __m128 d = _mm_sub_ss(yi[i], ai);
      si = _mm_add_ss(_mm_mul_ss(d, d), si);
    }
    for (int i = 0; i < 121; ++i) {
      __m128 d = _mm_sub_ss(yj[i], aj);
      sj = _mm_add_ss(_mm_mul_ss(d, d), sj);
    }
    si = _mm_sqrt_ss(si);
    sj = _mm_sqrt_ss(sj);

    // compute dot product of two normalized vector
    __m128 ncc = _mm_setzero_ps();
    for (int i = 0; i < 121; ++i) {
      __m128 di = _mm_sub_ss(yi[i], ai);
      __m128 dj = _mm_sub_ss(yj[i], aj);
      di = _mm_div_ss(di, si);
      dj = _mm_div_ss(dj, sj);
      ncc = _mm_add_ss(_mm_mul_ps(di, dj), ncc);
    }

    float result = _mm_cvtss_f32(ncc);
    if (isnan(result))
      return -1.0f;
    return result;
  }*/

  inline operator float() const
  {
    return value;
  }
};

}
