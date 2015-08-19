#include "test_helper.h"
#include <catch.hpp>

TEST_CASE("float Quaternion (array of struct)", "[aos_Quat]") {
  Quat_float u;

  SECTION("Quat initialization") {
    Quat q1 = Quat(1.0f, 2.0f, 3.0f, 4.0f);
    u.q = q1;
    REQUIRE(u.f[0] == 1.0f);
    REQUIRE(u.f[1] == 2.0f);
    REQUIRE(u.f[2] == 3.0f);
    REQUIRE(u.f[3] == 4.0f);

    q1 = Quat(Vec3(1.0f, 2.0f, 3.0f), 4.0f);
    u.q = q1;
    REQUIRE(u.f[0] == 1.0f);
    REQUIRE(u.f[1] == 2.0f);
    REQUIRE(u.f[2] == 3.0f);
    REQUIRE(u.f[3] == 4.0f);

    q1 = Quat::zero();
    u.q = q1;
    REQUIRE(u.f[0] == 0.0f);
    REQUIRE(u.f[1] == 0.0f);
    REQUIRE(u.f[2] == 0.0f);
    REQUIRE(u.f[3] == 0.0f);

    q1 = Quat::identity();
    u.q = q1;
    REQUIRE(u.f[0] == 0.0f);
    REQUIRE(u.f[1] == 0.0f);
    REQUIRE(u.f[2] == 0.0f);
    REQUIRE(u.f[3] == 1.0f);
  }

  SECTION("Quat initialization (2)") {
    Quat q1;
    Float angle;
    Vec3 v1, v2;

    // from angle and axis
    angle = M_PI / 2.0f;
    q1 = Quat::rotation(angle, Vec3(1.0f, 1.0f, 1.0f));
    u.q = q1;
    REQUIRE(u.f[0] == Approx(0.7071067811865475f));
    REQUIRE(u.f[1] == Approx(0.7071067811865475f));
    REQUIRE(u.f[2] == Approx(0.7071067811865475f));
    REQUIRE(u.f[3] == Approx(0.7071067811865475f));

    angle = 0.0f;
    q1 = Quat::rotation(angle, Vec3(1.0f, 2.0f, 3.0f));
    u.q = q1;
    REQUIRE(u.f[0] == Approx(0.0f));
    REQUIRE(u.f[1] == Approx(0.0f));
    REQUIRE(u.f[2] == Approx(0.0f));
    REQUIRE(u.f[3] == Approx(1.0f));

    // from two vectors
    v1 = Vec3(1.0f, 0.0f, 0.0f);
    v2 = Vec3(0.0f, 1.0f, 0.0f);
    q1 = Quat::rotation(v1, v2);
    u.q = q1;
    REQUIRE(u.f[0] == Approx(0.0f));
    REQUIRE(u.f[1] == Approx(0.0f));
    REQUIRE(u.f[2] == Approx(0.7071067811865475f));
    REQUIRE(u.f[3] == Approx(0.7071067811865475f));
  }

  SECTION("Quat load/store") {
    float data[] = { 5.0f, 10.0f, 70.0f, -1.0f };
    Quat q1 = Quat::load(data);
    u.q = q1;
    REQUIRE(u.f[0] == 5.0f);
    REQUIRE(u.f[1] == 10.0f);
    REQUIRE(u.f[2] == 70.0f);
    REQUIRE(u.f[3] == -1.0f);

    for (int i = 0; i < 4; ++i)
      data[i] = 0.0f;

    (-q1).store(data);
    REQUIRE(data[0] == -5.0f);
    REQUIRE(data[1] == -10.0f);
    REQUIRE(data[2] == -70.0f);
    REQUIRE(data[3] == 1.0f);
  }

  SECTION("extract Float from Quat") {
    Quat q1 = Quat(1.0f, 2.0f, 3.0f, 4.0f);

    REQUIRE((float)(q1.x()) == 1.0f);
    REQUIRE((float)(q1.y()) == 2.0f);
    REQUIRE((float)(q1.z()) == 3.0f);
    REQUIRE((float)(q1.w()) == 4.0f);
  }

  SECTION("extract vector from Quat") {
    Quat q1 = Quat(1.0f, 2.0f, 3.0f, 4.0f);

    Vec3_float vf;
    vf.v = q1.v();
    REQUIRE(vf.f[0] == 1.0f);
    REQUIRE(vf.f[1] == 2.0f);
    REQUIRE(vf.f[2] == 3.0f);
  }

  SECTION("Quat negation") {
    Quat q1 = Quat(1.0f, 2.0f, 3.0f, 4.0f);

    u.q = -q1;
    REQUIRE(u.f[0] == -1.0f);
    REQUIRE(u.f[1] == -2.0f);
    REQUIRE(u.f[2] == -3.0f);
    REQUIRE(u.f[3] == -4.0f);
  }

  SECTION("Quat conj") {
    Quat q1 = Quat(1.0f, 2.0f, 3.0f, 4.0f);

    u.q = ~q1;
    REQUIRE(u.f[0] == -1.0f);
    REQUIRE(u.f[1] == -2.0f);
    REQUIRE(u.f[2] == -3.0f);
    REQUIRE(u.f[3] == 4.0f);
  }

  SECTION("Quat addition") {
    Quat q1 = Quat(1.0f, 2.0f, 3.0f, 4.0f);
    Quat q2 = Quat(10.0f, 1.5f, -200.0f, -4.0f);

    u.q = q1 + q2;
    REQUIRE(u.f[0] == 11.0f);
    REQUIRE(u.f[1] == 3.5f);
    REQUIRE(u.f[2] == -197.0f);
    REQUIRE(u.f[3] == 0.0f);
  }

  SECTION("Quat subtraction") {
    Quat q1 = Quat(1.0f, 2.0f, 3.0f, 4.0f);
    Quat q2 = Quat(10.0f, 1.5f, -200.0f, -4.0f);

    u.q = q1 - q2;
    REQUIRE(u.f[0] == -9.0f);
    REQUIRE(u.f[1] == 0.5f);
    REQUIRE(u.f[2] == 203.0f);
    REQUIRE(u.f[3] == 8.0f);
  }

  SECTION("Quat scaling") {
    Quat q1 = Quat(1.0f, 2.0f, 3.0f, 4.0f);
    Float s = 2.0f;

    u.q = q1 * s;
    REQUIRE(u.f[0] == 2.0f);
    REQUIRE(u.f[1] == 4.0f);
    REQUIRE(u.f[2] == 6.0f);
    REQUIRE(u.f[3] == 8.0f);

    u.q = s * q1;
    REQUIRE(u.f[0] == 2.0f);
    REQUIRE(u.f[1] == 4.0f);
    REQUIRE(u.f[2] == 6.0f);
    REQUIRE(u.f[3] == 8.0f);

    u.q = q1 / s;
    REQUIRE(u.f[0] == 0.5f);
    REQUIRE(u.f[1] == 1.0f);
    REQUIRE(u.f[2] == 1.5f);
    REQUIRE(u.f[3] == 2.0f);
  }

  SECTION("Quat multiplication") {
    Quat q1 = Quat::rotation(Float(M_PI*0.25), Vec3(0,0,1));
    Quat q2 = Quat::rotation(Float(M_PI*0.5), Vec3(0,0,1));
    Quat q3 = Quat::rotation(Float(M_PI), Vec3(0,0,1));
    Quat delta = q3 - q2 * q1 * q1;

    u.q = delta;
    REQUIRE(u.f[0] == Approx(0.0f));
    REQUIRE(u.f[1] == Approx(0.0f));
    REQUIRE(u.f[2] == Approx(0.0f));
    REQUIRE(u.f[3] == Approx(0.0f));
  }

  SECTION("Quat rotates vector") {
    Quat q1 = Quat::rotation(Float(M_PI*0.5), Vec3(0,0,1));

    Vec3_float v;
    v.v = rotate(q1,Vec3(1,0,0));
    REQUIRE(v.f[0] == Approx(0.0f));
    REQUIRE(v.f[1] == Approx(1.0f));
    REQUIRE(v.f[2] == Approx(0.0f));
  }

  SECTION("Quat normalization") {
    Quat q1 = Quat(1.0f, 1.0f, 1.0f, 1.0f);

    u.q = normalize(q1);
    REQUIRE(u.f[0] == Approx(0.5f));
    REQUIRE(u.f[1] == Approx(0.5f));
    REQUIRE(u.f[2] == Approx(0.5f));
    REQUIRE(u.f[3] == Approx(0.5f));
  }

  SECTION("Quat lerp") {
    Quat q1 = Quat(1.0f, 2.0f, 3.0f, 4.0f);
    Quat q2 = Quat(5.0f, -2.0f, -5.0f, 1.0f);
    Float t = 0.5f;

    u.q = lerp(t, q1, q2);
    REQUIRE(u.f[0] == 3.0f);
    REQUIRE(u.f[1] == 0.0f);
    REQUIRE(u.f[2] == -1.0f);
    REQUIRE(u.f[3] == 2.5f);
  }

  SECTION("Quat slerp") {
    // TODO
  }

  SECTION("Quat squad") {
    // TODO
  }

  SECTION("Quat to matrix") {
    Quat q1 = Quat::identity();

    Mat3_float um;
    um.m = (Mat3)q1;
    REQUIRE(um.f[0][0] == 1.0f);
    REQUIRE(um.f[0][1] == 0.0f);
    REQUIRE(um.f[0][2] == 0.0f);
    REQUIRE(um.f[1][0] == 0.0f);
    REQUIRE(um.f[1][1] == 1.0f);
    REQUIRE(um.f[1][2] == 0.0f);
    REQUIRE(um.f[2][0] == 0.0f);
    REQUIRE(um.f[2][1] == 0.0f);
    REQUIRE(um.f[2][2] == 1.0f);

    q1 = Quat::rotation(Float((float)M_PI/2.0f), Vec3(0.0f, 0.0f, 1.0f));
    um.m = (Mat3)q1;
    REQUIRE(um.f[0][0] == Approx(0.0f));
    REQUIRE(um.f[0][1] == Approx(1.0f));
    REQUIRE(um.f[0][2] == Approx(0.0f));
    REQUIRE(um.f[1][0] == Approx(-1.0f));
    REQUIRE(um.f[1][1] == Approx(0.0f));
    REQUIRE(um.f[1][2] == Approx(0.0f));
    REQUIRE(um.f[2][0] == Approx(0.0f));
    REQUIRE(um.f[2][1] == Approx(0.0f));
    REQUIRE(um.f[2][2] == Approx(1.0f));
  }

  SECTION("matrix to Quat") {
    Quat q1 = Quat::rotation(Float((float)M_PI/3.0f), Vec3(0.0f, 0.0f, 1.0f));
    Quat q2 = Quat::rotation(((Mat3)q1));
    Vec3 v1 = Vec3(1.0f, 2.0f, 3.0f);
    Vec3_float uv1, uv2;
    uv1.v = rotate(q1, v1);
    uv2.v = rotate(q2, v1);
    REQUIRE((float)uv2.f[0] == Approx((float)uv1.f[0]));
    REQUIRE((float)uv2.f[1] == Approx((float)uv1.f[1]));
    REQUIRE((float)uv2.f[2] == Approx((float)uv1.f[2]));
  }
}
