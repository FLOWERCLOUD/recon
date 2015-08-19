#include "test_helper.h"
#include <catch.hpp>
#include <string.h>

static float fdata[] = {
  0.0f,
  (float)M_PI / 6.0f,
  (float)M_PI / 4.0f,
  (float)M_PI / 3.0f,
  (float)M_PI * 0.5f,
  (float)M_PI * 2.0f / 3.0f,
  (float)M_PI * 0.75f,
  (float)M_PI,
  -0.0f,
  -(float)M_PI / 6.0f,
  -(float)M_PI / 4.0f,
  -(float)M_PI / 3.0f,
  -(float)M_PI * 0.5f,
  -(float)M_PI * 2.0f / 3.0f,
  -(float)M_PI * 0.75f,
  -(float)M_PI
};

static float fdatasin[] = {
  0.0f,
  0.5f,
  0.707106781186548f,
  0.8660254037844386f,
  1.0f,
  0.8660254037844386f,
  0.707106781186548f,
  0.0f,
  -0.0f,
  -0.5f,
  -0.707106781186548f,
  -0.8660254037844386f,
  -1.0f,
  -0.8660254037844386f,
  -0.707106781186548f,
  -0.0f
};

static float fdatacos[] = {
  1.0f,
  0.8660254037844386f,
  0.707106781186548f,
  0.5f,
  0.0f,
  -0.5f,
  -0.707106781186548f,
  -1.0f,
  1.0f,
  0.8660254037844386f,
  0.707106781186548f,
  0.5f,
  0.0f,
  -0.5f,
  -0.707106781186548f,
  -1.0f
};

#define FDATA_COUNT 16

TEST_CASE("sincos(x, &s, &c)", "[sincos]") {
  SECTION("float4") {
    float4 x, s, c;
    float4_float u;

    for (int i = 0; i < FDATA_COUNT; i+=4) {
      memcpy(u.f, fdata + i, sizeof(float) * 4);
      x = u.vf;
      sincosf4(x, &s, &c);

      u.vf = s;
      REQUIRE(u.f[0] == Approx(fdatasin[i+0]));
      REQUIRE(u.f[1] == Approx(fdatasin[i+1]));
      REQUIRE(u.f[2] == Approx(fdatasin[i+2]));
      REQUIRE(u.f[3] == Approx(fdatasin[i+3]));

      u.vf = c;
      REQUIRE(u.f[0] == Approx(fdatacos[i+0]));
      REQUIRE(u.f[1] == Approx(fdatacos[i+1]));
      REQUIRE(u.f[2] == Approx(fdatacos[i+2]));
      REQUIRE(u.f[3] == Approx(fdatacos[i+3]));
    }

    x = _mm_setr_ps(0.0f, (float)M_PI/3.0f, (float)M_PI*0.5f, (float)M_PI);
    sincosf4(x, &s, &c);

    u.vf = s;
    REQUIRE(u.f[0] == Approx(0.0f));
    REQUIRE(u.f[1] == Approx(0.8660254037844386f));
    REQUIRE(u.f[2] == Approx(1.0f));
    REQUIRE(u.f[3] == Approx(0.0f));

    u.vf = c;
    REQUIRE(u.f[0] == Approx(1.0f));
    REQUIRE(u.f[1] == Approx(0.5f));
    REQUIRE(u.f[2] == Approx(0.0f));
    REQUIRE(u.f[3] == Approx(-1.0f));

    x = _mm_setr_ps((float)M_PI/4.0f, (float)M_PI/8.0f, (float)M_PI/16.0f, -(float)M_PI/3.0f);
    sincosf4(x, &s, &c);

    u.vf = s;
    REQUIRE(u.f[0] == Approx(0.707106781186548f));
    REQUIRE(u.f[1] == Approx(0.38268343236509f));
    REQUIRE(u.f[2] == Approx(0.195090322016128f));
    REQUIRE(u.f[3] == Approx(-0.8660254037844386f));

    u.vf = c;
    REQUIRE(u.f[0] == Approx(0.707106781186548f));
    REQUIRE(u.f[1] == Approx(0.923879532511287f));
    REQUIRE(u.f[2] == Approx(0.98078528040323f));
    REQUIRE(u.f[3] == Approx(0.5f));

  }
}
