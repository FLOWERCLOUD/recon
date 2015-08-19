#include "test_helper.h"
#include <catch.hpp>

TEST_CASE("float scalar (array of struct)", "[aos_Float]") {
  SECTION("Float initialization") {
    Float s = Float(3.14f);
    REQUIRE((float)s == 3.14f);

    s = 0.0f;
    REQUIRE((float)s == 0.0f);
  }

  SECTION("Float load/store") {
    float data[] = { 1234.0f };
    Float s = Float::load(data);
    REQUIRE((float)s == 1234.0f);

    float d[] = { 0.0f };
    s.store(d);
    REQUIRE(d[0] == 1234.0f);
  }

  SECTION("scalar negation") {
    Float s = 3.14f;
    REQUIRE((float)(-s) == -3.14f);
  }

  SECTION("scalar addition") {
    Float s1 = 1.0f;
    Float s2 = 3.5f;
    REQUIRE((float)(s1 + s2) == 4.5f);
  }

  SECTION("scalar subtraction") {
    Float s1 = 1.0f;
    Float s2 = 3.5f;
    REQUIRE((float)(s1 - s2) == -2.5f);
  }

  SECTION("scalar multiplication") {
    Float s1 = 2.0f;
    Float s2 = 3.5f;
    REQUIRE((float)(s1 * s2) == 7.0f);
  }

  SECTION("scalar division") {
    Float s1 = 3.5f;
    Float s2 = 2.0f;
    REQUIRE((float)(s1 / s2) == 1.75f);
  }

  SECTION("scalar absolute") {
    Float s1 = -3.14f;
    REQUIRE((float)abs(s1) == 3.14f);
  }
}

using vectormath::aos::PFloat;

TEST_CASE("4-component parallel float scalar (array of struct)", "[aos_PFloat]") {
  SECTION("PFloat from Float") {
    Float s1 = 3.14f;
    PFloat s2 = s1;
    REQUIRE((float)s2 == 3.14f);

    s2 = Float(4.0f);
    REQUIRE((float)s2 == 4.0f);
  }

  SECTION("PFloat to Float") {
    PFloat s1 = Float(10.0f);
    Float s2 = s1;
    REQUIRE((float)s2 == 10.0f);
  }

  SECTION("PFloat from float") {
    PFloat s1 = 2.0f;
    REQUIRE((float)s1 == 2.0f);
  }

  SECTION("scalar negation") {
    PFloat s = 3.14f;
    REQUIRE((float)(-s) == -3.14f);
  }

  SECTION("scalar addition") {
    PFloat s1 = 1.0f;
    PFloat s2 = 3.5f;
    REQUIRE((float)(s1 + s2) == 4.5f);
  }

  SECTION("scalar subtraction") {
    PFloat s1 = 1.0f;
    PFloat s2 = 3.5f;
    REQUIRE((float)(s1 - s2) == -2.5f);
  }

  SECTION("scalar multiplication") {
    PFloat s1 = 2.0f;
    PFloat s2 = 3.5f;
    REQUIRE((float)(s1 * s2) == 7.0f);
  }

  SECTION("scalar division") {
    PFloat s1 = 3.5f;
    PFloat s2 = 2.0f;
    REQUIRE((float)(s1 / s2) == 1.75f);
  }

  SECTION("scalar absolute") {
    PFloat s1 = -3.14f;
    REQUIRE((float)abs(s1) == 3.14f);
  }
}
