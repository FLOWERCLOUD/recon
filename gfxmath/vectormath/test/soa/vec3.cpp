#include "test_helper.h"
#include <catch.hpp>

TEST_CASE("3-component float vector (struct of array)", "[soa_Vec3]") {
  void* space = aligned_alloc(alignof(Vec3), sizeof(float) * 3 * FLOATVEC_SIZE);
  float* pxyz = (float*)space;

  REQUIRE(pxyz);

  SECTION("load and store") {
    // 0 1 2 3 4 5 6 7 8 9 10 11
    for (int i = 0; i < 3 * FLOATVEC_SIZE; ++i) {
      pxyz[i] = static_cast<float>(i);
    }

    Vec3 v0;
    load(v0, pxyz);

    float* px = (float*) &v0.x; // 0 3 6 9
    float* py = (float*) &v0.y; // 1 4 7 10
    float* pz = (float*) &v0.z; // 2 5 8 11

    for (int i = 0; i < FLOATVEC_SIZE; ++i) {
      REQUIRE(px[i] == i * 3);
      REQUIRE(py[i] == i * 3 + 1);
      REQUIRE(pz[i] == i * 3 + 2);
    }

    union {
      floatvec v;
      float f[FLOATVEC_SIZE];
    } vf;

    for (int i = 0; i < FLOATVEC_SIZE; ++i) {
      vf.f[i] = static_cast<float>(i);
    }
    v0.x = vf.v; // 0 1 2 3
    v0.y = vf.v; // 0 1 2 3
    v0.z = vf.v; // 0 1 2 3

    store(pxyz, v0);

    // 0 0 0 1 1 1 2 2 2 3 3 3
    for (int i = 0; i < 3 * FLOATVEC_SIZE; i += 3) {
      REQUIRE(pxyz[i+0] == static_cast<float>(i/3));
      REQUIRE(pxyz[i+1] == static_cast<float>(i/3));
      REQUIRE(pxyz[i+2] == static_cast<float>(i/3));
    }
  }

  aligned_free(space);
}
