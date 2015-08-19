#include "test_helper.h"
#include <vectormath/aos/utils/aabox.h>
#include <catch.hpp>

using vectormath::aos::utils::Point3;
using vectormath::aos::utils::AABox;

TEST_CASE("Axis Aligned Bounding Box", "[aos_utils_aabox]") {
  Vec3_float u;

  SECTION("Initialize AABox") {
    AABox b1 = AABox(Point3(1.0f, 2.0f, 3.0f));

    u.v = b1.minpos.data;
    REQUIRE(u.f[0] == 1.0f);
    REQUIRE(u.f[1] == 2.0f);
    REQUIRE(u.f[2] == 3.0f);
    u.v = b1.maxpos.data;
    REQUIRE(u.f[0] == 1.0f);
    REQUIRE(u.f[1] == 2.0f);
    REQUIRE(u.f[2] == 3.0f);

    b1 = AABox(Point3(3.0f, 0.0f, -2.0f), Point3(1.0f, -2.0f, 3.0f));

    u.v = b1.minpos.data;
    REQUIRE(u.f[0] == 1.0f);
    REQUIRE(u.f[1] == -2.0f);
    REQUIRE(u.f[2] == -2.0f);
    u.v = b1.maxpos.data;
    REQUIRE(u.f[0] == 3.0f);
    REQUIRE(u.f[1] == 0.0f);
    REQUIRE(u.f[2] == 3.0f);
  }

  SECTION("Add point to AABox") {
    AABox b1 = AABox(Point3(1.0f, 2.0f, 3.0f));
    b1.add(Point3(-1.0f, 4.0f, 1.0f));

    u.v = b1.minpos.data;
    REQUIRE(u.f[0] == -1.0f);
    REQUIRE(u.f[1] == 2.0f);
    REQUIRE(u.f[2] == 1.0f);
    u.v = b1.maxpos.data;
    REQUIRE(u.f[0] == 1.0f);
    REQUIRE(u.f[1] == 4.0f);
    REQUIRE(u.f[2] == 3.0f);
  }

  SECTION("Extract center and extent") {
    AABox b1 = AABox(Point3(1.0f, 2.0f, 3.0f), Point3(2.0f, 3.0f, 4.0f));
    Point3 c = b1.center();
    Vec3 e = b1.extent();

    u.v = (Vec3)c;
    REQUIRE(u.f[0] == 1.5f);
    REQUIRE(u.f[1] == 2.5f);
    REQUIRE(u.f[2] == 3.5f);

    u.v = e;
    REQUIRE(u.f[0] == 1.0f);
    REQUIRE(u.f[1] == 1.0f);
    REQUIRE(u.f[2] == 1.0f);
  }

  SECTION("Linear Interpolation") {
    AABox b1 = AABox(Point3(1.0f, 2.0f, 3.0f), Point3(2.0f, 4.0f, 7.0f));
    Point3 p = b1.lerp(0.2f, 0.5f, 0.6f);

    u.v = (Vec3)p;
    REQUIRE(u.f[0] == 1.2f);
    REQUIRE(u.f[1] == 3.0f);
    REQUIRE(u.f[2] == 5.4f);
  }

  SECTION("Extract corners in Morton order") {
    AABox b1 = AABox(Point3(1.0f, 2.0f, 3.0f), Point3(2.0f, 3.0f, 4.0f));
    Point3 p;

    p = b1.corner0();
    REQUIRE((float)p.x() == 1.0f);
    REQUIRE((float)p.y() == 2.0f);
    REQUIRE((float)p.z() == 3.0f);

    p = b1.corner1();
    REQUIRE((float)p.x() == 2.0f);
    REQUIRE((float)p.y() == 2.0f);
    REQUIRE((float)p.z() == 3.0f);

    p = b1.corner2();
    REQUIRE((float)p.x() == 1.0f);
    REQUIRE((float)p.y() == 3.0f);
    REQUIRE((float)p.z() == 3.0f);

    p = b1.corner3();
    REQUIRE((float)p.x() == 2.0f);
    REQUIRE((float)p.y() == 3.0f);
    REQUIRE((float)p.z() == 3.0f);

    p = b1.corner4();
    REQUIRE((float)p.x() == 1.0f);
    REQUIRE((float)p.y() == 2.0f);
    REQUIRE((float)p.z() == 4.0f);

    p = b1.corner5();
    REQUIRE((float)p.x() == 2.0f);
    REQUIRE((float)p.y() == 2.0f);
    REQUIRE((float)p.z() == 4.0f);

    p = b1.corner6();
    REQUIRE((float)p.x() == 1.0f);
    REQUIRE((float)p.y() == 3.0f);
    REQUIRE((float)p.z() == 4.0f);

    p = b1.corner7();
    REQUIRE((float)p.x() == 2.0f);
    REQUIRE((float)p.y() == 3.0f);
    REQUIRE((float)p.z() == 4.0f);
  }

}
