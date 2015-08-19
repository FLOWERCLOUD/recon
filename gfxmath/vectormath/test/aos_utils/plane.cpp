#include "test_helper.h"
#include <vectormath/aos/utils/plane.h>
#include <catch.hpp>

using vectormath::aos::utils::Point3;
using vectormath::aos::utils::Plane;

TEST_CASE("Plane", "[aos_utils_plane]") {
  Vec4_float u;

  SECTION("initialization") {
    Plane p1 = Plane(Vec4(1.0f, 2.0f, 3.0f, 4.0f));
    u.v = p1.coeff;
    REQUIRE(u.f[0] == 1.0f);
    REQUIRE(u.f[1] == 2.0f);
    REQUIRE(u.f[2] == 3.0f);
    REQUIRE(u.f[3] == 4.0f);

    p1 = Plane(Vec3(-1.0f, -2.0f, -3.0f), 5.0f);
    u.v = p1.coeff;
    REQUIRE(u.f[0] == -1.0f);
    REQUIRE(u.f[1] == -2.0f);
    REQUIRE(u.f[2] == -3.0f);
    REQUIRE(u.f[3] == 5.0f);

    p1 = Plane(Vec3(1.0f, 2.0f, 3.0f), Point3(10.0f, 20.0f, 30.0f));
    u.v = p1.coeff;
    REQUIRE(u.f[0] == 1.0f);
    REQUIRE(u.f[1] == 2.0f);
    REQUIRE(u.f[2] == 3.0f);
    REQUIRE(u.f[3] == -140.0f);
  }

  SECTION("dot product") {
    Plane p1 = Plane(Vec4(1.0f, 2.0f, 3.0f, 4.0f));
    REQUIRE((float)(p1[Point3(1.0f, -1.0f, -1.0f)]) == 0.0f);
    REQUIRE((float)(p1[Point3(1.0f, 1.0f, 1.0f)]) == 10.0f);
  }
}
