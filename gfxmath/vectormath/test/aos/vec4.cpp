#include "test_helper.h"
#include <catch.hpp>

TEST_CASE("4-component float vector (array of struct)", "[aos_Vec4]") {
  Vec4_float u;

  SECTION("vector initialization") {
    Vec4 v1 = Vec4(1.0f, 2.0f, 3.0f, 4.0f);
    u.v = v1;
    REQUIRE(u.f[0] == 1.0f);
    REQUIRE(u.f[1] == 2.0f);
    REQUIRE(u.f[2] == 3.0f);
    REQUIRE(u.f[3] == 4.0f);

    v1 = Vec4::zero();
    u.v = v1;
    REQUIRE(u.f[0] == 0.0f);
    REQUIRE(u.f[1] == 0.0f);
    REQUIRE(u.f[2] == 0.0f);
    REQUIRE(u.f[3] == 0.0f);
  }

  SECTION("vector load/store") {
    float data[] = { 5.0f, 10.0f, 70.0f, -1.0f };
    Vec4 v1 = Vec4::load(data);

    u.v = v1;
    REQUIRE(u.f[0] == 5.0f);
    REQUIRE(u.f[1] == 10.0f);
    REQUIRE(u.f[2] == 70.0f);
    REQUIRE(u.f[3] == -1.0f);

    for (int i = 0; i < 4; ++i)
      data[i] = 0.0f;

    (-v1).store(data);
    REQUIRE(data[0] == -5.0f);
    REQUIRE(data[1] == -10.0f);
    REQUIRE(data[2] == -70.0f);
    REQUIRE(data[3] == 1.0f);
  }

  SECTION("vector negation") {
    Vec4 v1 = Vec4(1.0f, 2.0f, 3.0f, 4.0f);

    u.v = -v1;
    REQUIRE(u.f[0] == -1.0f);
    REQUIRE(u.f[1] == -2.0f);
    REQUIRE(u.f[2] == -3.0f);
    REQUIRE(u.f[3] == -4.0f);
  }

  SECTION("vector addition") {
    Vec4 v1 = Vec4(1.0f, 2.0f, 3.0f, 4.0f);
    Vec4 v2 = Vec4(10.0f, 1.5f, -200.0f, -4.0f);

    u.v = v1 + v2;
    REQUIRE(u.f[0] == 11.0f);
    REQUIRE(u.f[1] == 3.5f);
    REQUIRE(u.f[2] == -197.0f);
    REQUIRE(u.f[3] == 0.0f);
  }

  SECTION("vector subtraction") {
    Vec4 v1 = Vec4(1.0f, 2.0f, 3.0f, 4.0f);
    Vec4 v2 = Vec4(10.0f, 1.5f, -200.0f, -4.0f);

    u.v = v1 - v2;
    REQUIRE(u.f[0] == -9.0f);
    REQUIRE(u.f[1] == 0.5f);
    REQUIRE(u.f[2] == 203.0f);
    REQUIRE(u.f[3] == 8.0f);
  }

  SECTION("vector scaling") {
    Vec4 v1 = Vec4(1.0f, 2.0f, 3.0f, 4.0f);
    Float s = 2.0f;

    u.v = v1 * s;
    REQUIRE(u.f[0] == 2.0f);
    REQUIRE(u.f[1] == 4.0f);
    REQUIRE(u.f[2] == 6.0f);
    REQUIRE(u.f[3] == 8.0f);

    u.v = s * v1;
    REQUIRE(u.f[0] == 2.0f);
    REQUIRE(u.f[1] == 4.0f);
    REQUIRE(u.f[2] == 6.0f);
    REQUIRE(u.f[3] == 8.0f);

    u.v = v1 / s;
    REQUIRE(u.f[0] == 0.5f);
    REQUIRE(u.f[1] == 1.0f);
    REQUIRE(u.f[2] == 1.5f);
    REQUIRE(u.f[3] == 2.0f);
  }

  SECTION("vector absolute") {
    Vec4 v1 = Vec4(1.0f, -2.0f, 0.5f, -3.5f);

    u.v = abs(v1);
    REQUIRE(u.f[0] == 1.0f);
    REQUIRE(u.f[1] == 2.0f);
    REQUIRE(u.f[2] == 0.5f);
    REQUIRE(u.f[3] == 3.5f);
  }

  SECTION("vector inner product") {
    Vec4 v1 = Vec4(1.0f, 2.0f, 3.0f, 10.0f);
    Vec4 v2 = Vec4(1.0f, -1.0f, 0.0f, 0.5f);

    REQUIRE((float)dot(v1, v2) == 4.0f);
  }

  SECTION("vector normalization") {
    Vec4 v1 = Vec4(1.0f, 1.0f, 1.0f, 1.0f);

    u.v = normalize(v1);
    REQUIRE(u.f[0] == Approx(0.5f));
    REQUIRE(u.f[1] == Approx(0.5f));
    REQUIRE(u.f[2] == Approx(0.5f));
    REQUIRE(u.f[3] == Approx(0.5f));
  }

  SECTION("vector length") {
    Vec4 v1 = Vec4(1.0f, 1.0f, 1.0f, 1.0f);
    REQUIRE((float)length(v1) == Approx(2.0f));
  }

  SECTION("vector lerp") {
    Vec4 v1 = Vec4(1.0f, 2.0f, 3.0f, 4.0f);
    Vec4 v2 = Vec4(5.0f, -2.0f, -5.0f, 1.0f);
    Float t = 0.5f;

    u.v = lerp(t, v1, v2);
    REQUIRE(u.f[0] == 3.0f);
    REQUIRE(u.f[1] == 0.0f);
    REQUIRE(u.f[2] == -1.0f);
    REQUIRE(u.f[3] == 2.5f);
  }

  SECTION("extract Float from vector") {
    Vec4 v1 = Vec4(1.0f, 2.0f, 3.0f, 4.0f);

    REQUIRE((float)(v1.x()) == 1.0f);
    REQUIRE((float)(v1.y()) == 2.0f);
    REQUIRE((float)(v1.z()) == 3.0f);
    REQUIRE((float)(v1.w()) == 4.0f);
  }

  SECTION("convert from Vec3") {
    Vec3 v1 = Vec3(1.0f, 2.0f, 3.0f);
    Vec4 v2 = Vec4(v1, Float(4.0f));

    u.v = v2;
    REQUIRE(u.f[0] == 1.0f);
    REQUIRE(u.f[1] == 2.0f);
    REQUIRE(u.f[2] == 3.0f);
    REQUIRE(u.f[3] == 4.0f);
  }

}
