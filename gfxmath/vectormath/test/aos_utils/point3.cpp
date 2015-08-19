#include "test_helper.h"
#include <vectormath/aos/utils/point3.h>
#include <catch.hpp>

using vectormath::aos::utils::Point3;

TEST_CASE("Point3", "[aos_utils_Point3]") {
  Vec3_float u;

  SECTION("point initialization") {
    Point3 p1 = Point3(1.0f, 2.0f, 3.0f);
    u.v = p1.data;
    REQUIRE(u.f[0] == 1.0f);
    REQUIRE(u.f[1] == 2.0f);
    REQUIRE(u.f[2] == 3.0f);

    p1 = Point3::zero();
    u.v = p1.data;
    REQUIRE(u.f[0] == 0.0f);
    REQUIRE(u.f[1] == 0.0f);
    REQUIRE(u.f[2] == 0.0f);
  }

  SECTION("point load/store") {
    float data[] = { 5.0f, 10.0f, 70.0f };
    Point3 p1 = Point3::load(data);

    u.v = p1.data;
    REQUIRE(u.f[0] == 5.0f);
    REQUIRE(u.f[1] == 10.0f);
    REQUIRE(u.f[2] == 70.0f);

    for (int i = 0; i < 3; ++i)
      data[i] = 0.0f;

    p1.store(data);
    REQUIRE(data[0] == 5.0f);
    REQUIRE(data[1] == 10.0f);
    REQUIRE(data[2] == 70.0f);
  }

  SECTION("point from/to vector") {
    Point3 p1 = Point3(1.0f, 2.0f, 3.0f);

    u.v = (Vec3)p1;
    REQUIRE(u.f[0] == 1.0f);
    REQUIRE(u.f[1] == 2.0f);
    REQUIRE(u.f[2] == 3.0f);

    p1 = Point3(Vec3(-2.0f, 100.0f, 3.14f));
    u.v = p1.data;
    REQUIRE(u.f[0] == -2.0f);
    REQUIRE(u.f[1] == 100.0f);
    REQUIRE(u.f[2] == 3.14f);
  }

  SECTION("extract Float from point") {
    Point3 p1 = Point3(1.0f, 2.0f, 3.0f);

    REQUIRE((float)(p1.x()) == 1.0f);
    REQUIRE((float)(p1.y()) == 2.0f);
    REQUIRE((float)(p1.z()) == 3.0f);
  }

  SECTION("point + vector") {
    Point3 p1 = Point3(1.0f, 2.0f, 3.0f);
    Vec3 v2 = Vec3(10.0f, 1.5f, -200.0f);

    Point3 p3 = p1 + v2;
    u.v = p3.data;
    REQUIRE(u.f[0] == 11.0f);
    REQUIRE(u.f[1] == 3.5f);
    REQUIRE(u.f[2] == -197.0f);
  }

  SECTION("point - vector") {
    Point3 p1 = Point3(1.0f, 2.0f, 3.0f);
    Vec3 v2 = Vec3(10.0f, 1.5f, -200.0f);

    Point3 p3 = p1 - v2;
    u.v = p3.data;
    REQUIRE(u.f[0] == -9.0f);
    REQUIRE(u.f[1] == 0.5f);
    REQUIRE(u.f[2] == 203.0f);
  }

  SECTION("vector subtraction") {
    Point3 p1 = Point3(1.0f, 2.0f, 3.0f);
    Point3 p2 = Point3(10.0f, 1.5f, -200.0f);
    Vec3 v3 = p1 - p2;

    u.v = v3;
    REQUIRE(u.f[0] == -9.0f);
    REQUIRE(u.f[1] == 0.5f);
    REQUIRE(u.f[2] == 203.0f);
  }
}
