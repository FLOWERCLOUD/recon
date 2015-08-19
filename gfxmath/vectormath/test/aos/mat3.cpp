#include "test_helper.h"
#include <catch.hpp>

TEST_CASE("3x3 float matrix (array of struct)", "[aos_Mat3]") {
  Mat3_float u;

  SECTION("matrix initialization") {
    Vec3 v0 = Vec3(1.0f, 2.0f, 3.0f);
    Vec3 v1 = Vec3(-4.0f, -5.0f, -6.0f);
    Vec3 v2 = Vec3(9.0f, 10.0f, 2.5f);
    Mat3 m1 = Mat3(v0, v1, v2);

    u.m = m1;
    REQUIRE(u.f[0][0] == 1.0f);
    REQUIRE(u.f[0][1] == 2.0f);
    REQUIRE(u.f[0][2] == 3.0f);
    REQUIRE(u.f[1][0] == -4.0f);
    REQUIRE(u.f[1][1] == -5.0f);
    REQUIRE(u.f[1][2] == -6.0f);
    REQUIRE(u.f[2][0] == 9.0f);
    REQUIRE(u.f[2][1] == 10.0f);
    REQUIRE(u.f[2][2] == 2.5f);

    m1 = Mat3::zero();
    u.m = m1;
    REQUIRE(u.f[0][0] == 0.0f);
    REQUIRE(u.f[0][1] == 0.0f);
    REQUIRE(u.f[0][2] == 0.0f);
    REQUIRE(u.f[1][0] == 0.0f);
    REQUIRE(u.f[1][1] == 0.0f);
    REQUIRE(u.f[1][2] == 0.0f);
    REQUIRE(u.f[2][0] == 0.0f);
    REQUIRE(u.f[2][1] == 0.0f);
    REQUIRE(u.f[2][2] == 0.0f);

    m1 = Mat3::identity();
    u.m = m1;
    REQUIRE(u.f[0][0] == 1.0f);
    REQUIRE(u.f[0][1] == 0.0f);
    REQUIRE(u.f[0][2] == 0.0f);
    REQUIRE(u.f[1][0] == 0.0f);
    REQUIRE(u.f[1][1] == 1.0f);
    REQUIRE(u.f[1][2] == 0.0f);
    REQUIRE(u.f[2][0] == 0.0f);
    REQUIRE(u.f[2][1] == 0.0f);
    REQUIRE(u.f[2][2] == 1.0f);
  }

  SECTION("make scaling matrix") {
    Mat3 m1 = Mat3::scaling(2.0f, 5.0f, -1.0f);
    Vec3 v1 = Vec3(7.0f, 2.0f, 100.0f);

    Vec3_float uv;
    uv.v = m1 * v1;
    REQUIRE(uv.f[0] == 14.0f);
    REQUIRE(uv.f[1] == 10.0f);
    REQUIRE(uv.f[2] == -100.0f);
  }

  SECTION("make rotation matrix") {
    Mat3 m1 = Mat3::rotation((float)M_PI / 2.0f, Vec3(0.0f, 0.0f, 1.0f));
    Vec3 v1 = Vec3(1.0f, 2.0f, 3.0f);
    Vec3_float uv;
    uv.v = m1 * v1;
    REQUIRE(uv.f[0] == Approx(-2.0f));
    REQUIRE(uv.f[1] == Approx(1.0f));
    REQUIRE(uv.f[2] == Approx(3.0f));
  }

  SECTION("matrix load/store") {
    float data[] = {
      1.0f, 2.0f, 3.0f,
      4.0f, 5.0f, 6.0f,
      7.0f, 8.0f, 9.0f
    };
    Mat3 m1 = Mat3::load(data);
    u.m = m1;
    REQUIRE(u.f[0][0] == 1.0f);
    REQUIRE(u.f[0][1] == 2.0f);
    REQUIRE(u.f[0][2] == 3.0f);
    REQUIRE(u.f[1][0] == 4.0f);
    REQUIRE(u.f[1][1] == 5.0f);
    REQUIRE(u.f[1][2] == 6.0f);
    REQUIRE(u.f[2][0] == 7.0f);
    REQUIRE(u.f[2][1] == 8.0f);
    REQUIRE(u.f[2][2] == 9.0f);

    transpose(-m1).store(data);
    REQUIRE(data[0] == -1.0f);
    REQUIRE(data[1] == -4.0f);
    REQUIRE(data[2] == -7.0f);
    REQUIRE(data[3] == -2.0f);
    REQUIRE(data[4] == -5.0f);
    REQUIRE(data[5] == -8.0f);
    REQUIRE(data[6] == -3.0f);
    REQUIRE(data[7] == -6.0f);
    REQUIRE(data[8] == -9.0f);
  }

  const float data1[] = {
    1.0f, 2.0f, 3.0f,
    4.0f, 5.0f, 6.0f,
    7.0f, 8.0f, 9.0f
  };

  SECTION("matrix negation") {
    Mat3 m1 = Mat3::load(data1);

    u.m = -m1;
    REQUIRE(u.f[0][0] == -1.0f);
    REQUIRE(u.f[0][1] == -2.0f);
    REQUIRE(u.f[0][2] == -3.0f);
    REQUIRE(u.f[1][0] == -4.0f);
    REQUIRE(u.f[1][1] == -5.0f);
    REQUIRE(u.f[1][2] == -6.0f);
    REQUIRE(u.f[2][0] == -7.0f);
    REQUIRE(u.f[2][1] == -8.0f);
    REQUIRE(u.f[2][2] == -9.0f);
  }

  SECTION("matrix addition") {
    Mat3 m1 = Mat3::load(data1);
    Mat3 m2 = Mat3::load(data1);

    u.m = m1 + m2;
    REQUIRE(u.f[0][0] == 2.0f);
    REQUIRE(u.f[0][1] == 4.0f);
    REQUIRE(u.f[0][2] == 6.0f);
    REQUIRE(u.f[1][0] == 8.0f);
    REQUIRE(u.f[1][1] == 10.0f);
    REQUIRE(u.f[1][2] == 12.0f);
    REQUIRE(u.f[2][0] == 14.0f);
    REQUIRE(u.f[2][1] == 16.0f);
    REQUIRE(u.f[2][2] == 18.0f);
  }

  SECTION("matrix subtraction") {
    Mat3 m1 = Mat3::load(data1);
    Mat3 m2 = Mat3::load(data1);

    u.m = m1 - m2;
    REQUIRE(u.f[0][0] == 0.0f);
    REQUIRE(u.f[0][1] == 0.0f);
    REQUIRE(u.f[0][2] == 0.0f);
    REQUIRE(u.f[1][0] == 0.0f);
    REQUIRE(u.f[1][1] == 0.0f);
    REQUIRE(u.f[1][2] == 0.0f);
    REQUIRE(u.f[2][0] == 0.0f);
    REQUIRE(u.f[2][1] == 0.0f);
    REQUIRE(u.f[2][2] == 0.0f);
  }

  SECTION("matrix scaling") {
    Mat3 m1 = Mat3::load(data1);
    Float s = 2.0f;

    u.m = m1 * s;
    REQUIRE(u.f[0][0] == 2.0f);
    REQUIRE(u.f[0][1] == 4.0f);
    REQUIRE(u.f[0][2] == 6.0f);
    REQUIRE(u.f[1][0] == 8.0f);
    REQUIRE(u.f[1][1] == 10.0f);
    REQUIRE(u.f[1][2] == 12.0f);
    REQUIRE(u.f[2][0] == 14.0f);
    REQUIRE(u.f[2][1] == 16.0f);
    REQUIRE(u.f[2][2] == 18.0f);

    u.m = s * m1;
    REQUIRE(u.f[0][0] == 2.0f);
    REQUIRE(u.f[0][1] == 4.0f);
    REQUIRE(u.f[0][2] == 6.0f);
    REQUIRE(u.f[1][0] == 8.0f);
    REQUIRE(u.f[1][1] == 10.0f);
    REQUIRE(u.f[1][2] == 12.0f);
    REQUIRE(u.f[2][0] == 14.0f);
    REQUIRE(u.f[2][1] == 16.0f);
    REQUIRE(u.f[2][2] == 18.0f);

    u.m = m1 / s;
    REQUIRE(u.f[0][0] == 0.5f);
    REQUIRE(u.f[0][1] == 1.0f);
    REQUIRE(u.f[0][2] == 1.5f);
    REQUIRE(u.f[1][0] == 2.0f);
    REQUIRE(u.f[1][1] == 2.5f);
    REQUIRE(u.f[1][2] == 3.0f);
    REQUIRE(u.f[2][0] == 3.5f);
    REQUIRE(u.f[2][1] == 4.0f);
    REQUIRE(u.f[2][2] == 4.5f);
  }

  SECTION("matrix transformation") {
    Mat3 m1 = Mat3::load(data1);
    Vec3 v1 = Vec3(1.0f, 2.0f, -1.0f);

    Vec3_float uv;
    uv.v = m1 * v1;
    REQUIRE(uv.f[0] == 2.0f);
    REQUIRE(uv.f[1] == 4.0f);
    REQUIRE(uv.f[2] == 6.0f);
  }

  SECTION("matrix multiplication") {
    Mat3 m1 = Mat3::load(data1);
    Mat3 m2 = Mat3::identity();

    u.m = m1 * m2;
    REQUIRE(u.f[0][0] == 1.0f);
    REQUIRE(u.f[0][1] == 2.0f);
    REQUIRE(u.f[0][2] == 3.0f);
    REQUIRE(u.f[1][0] == 4.0f);
    REQUIRE(u.f[1][1] == 5.0f);
    REQUIRE(u.f[1][2] == 6.0f);
    REQUIRE(u.f[2][0] == 7.0f);
    REQUIRE(u.f[2][1] == 8.0f);
    REQUIRE(u.f[2][2] == 9.0f);
  }

  SECTION("matrix transposition") {
    Mat3 m1 = Mat3::load(data1);
    u.m = transpose(m1);
    REQUIRE(u.f[0][0] == 1.0f);
    REQUIRE(u.f[0][1] == 4.0f);
    REQUIRE(u.f[0][2] == 7.0f);
    REQUIRE(u.f[1][0] == 2.0f);
    REQUIRE(u.f[1][1] == 5.0f);
    REQUIRE(u.f[1][2] == 8.0f);
    REQUIRE(u.f[2][0] == 3.0f);
    REQUIRE(u.f[2][1] == 6.0f);
    REQUIRE(u.f[2][2] == 9.0f);
  }

  SECTION("matrix get columns") {
    Mat3 m1 = Mat3::load(data1);
    Vec3_float uv;

    uv.v = m1.column0();
    REQUIRE(uv.f[0] == 1.0f);
    REQUIRE(uv.f[1] == 2.0f);
    REQUIRE(uv.f[2] == 3.0f);
    uv.v = m1.column1();
    REQUIRE(uv.f[0] == 4.0f);
    REQUIRE(uv.f[1] == 5.0f);
    REQUIRE(uv.f[2] == 6.0f);
    uv.v = m1.column2();
    REQUIRE(uv.f[0] == 7.0f);
    REQUIRE(uv.f[1] == 8.0f);
    REQUIRE(uv.f[2] == 9.0f);
  }

  SECTION("matrix determinant") {
    Mat3 m1 = Mat3::load(data1);
    Float s = determinant(m1);
    REQUIRE((float)s == 0.0f);
  }

  SECTION("matrix inverse") {
    Mat3 m1 = Mat3::identity();
    u.m = inverse(m1);
    REQUIRE(u.f[0][0] == 1.0f);
    REQUIRE(u.f[0][1] == 0.0f);
    REQUIRE(u.f[0][2] == 0.0f);
    REQUIRE(u.f[1][0] == 0.0f);
    REQUIRE(u.f[1][1] == 1.0f);
    REQUIRE(u.f[1][2] == 0.0f);
    REQUIRE(u.f[2][0] == 0.0f);
    REQUIRE(u.f[2][1] == 0.0f);
    REQUIRE(u.f[2][2] == 1.0f);
  }

  SECTION("matrix trace") {
    Mat3 m1 = Mat3::load(data1);
    Float s = trace(m1);
    REQUIRE((float)s == 15.0f);
  }
}
