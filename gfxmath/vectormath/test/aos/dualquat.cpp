#include "test_helper.h"
#include <catch.hpp>

TEST_CASE("float dual Quaternion (array of struct)", "[aos_DQuat]") {
  Quat_float u;

  SECTION("DQuat initialization") {
    DQuat dq1 = DQuat{Quat(1.0f, 2.0f, 3.0f, 4.0f),
                            Quat(10.0f, 20.0f, 30.0f, 40.0f)};
    u.q = dq1.real();
    REQUIRE(u.f[0] == 1.0f);
    REQUIRE(u.f[1] == 2.0f);
    REQUIRE(u.f[2] == 3.0f);
    REQUIRE(u.f[3] == 4.0f);
    u.q = dq1.dual();
    REQUIRE(u.f[0] == 10.0f);
    REQUIRE(u.f[1] == 20.0f);
    REQUIRE(u.f[2] == 30.0f);
    REQUIRE(u.f[3] == 40.0f);

    dq1 = DQuat::identity();
    u.q = dq1.real();
    REQUIRE(u.f[0] == 0.0f);
    REQUIRE(u.f[1] == 0.0f);
    REQUIRE(u.f[2] == 0.0f);
    REQUIRE(u.f[3] == 1.0f);
    u.q = dq1.dual();
    REQUIRE(u.f[0] == 0.0f);
    REQUIRE(u.f[1] == 0.0f);
    REQUIRE(u.f[2] == 0.0f);
    REQUIRE(u.f[3] == 0.0f);
  }

  SECTION("DQuat transform + Mat4 conversion") {
    Quat rot = Quat::rotation((float)(M_PI / 4.0), Vec3(0.0f, 0.0f, 1.0f));
    Vec3 trans = Vec3(25.0f, -10.0f, 100.0f);
    DQuat dq1 = DQuat::transform(rot, trans);
    Mat4 m1 = (Mat4)dq1;
    Mat4 m2 = Mat4((Mat3)rot, trans);
    Mat4_float um1, um2;
    um1.m = m1;
    um2.m = m2;
    REQUIRE(um1.f[0][0] == Approx(um2.f[0][0]));
    REQUIRE(um1.f[0][1] == Approx(um2.f[0][1]));
    REQUIRE(um1.f[0][2] == Approx(um2.f[0][2]));
    REQUIRE(um1.f[0][3] == Approx(um2.f[0][3]));
    REQUIRE(um1.f[1][0] == Approx(um2.f[1][0]));
    REQUIRE(um1.f[1][1] == Approx(um2.f[1][1]));
    REQUIRE(um1.f[1][2] == Approx(um2.f[1][2]));
    REQUIRE(um1.f[1][3] == Approx(um2.f[1][3]));
    REQUIRE(um1.f[2][0] == Approx(um2.f[2][0]));
    REQUIRE(um1.f[2][1] == Approx(um2.f[2][1]));
    REQUIRE(um1.f[2][2] == Approx(um2.f[2][2]));
    REQUIRE(um1.f[2][3] == Approx(um2.f[2][3]));
    REQUIRE(um1.f[3][0] == Approx(um2.f[3][0]));
    REQUIRE(um1.f[3][1] == Approx(um2.f[3][1]));
    REQUIRE(um1.f[3][2] == Approx(um2.f[3][2]));
    REQUIRE(um1.f[3][3] == Approx(um2.f[3][3]));
  }

  SECTION("DQuat negation") {
    DQuat dq1 = DQuat{Quat(1.0f, 2.0f, 3.0f, 4.0f),
                            Quat(10.0f, 20.0f, 30.0f, 40.0f)};
    DQuat dq2 = -dq1;
    u.q = dq2.real();
    REQUIRE(u.f[0] == -1.0f);
    REQUIRE(u.f[1] == -2.0f);
    REQUIRE(u.f[2] == -3.0f);
    REQUIRE(u.f[3] == -4.0f);
    u.q = dq2.dual();
    REQUIRE(u.f[0] == -10.0f);
    REQUIRE(u.f[1] == -20.0f);
    REQUIRE(u.f[2] == -30.0f);
    REQUIRE(u.f[3] == -40.0f);
  }

  SECTION("DQuat addition") {
    DQuat dq1 = DQuat{Quat(1.0f, 2.0f, 3.0f, 4.0f),
                            Quat(10.0f, 20.0f, 30.0f, 40.0f)};
    DQuat dq2 = DQuat{Quat(14.0f, -2.0f, -10.0f, 4.0f),
                            Quat(0.0f, 10.0f, -32.0f, 2.0f)};
    DQuat dq3 = dq1 + dq2;
    u.q = dq3.real();
    REQUIRE(u.f[0] == 15.0f);
    REQUIRE(u.f[1] == 0.0f);
    REQUIRE(u.f[2] == -7.0f);
    REQUIRE(u.f[3] == 8.0f);
    u.q = dq3.dual();
    REQUIRE(u.f[0] == 10.0f);
    REQUIRE(u.f[1] == 30.0f);
    REQUIRE(u.f[2] == -2.0f);
    REQUIRE(u.f[3] == 42.0f);
  }

  SECTION("DQuat subtraction") {
    DQuat dq1 = DQuat{Quat(1.0f, 2.0f, 3.0f, 4.0f),
                            Quat(10.0f, 20.0f, 30.0f, 40.0f)};
    DQuat dq2 = DQuat{Quat(14.0f, -2.0f, -10.0f, 4.0f),
                            Quat(0.0f, 10.0f, -32.0f, 2.0f)};
    DQuat dq3 = dq1 - dq2;
    u.q = dq3.real();
    REQUIRE(u.f[0] == -13.0f);
    REQUIRE(u.f[1] == 4.0f);
    REQUIRE(u.f[2] == 13.0f);
    REQUIRE(u.f[3] == 0.0f);
    u.q = dq3.dual();
    REQUIRE(u.f[0] == 10.0f);
    REQUIRE(u.f[1] == 10.0f);
    REQUIRE(u.f[2] == 62.0f);
    REQUIRE(u.f[3] == 38.0f);
  }

  SECTION("DQuat scaling") {
    DQuat dq1 = DQuat{Quat(1.0f, 2.0f, 3.0f, 4.0f),
                            Quat(10.0f, 20.0f, 30.0f, 40.0f)};
    DQuat dq2 = dq1 * 2.0f;
    u.q = dq2.real();
    REQUIRE(u.f[0] == 2.0f);
    REQUIRE(u.f[1] == 4.0f);
    REQUIRE(u.f[2] == 6.0f);
    REQUIRE(u.f[3] == 8.0f);
    u.q = dq2.dual();
    REQUIRE(u.f[0] == 20.0f);
    REQUIRE(u.f[1] == 40.0f);
    REQUIRE(u.f[2] == 60.0f);
    REQUIRE(u.f[3] == 80.0f);

    dq2 = 2.0f * dq1;
    u.q = dq2.real();
    REQUIRE(u.f[0] == 2.0f);
    REQUIRE(u.f[1] == 4.0f);
    REQUIRE(u.f[2] == 6.0f);
    REQUIRE(u.f[3] == 8.0f);
    u.q = dq2.dual();
    REQUIRE(u.f[0] == 20.0f);
    REQUIRE(u.f[1] == 40.0f);
    REQUIRE(u.f[2] == 60.0f);
    REQUIRE(u.f[3] == 80.0f);

    dq2 = dq1 / 2.0f;
    u.q = dq2.real();
    REQUIRE(u.f[0] == 0.5f);
    REQUIRE(u.f[1] == 1.0f);
    REQUIRE(u.f[2] == 1.5f);
    REQUIRE(u.f[3] == 2.0f);
    u.q = dq2.dual();
    REQUIRE(u.f[0] == 5.0f);
    REQUIRE(u.f[1] == 10.0f);
    REQUIRE(u.f[2] == 15.0f);
    REQUIRE(u.f[3] == 20.0f);
  }

  SECTION("DQuat multiplication") {
    Quat rot1 = Quat::rotation((float)(M_PI * 0.25), Vec3(0.0f, 0.0f, 1.0f));
    Quat rot2 = Quat::rotation((float)(M_PI * 0.25), Vec3(0.0f, 1.0f, 0.0));
    Vec3 t1 = Vec3(1.0f, 2.0f, 3.0f);
    Vec3 t2 = Vec3(-100.0f, 3.2f, -7.0f);
    DQuat dq1 = DQuat::transform(rot1, t1);
    DQuat dq2 = DQuat::transform(rot2, t2);
    DQuat dq3 = dq2 * dq1;
    Mat4 m1 = (Mat4)dq3;
    Mat4 m2 = Mat4((Mat3)(rot2 * rot1), rotate(rot2, t1) + t2);
    Mat4_float um1, um2;
    um1.m = m1;
    um2.m = m2;
    REQUIRE(um1.f[0][0] == Approx(um2.f[0][0]));
    REQUIRE(um1.f[0][1] == Approx(um2.f[0][1]));
    REQUIRE(um1.f[0][2] == Approx(um2.f[0][2]));
    REQUIRE(um1.f[0][3] == Approx(um2.f[0][3]));
    REQUIRE(um1.f[1][0] == Approx(um2.f[1][0]));
    REQUIRE(um1.f[1][1] == Approx(um2.f[1][1]));
    REQUIRE(um1.f[1][2] == Approx(um2.f[1][2]));
    REQUIRE(um1.f[1][3] == Approx(um2.f[1][3]));
    REQUIRE(um1.f[2][0] == Approx(um2.f[2][0]));
    REQUIRE(um1.f[2][1] == Approx(um2.f[2][1]));
    REQUIRE(um1.f[2][2] == Approx(um2.f[2][2]));
    REQUIRE(um1.f[2][3] == Approx(um2.f[2][3]));
    REQUIRE(um1.f[3][0] == Approx(um2.f[3][0]));
    REQUIRE(um1.f[3][1] == Approx(um2.f[3][1]));
    REQUIRE(um1.f[3][2] == Approx(um2.f[3][2]));
    REQUIRE(um1.f[3][3] == Approx(um2.f[3][3]));
  }

  SECTION("DQuat transform") {
    Quat rot = Quat::rotation((float)(M_PI * 0.25), Vec3(0.0f, 0.0f, 1.0f));
    Vec3 trans = Vec3(1.0f, -2.0f, 3.0f);
    Vec3 pt = Vec3(10.0f, 1.0f, 2.0f);
    DQuat dq1 = DQuat::transform(rot, trans);
    Vec3 pt1 = transform(dq1, pt);
    Vec3 pt2 = rotate(rot, pt) + trans;
    Vec3_float uv1, uv2;
    uv1.v = pt1;
    uv2.v = pt2;
    REQUIRE(uv1.f[0] == Approx(uv2.f[0]));
    REQUIRE(uv1.f[1] == Approx(uv2.f[1]));
    REQUIRE(uv1.f[2] == Approx(uv2.f[2]));
  }

  SECTION("DQuat lerp") {
    DQuat dq1 = DQuat{Quat(1.0f, 2.0f, 3.0f, 4.0f),
                            Quat(10.0f, 20.0f, 30.0f, 40.0f)};
    DQuat dq2 = DQuat{Quat(14.0f, -2.0f, -10.0f, 4.0f),
                            Quat(0.0f, 10.0f, -32.0f, 2.0f)};
    DQuat dq3 = lerp(Float(0.5f), dq1, dq2);
    u.q = dq3.real();
    REQUIRE(u.f[0] == 7.5f);
    REQUIRE(u.f[1] == 0.0f);
    REQUIRE(u.f[2] == -3.5f);
    REQUIRE(u.f[3] == 4.0f);
    u.q = dq3.dual();
    REQUIRE(u.f[0] == 5.0f);
    REQUIRE(u.f[1] == 15.0f);
    REQUIRE(u.f[2] == -1.0f);
    REQUIRE(u.f[3] == 21.0f);
  }
}
