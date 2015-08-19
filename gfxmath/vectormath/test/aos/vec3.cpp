#include "test_helper.h"
#include <catch.hpp>

TEST_CASE("3-component float vector (array of struct)", "[aos_Vec3]") {
  Vec3_float u;

  SECTION("vector initialization") {
    Vec3 v1 = Vec3(1.0f, 2.0f, 3.0f);
    u.v = v1;
    REQUIRE(u.f[0] == 1.0f);
    REQUIRE(u.f[1] == 2.0f);
    REQUIRE(u.f[2] == 3.0f);

    v1 = Vec3::zero();
    u.v = v1;
    REQUIRE(u.f[0] == 0.0f);
    REQUIRE(u.f[1] == 0.0f);
    REQUIRE(u.f[2] == 0.0f);
  }

  SECTION("vector load/store") {
    float data[] = { 5.0f, 10.0f, 70.0f };
    Vec3 v1 = Vec3::load(data);

    u.v = v1;
    REQUIRE(u.f[0] == 5.0f);
    REQUIRE(u.f[1] == 10.0f);
    REQUIRE(u.f[2] == 70.0f);

    for (int i = 0; i < 3; ++i)
      data[i] = 0.0f;

    (-v1).store(data);
    REQUIRE(data[0] == -5.0f);
    REQUIRE(data[1] == -10.0f);
    REQUIRE(data[2] == -70.0f);
  }

  SECTION("vector negation") {
    Vec3 v1 = Vec3(1.0f, 2.0f, 3.0f);

    u.v = -v1;
    REQUIRE(u.f[0] == -1.0f);
    REQUIRE(u.f[1] == -2.0f);
    REQUIRE(u.f[2] == -3.0f);
  }

  SECTION("vector addition") {
    Vec3 v1 = Vec3(1.0f, 2.0f, 3.0f);
    Vec3 v2 = Vec3(10.0f, 1.5f, -200.0f);

    u.v = v1 + v2;
    REQUIRE(u.f[0] == 11.0f);
    REQUIRE(u.f[1] == 3.5f);
    REQUIRE(u.f[2] == -197.0f);
  }

  SECTION("vector subtraction") {
    Vec3 v1 = Vec3(1.0f, 2.0f, 3.0f);
    Vec3 v2 = Vec3(10.0f, 1.5f, -200.0f);

    u.v = v1 - v2;
    REQUIRE(u.f[0] == -9.0f);
    REQUIRE(u.f[1] == 0.5f);
    REQUIRE(u.f[2] == 203.0f);
  }

  SECTION("vector scaling") {
    Vec3 v1 = Vec3(1.0f, 2.0f, 3.0f);
    Float s = 2.0f;

    u.v = v1 * s;
    REQUIRE(u.f[0] == 2.0f);
    REQUIRE(u.f[1] == 4.0f);
    REQUIRE(u.f[2] == 6.0f);

    u.v = s * v1;
    REQUIRE(u.f[0] == 2.0f);
    REQUIRE(u.f[1] == 4.0f);
    REQUIRE(u.f[2] == 6.0f);

    u.v = v1 / s;
    REQUIRE(u.f[0] == 0.5f);
    REQUIRE(u.f[1] == 1.0f);
    REQUIRE(u.f[2] == 1.5f);
  }

  SECTION("vector absolute") {
    Vec3 v1 = Vec3(1.0f, -2.0f, 0.5f);

    u.v = abs(v1);
    REQUIRE(u.f[0] == 1.0f);
    REQUIRE(u.f[1] == 2.0f);
    REQUIRE(u.f[2] == 0.5f);
  }

  SECTION("vector inner product") {
    Vec3 v1 = Vec3(1.0f, 2.0f, 3.0f);
    Vec3 v2 = Vec3(1.0f, -1.0f, 0.0f);

    REQUIRE((float)dot(v1, v2) == -1.0f);
  }

  SECTION("vector cross product") {
    Vec3 v1 = Vec3(1.0f, 0.0f, 0.0f);
    Vec3 v2 = Vec3(0.0f, 1.0f, 0.0f);

    u.v = cross(v1, v2);
    REQUIRE(u.f[0] == 0.0f);
    REQUIRE(u.f[1] == 0.0f);
    REQUIRE(u.f[2] == 1.0f);
  }

  SECTION("vector normalization") {
    Vec3 v1 = Vec3(1.0f, 1.0f, 1.0f);

    u.v = normalize(v1);
    REQUIRE(u.f[0] == Approx(0.5773502691896258f));
    REQUIRE(u.f[1] == Approx(0.5773502691896258f));
    REQUIRE(u.f[2] == Approx(0.5773502691896258f));
  }

  SECTION("vector length") {
    Vec3 v1 = Vec3(1.0f, 1.0f, 1.0f);
    REQUIRE((float)length(v1) == Approx(1.73205f));
  }

  SECTION("vector lerp") {
    Vec3 v1 = Vec3(1.0f, 2.0f, 3.0f);
    Vec3 v2 = Vec3(5.0f, -2.0f, -5.0f);
    Float t = 0.5f;

    u.v = lerp(t, v1, v2);
    REQUIRE(u.f[0] == 3.0f);
    REQUIRE(u.f[1] == 0.0f);
    REQUIRE(u.f[2] == -1.0f);
  }

  SECTION("vector component-wise min/max") {
    Vec3 v1 = Vec3(1.0f, 2.0f, 3.0f);
    Vec3 v2 = Vec3(5.0f, -2.0f, -5.0f);

    u.v = min(v1, v2);
    REQUIRE(u.f[0] == 1.0f);
    REQUIRE(u.f[1] == -2.0f);
    REQUIRE(u.f[2] == -5.0f);

    u.v = max(v1, v2);
    REQUIRE(u.f[0] == 5.0f);
    REQUIRE(u.f[1] == 2.0f);
    REQUIRE(u.f[2] == 3.0f);
  }

  SECTION("extract Float from vector") {
    Vec3 v1 = Vec3(1.0f, 2.0f, 3.0f);

    REQUIRE((float)(v1.x()) == 1.0f);
    REQUIRE((float)(v1.y()) == 2.0f);
    REQUIRE((float)(v1.z()) == 3.0f);
  }

  SECTION("copy_x copy_y copy_z") {
    Vec3 v1 = Vec3(1.0f, 2.0f, 3.0f);
    Vec3 v2 = Vec3(4.0f, 5.0f, 6.0f);

    u.v = copy_x(v1, v2);
    REQUIRE(u.f[0] == 4.0f);
    REQUIRE(u.f[1] == 2.0f);
    REQUIRE(u.f[2] == 3.0f);

    u.v = copy_y(v1, v2);
    REQUIRE(u.f[0] == 1.0f);
    REQUIRE(u.f[1] == 5.0f);
    REQUIRE(u.f[2] == 3.0f);

    u.v = copy_z(v1, v2);
    REQUIRE(u.f[0] == 1.0f);
    REQUIRE(u.f[1] == 2.0f);
    REQUIRE(u.f[2] == 6.0f);
  }

  SECTION("clamp") {
    Vec3 v1 = Vec3(1.0f, -2.0f, 3.0f);
    Vec3 v2 = Vec3(1.5f, -5.0f, 6.0f);
    Vec3 v3 = Vec3(2.0f, -3.0f, 7.0f);

    u.v = clamp(v1, Float(0.0f), Float(2.5f));
    REQUIRE(u.f[0] == 1.0f);
    REQUIRE(u.f[1] == 0.0f);
    REQUIRE(u.f[2] == 2.5f);

    u.v = clamp(v1, v2, v3);
    REQUIRE(u.f[0] == 1.5f);
    REQUIRE(u.f[1] == -3.0f);
    REQUIRE(u.f[2] == 6.0f);
  }

  SECTION("sqrt and rsqrt") {
    Vec3 v1 = Vec3(1.0f, 4.0f, 0.25f);

    u.v = sqrt(v1);
    REQUIRE(u.f[0] == Approx(1.0f));
    REQUIRE(u.f[1] == Approx(2.0f));
    REQUIRE(u.f[2] == Approx(0.5f));

    u.v = rsqrt(v1);
    REQUIRE(u.f[0] == Approx(1.0f));
    REQUIRE(u.f[1] == Approx(0.5f));
    REQUIRE(u.f[2] == Approx(2.0f));
  }

  SECTION("square and mul") {
    Vec3 v1 = Vec3(1.0f, -2.0f, 3.0f);
    Vec3 v2 = Vec3(5.0f, 2.0f, 10.0f);

    u.v = square(v1);
    REQUIRE(u.f[0] == 1.0f);
    REQUIRE(u.f[1] == 4.0f);
    REQUIRE(u.f[2] == 9.0f);

    u.v = mul(v1, v2);
    REQUIRE(u.f[0] == 5.0f);
    REQUIRE(u.f[1] == -4.0f);
    REQUIRE(u.f[2] == 30.0f);
  }

  SECTION("convert from Vec4") {
    Vec4 v1 = Vec4(1.0f, 2.0f, 3.0f, 4.0f);
    Vec3 v2 = Vec3::cast(v1);

    u.v = v2;
    REQUIRE(u.f[0] == 1.0f);
    REQUIRE(u.f[1] == 2.0f);
    REQUIRE(u.f[2] == 3.0f);
  }

  SECTION("project from Vec4") {
    Vec4 v1= Vec4(1.0f, 2.0f, 3.0f, 4.0f);
    Vec3 v2 = Vec3::proj(v1);

    u.v = v2;
    REQUIRE(u.f[0] == 0.25f);
    REQUIRE(u.f[1] == 0.5f);
    REQUIRE(u.f[2] == 0.75f);
  }
}
