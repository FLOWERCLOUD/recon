#include "test_helper.h"
#include <catch.hpp>

TEST_CASE("4x4 float matrix (array of struct)", "[aos_Mat4]") {
  Mat4_float u;

  SECTION("matrix initialization") {
    Vec4 v0 = Vec4(1.0f, 2.0f, 3.0f, 4.0f);
    Vec4 v1 = Vec4(-5.0f, -6.0f, -7.0f, -8.0f);
    Vec4 v2 = Vec4(9.0f, 10.0f, 2.5f, -1.5f);
    Vec4 v3 = Vec4(1.0f, 0.0f, 1.0f, 0.0f);
    Mat4 m1 = Mat4(v0, v1, v2, v3);

    u.m = m1;
    REQUIRE(u.f[0][0] == 1.0f);
    REQUIRE(u.f[0][1] == 2.0f);
    REQUIRE(u.f[0][2] == 3.0f);
    REQUIRE(u.f[0][3] == 4.0f);
    REQUIRE(u.f[1][0] == -5.0f);
    REQUIRE(u.f[1][1] == -6.0f);
    REQUIRE(u.f[1][2] == -7.0f);
    REQUIRE(u.f[1][3] == -8.0f);
    REQUIRE(u.f[2][0] == 9.0f);
    REQUIRE(u.f[2][1] == 10.0f);
    REQUIRE(u.f[2][2] == 2.5f);
    REQUIRE(u.f[2][3] == -1.5f);
    REQUIRE(u.f[3][0] == 1.0f);
    REQUIRE(u.f[3][1] == 0.0f);
    REQUIRE(u.f[3][2] == 1.0f);
    REQUIRE(u.f[3][3] == 0.0f);

    m1 = Mat4(Mat3::identity(), Vec3(1.0f, 2.0f, 3.0f));
    u.m = m1;
    REQUIRE(u.f[0][0] == 1.0f);
    REQUIRE(u.f[0][1] == 0.0f);
    REQUIRE(u.f[0][2] == 0.0f);
    REQUIRE(u.f[0][3] == 0.0f);
    REQUIRE(u.f[1][0] == 0.0f);
    REQUIRE(u.f[1][1] == 1.0f);
    REQUIRE(u.f[1][2] == 0.0f);
    REQUIRE(u.f[1][3] == 0.0f);
    REQUIRE(u.f[2][0] == 0.0f);
    REQUIRE(u.f[2][1] == 0.0f);
    REQUIRE(u.f[2][2] == 1.0f);
    REQUIRE(u.f[2][3] == 0.0f);
    REQUIRE(u.f[3][0] == 1.0f);
    REQUIRE(u.f[3][1] == 2.0f);
    REQUIRE(u.f[3][2] == 3.0f);
    REQUIRE(u.f[3][3] == 1.0f);

    m1 = Mat4::zero();
    u.m = m1;
    REQUIRE(u.f[0][0] == 0.0f);
    REQUIRE(u.f[0][1] == 0.0f);
    REQUIRE(u.f[0][2] == 0.0f);
    REQUIRE(u.f[0][3] == 0.0f);
    REQUIRE(u.f[1][0] == 0.0f);
    REQUIRE(u.f[1][1] == 0.0f);
    REQUIRE(u.f[1][2] == 0.0f);
    REQUIRE(u.f[1][3] == 0.0f);
    REQUIRE(u.f[2][0] == 0.0f);
    REQUIRE(u.f[2][1] == 0.0f);
    REQUIRE(u.f[2][2] == 0.0f);
    REQUIRE(u.f[2][3] == 0.0f);
    REQUIRE(u.f[3][0] == 0.0f);
    REQUIRE(u.f[3][1] == 0.0f);
    REQUIRE(u.f[3][2] == 0.0f);
    REQUIRE(u.f[3][3] == 0.0f);

    m1 = Mat4::identity();
    u.m = m1;
    REQUIRE(u.f[0][0] == 1.0f);
    REQUIRE(u.f[0][1] == 0.0f);
    REQUIRE(u.f[0][2] == 0.0f);
    REQUIRE(u.f[0][3] == 0.0f);
    REQUIRE(u.f[1][0] == 0.0f);
    REQUIRE(u.f[1][1] == 1.0f);
    REQUIRE(u.f[1][2] == 0.0f);
    REQUIRE(u.f[1][3] == 0.0f);
    REQUIRE(u.f[2][0] == 0.0f);
    REQUIRE(u.f[2][1] == 0.0f);
    REQUIRE(u.f[2][2] == 1.0f);
    REQUIRE(u.f[2][3] == 0.0f);
    REQUIRE(u.f[3][0] == 0.0f);
    REQUIRE(u.f[3][1] == 0.0f);
    REQUIRE(u.f[3][2] == 0.0f);
    REQUIRE(u.f[3][3] == 1.0f);
  }

  SECTION("matrix load/store") {
    float data[] = {
      1.0f, 2.0f, 3.0f, 4.0f,
      5.0f, 6.0f, 7.0f, 8.0f,
      9.0f, 10.0f, 11.0f, 12.0f,
      13.0f, 14.0f, 15.0f, 16.0f
    };
    Mat4 m1 = Mat4::load(data);
    u.m = m1;
    REQUIRE(u.f[0][0] == 1.0f);
    REQUIRE(u.f[0][1] == 2.0f);
    REQUIRE(u.f[0][2] == 3.0f);
    REQUIRE(u.f[0][3] == 4.0f);
    REQUIRE(u.f[1][0] == 5.0f);
    REQUIRE(u.f[1][1] == 6.0f);
    REQUIRE(u.f[1][2] == 7.0f);
    REQUIRE(u.f[1][3] == 8.0f);
    REQUIRE(u.f[2][0] == 9.0f);
    REQUIRE(u.f[2][1] == 10.0f);
    REQUIRE(u.f[2][2] == 11.0f);
    REQUIRE(u.f[2][3] == 12.0f);
    REQUIRE(u.f[3][0] == 13.0f);
    REQUIRE(u.f[3][1] == 14.0f);
    REQUIRE(u.f[3][2] == 15.0f);
    REQUIRE(u.f[3][3] == 16.0f);

    transpose(-m1).store(data);
    REQUIRE(data[0] == -1.0f);
    REQUIRE(data[1] == -5.0f);
    REQUIRE(data[2] == -9.0f);
    REQUIRE(data[3] == -13.0f);
    REQUIRE(data[4] == -2.0f);
    REQUIRE(data[5] == -6.0f);
    REQUIRE(data[6] == -10.0f);
    REQUIRE(data[7] == -14.0f);
    REQUIRE(data[8] == -3.0f);
    REQUIRE(data[9] == -7.0f);
    REQUIRE(data[10] == -11.0f);
    REQUIRE(data[11] == -15.0f);
    REQUIRE(data[12] == -4.0f);
    REQUIRE(data[13] == -8.0f);
    REQUIRE(data[14] == -12.0f);
    REQUIRE(data[15] == -16.0f);
  }

  const float data1[] = {
    1.0f, 2.0f, 3.0f, 4.0f,
    5.0f, 6.0f, 7.0f, 8.0f,
    9.0f, 10.0f, 11.0f, 12.0f,
    13.0f, 14.0f, 15.0f, 16.0f
  };

  SECTION("matrix negation") {
    Mat4 m1 = Mat4::load(data1);

    u.m = -m1;
    REQUIRE(u.f[0][0] == -1.0f);
    REQUIRE(u.f[0][1] == -2.0f);
    REQUIRE(u.f[0][2] == -3.0f);
    REQUIRE(u.f[0][3] == -4.0f);
    REQUIRE(u.f[1][0] == -5.0f);
    REQUIRE(u.f[1][1] == -6.0f);
    REQUIRE(u.f[1][2] == -7.0f);
    REQUIRE(u.f[1][3] == -8.0f);
    REQUIRE(u.f[2][0] == -9.0f);
    REQUIRE(u.f[2][1] == -10.0f);
    REQUIRE(u.f[2][2] == -11.0f);
    REQUIRE(u.f[2][3] == -12.0f);
    REQUIRE(u.f[3][0] == -13.0f);
    REQUIRE(u.f[3][1] == -14.0f);
    REQUIRE(u.f[3][2] == -15.0f);
    REQUIRE(u.f[3][3] == -16.0f);
  }

  SECTION("matrix addition") {
    Mat4 m1 = Mat4::load(data1);
    Mat4 m2 = Mat4::load(data1);

    u.m = m1 + m2;
    REQUIRE(u.f[0][0] == 2.0f);
    REQUIRE(u.f[0][1] == 4.0f);
    REQUIRE(u.f[0][2] == 6.0f);
    REQUIRE(u.f[0][3] == 8.0f);
    REQUIRE(u.f[1][0] == 10.0f);
    REQUIRE(u.f[1][1] == 12.0f);
    REQUIRE(u.f[1][2] == 14.0f);
    REQUIRE(u.f[1][3] == 16.0f);
    REQUIRE(u.f[2][0] == 18.0f);
    REQUIRE(u.f[2][1] == 20.0f);
    REQUIRE(u.f[2][2] == 22.0f);
    REQUIRE(u.f[2][3] == 24.0f);
    REQUIRE(u.f[3][0] == 26.0f);
    REQUIRE(u.f[3][1] == 28.0f);
    REQUIRE(u.f[3][2] == 30.0f);
    REQUIRE(u.f[3][3] == 32.0f);
  }

  SECTION("matrix subtraction") {
    Mat4 m1 = Mat4::load(data1);
    Mat4 m2 = Mat4::load(data1);

    u.m = m1 - m2;
    REQUIRE(u.f[0][0] == 0.0f);
    REQUIRE(u.f[0][1] == 0.0f);
    REQUIRE(u.f[0][2] == 0.0f);
    REQUIRE(u.f[0][3] == 0.0f);
    REQUIRE(u.f[1][0] == 0.0f);
    REQUIRE(u.f[1][1] == 0.0f);
    REQUIRE(u.f[1][2] == 0.0f);
    REQUIRE(u.f[1][3] == 0.0f);
    REQUIRE(u.f[2][0] == 0.0f);
    REQUIRE(u.f[2][1] == 0.0f);
    REQUIRE(u.f[2][2] == 0.0f);
    REQUIRE(u.f[2][3] == 0.0f);
    REQUIRE(u.f[3][0] == 0.0f);
    REQUIRE(u.f[3][1] == 0.0f);
    REQUIRE(u.f[3][2] == 0.0f);
    REQUIRE(u.f[3][3] == 0.0f);
  }

  SECTION("matrix scaling") {
    Mat4 m1 = Mat4::load(data1);
    Float s = 2.0f;

    u.m = m1 * s;
    REQUIRE(u.f[0][0] == 2.0f);
    REQUIRE(u.f[0][1] == 4.0f);
    REQUIRE(u.f[0][2] == 6.0f);
    REQUIRE(u.f[0][3] == 8.0f);
    REQUIRE(u.f[1][0] == 10.0f);
    REQUIRE(u.f[1][1] == 12.0f);
    REQUIRE(u.f[1][2] == 14.0f);
    REQUIRE(u.f[1][3] == 16.0f);
    REQUIRE(u.f[2][0] == 18.0f);
    REQUIRE(u.f[2][1] == 20.0f);
    REQUIRE(u.f[2][2] == 22.0f);
    REQUIRE(u.f[2][3] == 24.0f);
    REQUIRE(u.f[3][0] == 26.0f);
    REQUIRE(u.f[3][1] == 28.0f);
    REQUIRE(u.f[3][2] == 30.0f);
    REQUIRE(u.f[3][3] == 32.0f);

    u.m = s * m1;
    REQUIRE(u.f[0][0] == 2.0f);
    REQUIRE(u.f[0][1] == 4.0f);
    REQUIRE(u.f[0][2] == 6.0f);
    REQUIRE(u.f[0][3] == 8.0f);
    REQUIRE(u.f[1][0] == 10.0f);
    REQUIRE(u.f[1][1] == 12.0f);
    REQUIRE(u.f[1][2] == 14.0f);
    REQUIRE(u.f[1][3] == 16.0f);
    REQUIRE(u.f[2][0] == 18.0f);
    REQUIRE(u.f[2][1] == 20.0f);
    REQUIRE(u.f[2][2] == 22.0f);
    REQUIRE(u.f[2][3] == 24.0f);
    REQUIRE(u.f[3][0] == 26.0f);
    REQUIRE(u.f[3][1] == 28.0f);
    REQUIRE(u.f[3][2] == 30.0f);
    REQUIRE(u.f[3][3] == 32.0f);

    u.m = m1 / s;
    REQUIRE(u.f[0][0] == 0.5f);
    REQUIRE(u.f[0][1] == 1.0f);
    REQUIRE(u.f[0][2] == 1.5f);
    REQUIRE(u.f[0][3] == 2.0f);
    REQUIRE(u.f[1][0] == 2.5f);
    REQUIRE(u.f[1][1] == 3.0f);
    REQUIRE(u.f[1][2] == 3.5f);
    REQUIRE(u.f[1][3] == 4.0f);
    REQUIRE(u.f[2][0] == 4.5f);
    REQUIRE(u.f[2][1] == 5.0f);
    REQUIRE(u.f[2][2] == 5.5f);
    REQUIRE(u.f[2][3] == 6.0f);
    REQUIRE(u.f[3][0] == 6.5f);
    REQUIRE(u.f[3][1] == 7.0f);
    REQUIRE(u.f[3][2] == 7.5f);
    REQUIRE(u.f[3][3] == 8.0f);
  }

  SECTION("matrix transformation") {
    Mat4 m1 = Mat4::load(data1);
    Vec4 v1 = Vec4(1.0f, 2.0f, -1.0f, 1.0f);

    Vec4_float uv;
    uv.v = m1 * v1;
    REQUIRE(uv.f[0] == 15.0f);
    REQUIRE(uv.f[1] == 18.0f);
    REQUIRE(uv.f[2] == 21.0f);
    REQUIRE(uv.f[3] == 24.0f);
  }

  SECTION("matrix multiplication") {
    Mat4 m1 = Mat4::load(data1);
    Mat4 m2 = Mat4::identity();

    u.m = m1 * m2;
    REQUIRE(u.f[0][0] == 1.0f);
    REQUIRE(u.f[0][1] == 2.0f);
    REQUIRE(u.f[0][2] == 3.0f);
    REQUIRE(u.f[0][3] == 4.0f);
    REQUIRE(u.f[1][0] == 5.0f);
    REQUIRE(u.f[1][1] == 6.0f);
    REQUIRE(u.f[1][2] == 7.0f);
    REQUIRE(u.f[1][3] == 8.0f);
    REQUIRE(u.f[2][0] == 9.0f);
    REQUIRE(u.f[2][1] == 10.0f);
    REQUIRE(u.f[2][2] == 11.0f);
    REQUIRE(u.f[2][3] == 12.0f);
    REQUIRE(u.f[3][0] == 13.0f);
    REQUIRE(u.f[3][1] == 14.0f);
    REQUIRE(u.f[3][2] == 15.0f);
    REQUIRE(u.f[3][3] == 16.0f);
  }

  SECTION("matrix transposition") {
    Mat4 m1 = Mat4::load(data1);
    u.m = transpose(m1);
    REQUIRE(u.f[0][0] == 1.0f);
    REQUIRE(u.f[0][1] == 5.0f);
    REQUIRE(u.f[0][2] == 9.0f);
    REQUIRE(u.f[0][3] == 13.0f);
    REQUIRE(u.f[1][0] == 2.0f);
    REQUIRE(u.f[1][1] == 6.0f);
    REQUIRE(u.f[1][2] == 10.0f);
    REQUIRE(u.f[1][3] == 14.0f);
    REQUIRE(u.f[2][0] == 3.0f);
    REQUIRE(u.f[2][1] == 7.0f);
    REQUIRE(u.f[2][2] == 11.0f);
    REQUIRE(u.f[2][3] == 15.0f);
    REQUIRE(u.f[3][0] == 4.0f);
    REQUIRE(u.f[3][1] == 8.0f);
    REQUIRE(u.f[3][2] == 12.0f);
    REQUIRE(u.f[3][3] == 16.0f);
  }

  SECTION("matrix get columns") {
    Mat4 m1 = Mat4::load(data1);
    Vec4_float uv;

    uv.v = m1.column0();
    REQUIRE(uv.f[0] == 1.0f);
    REQUIRE(uv.f[1] == 2.0f);
    REQUIRE(uv.f[2] == 3.0f);
    REQUIRE(uv.f[3] == 4.0f);
    uv.v = m1.column1();
    REQUIRE(uv.f[0] == 5.0f);
    REQUIRE(uv.f[1] == 6.0f);
    REQUIRE(uv.f[2] == 7.0f);
    REQUIRE(uv.f[3] == 8.0f);
    uv.v = m1.column2();
    REQUIRE(uv.f[0] == 9.0f);
    REQUIRE(uv.f[1] == 10.0f);
    REQUIRE(uv.f[2] == 11.0f);
    REQUIRE(uv.f[3] == 12.0f);
    uv.v = m1.column3();
    REQUIRE(uv.f[0] == 13.0f);
    REQUIRE(uv.f[1] == 14.0f);
    REQUIRE(uv.f[2] == 15.0f);
    REQUIRE(uv.f[3] == 16.0f);
  }

  SECTION("matrix determinant") {
    Mat4 m1 = Mat4::load(data1);
    Float s = determinant(m1);
    REQUIRE((float)s == 0.0f);
  }

  SECTION("matrix inverse") {
    Mat4 m1 = Mat4::identity();
    u.m = inverse(m1);
    REQUIRE(u.f[0][0] == 1.0f);
    REQUIRE(u.f[0][1] == 0.0f);
    REQUIRE(u.f[0][2] == 0.0f);
    REQUIRE(u.f[0][3] == 0.0f);
    REQUIRE(u.f[1][0] == 0.0f);
    REQUIRE(u.f[1][1] == 1.0f);
    REQUIRE(u.f[1][2] == 0.0f);
    REQUIRE(u.f[1][3] == 0.0f);
    REQUIRE(u.f[2][0] == 0.0f);
    REQUIRE(u.f[2][1] == 0.0f);
    REQUIRE(u.f[2][2] == 1.0f);
    REQUIRE(u.f[2][3] == 0.0f);
    REQUIRE(u.f[3][0] == 0.0f);
    REQUIRE(u.f[3][1] == 0.0f);
    REQUIRE(u.f[3][2] == 0.0f);
    REQUIRE(u.f[3][3] == 1.0f);
  }

  SECTION("extract upper 3x3") {
    Mat4 m1 = Mat4::load(data1);
    Mat3_float um;
    um.m = m1.upper3x3();
    REQUIRE(um.f[0][0] == 1.0f);
    REQUIRE(um.f[0][1] == 2.0f);
    REQUIRE(um.f[0][2] == 3.0f);
    REQUIRE(um.f[1][0] == 5.0f);
    REQUIRE(um.f[1][1] == 6.0f);
    REQUIRE(um.f[1][2] == 7.0f);
    REQUIRE(um.f[2][0] == 9.0f);
    REQUIRE(um.f[2][1] == 10.0f);
    REQUIRE(um.f[2][2] == 11.0f);
  }

  SECTION("matrix trace") {
    Mat4 m1 = Mat4::load(data1);
    Float s = trace(m1);
    REQUIRE((float)s == 34.0f);
  }
}
