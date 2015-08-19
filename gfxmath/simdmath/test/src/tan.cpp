#include "test_helper.h"
#include <catch.hpp>

TEST_CASE("y = tan(x)", "[tan]") {
  SECTION("float4") {
    float4 x, y;
    float4_float u;

    x = _mm_setr_ps(0.0f, (float)M_PI/3.0f, (float)M_PI*0.25f, (float)M_PI*0.5f-0.0000001f);
    y = tanf4(x);

    u.vf = y;
    REQUIRE(u.f[0] == Approx(0.0f));
    REQUIRE(u.f[1] == Approx(1.7320508075688772f));
    REQUIRE(u.f[2] == Approx(1.0f));
    REQUIRE(u.f[3] > 100000.0f);
  }
}
