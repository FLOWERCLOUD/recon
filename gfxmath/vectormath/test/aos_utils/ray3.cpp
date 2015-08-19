#include "test_helper.h"
#include <vectormath/aos/utils/ray3.h>
#include <catch.hpp>

using vectormath::aos::utils::Point3;
using vectormath::aos::utils::Ray3;

TEST_CASE("3D Ray", "[aos_utils_Ray3]") {
  SECTION("Initialize") {
    Ray3 r1 = Ray3(Point3(1.0f, 2.0f, 3.0f), Vec3(2.0f, -1.0f, 0.0f));
    Point3 s = r1.start;
    Vec3 d = r1.diff;
    REQUIRE((float)s.x() == 1.0f);
    REQUIRE((float)s.y() == 2.0f);
    REQUIRE((float)s.z() == 3.0f);
    REQUIRE((float)d.x() == 2.0f);
    REQUIRE((float)d.y() == -1.0f);
    REQUIRE((float)d.z() == 0.0f);

    r1 = Ray3::make(Point3(1.0f, 2.0f, 3.0f), Point3(2.0f, 1.0f, 4.0f));
    s = r1.start;
    d = r1.diff;
    REQUIRE((float)s.x() == 1.0f);
    REQUIRE((float)s.y() == 2.0f);
    REQUIRE((float)s.z() == 3.0f);
    REQUIRE((float)d.x() == Approx(0.5773502691896258f));
    REQUIRE((float)d.y() == Approx(-0.5773502691896258f));
    REQUIRE((float)d.z() == Approx(0.5773502691896258f));
  }

  SECTION("Extract a point") {
    Ray3 r1 = Ray3(Point3(1.0f, 2.0f, 3.0f), Vec3(2.0f, -1.0f, 0.0f));
    Point3 p = r1[ 0.5f ];
    REQUIRE((float)p.x() == 2.0f);
    REQUIRE((float)p.y() == 1.5f);
    REQUIRE((float)p.z() == 3.0f);
  }

  SECTION("Projection") {
    Ray3 r1 = Ray3(Point3(1.0f, 2.0f, 3.0f), Vec3(1.0f, 0.0f, 0.0f));
    Point3 p1 = Point3(4.0f, 6.0f, 100.0f);
    Float s = r1.projection(p1);
    REQUIRE((float)s == 3.0f);
  }
}
