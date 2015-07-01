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
  float red[121];
  float green[121];
  float blue[121];

  SampleWindow()
  : valid(false)
  {
  }

  SampleWindow(const QImage& image, Vec3 xy)
  : SampleWindow()
  {
    set_bilinear(image, xy);
    //set_floor(image, xy);

    // normalize pixel value to 0.0 - 1.0
    for (int i = 0; i < 121; ++i) {
      red[i] /= 255.0f;
      green[i] /= 255.0f;
      blue[i] /= 255.0f;
    }
  }

  inline Vec3 operator[](int i) const
  {
    return Vec3{ red[i], green[i], blue[i] };
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
          red[index] = bilinear(fx, fy, r[i][j], r[i][j+1], r[i+1][j], r[i+1][j+1]);
          green[index] = bilinear(fx, fy, g[i][j], g[i][j+1], g[i+1][j], g[i+1][j+1]);
          blue[index] = bilinear(fx, fy, b[i][j], b[i][j+1], b[i+1][j], b[i+1][j+1]);
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
      si += powf((yi[i] - ai), 2);
      sj += powf((yj[i] - aj), 2);
    }
    si = sqrtf(si);
    sj = sqrtf(sj);

    // compute dot product of two normalized vector
    float ncc = 0.0f;
    for (int i = 0; i < 121; ++i) {
      ncc += ((yi[i] - ai) / si) * ((yj[i] - aj) / sj);
    }

    if (isnan(ncc))
      return -1.0f;
    return ncc;
  }

  inline operator float() const
  {
    return value;
  }
};

}
