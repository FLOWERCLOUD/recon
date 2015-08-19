#include "test_helper.h"
#include <catch.hpp>

TEST_CASE("4-component float vector (struct of array)", "[soa_Vec4]") {
  void* space = aligned_alloc(alignof(Vec4), sizeof(float) * 4 * FLOATVEC_SIZE);
  float* pxyzw = (float*)space;

  REQUIRE(pxyzw);

  SECTION("load and store") {
    // 0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15
    for (int i = 0; i < 4 * FLOATVEC_SIZE; ++i) {
      pxyzw[i] = static_cast<float>(i);
    }

    Vec4 v0;
    load(v0, pxyzw);

    float* px = (float*) &v0.x; // 0 4 8  12
    float* py = (float*) &v0.y; // 1 5 9  13
    float* pz = (float*) &v0.z; // 2 6 10 14
    float* pw = (float*) &v0.w; // 3 7 11 15

    for (int i = 0; i < FLOATVEC_SIZE; ++i) {
      REQUIRE(px[i] == i * 4);
      REQUIRE(py[i] == i * 4 + 1);
      REQUIRE(pz[i] == i * 4 + 2);
      REQUIRE(pw[i] == i * 4 + 3);
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
    v0.w = vf.v; // 0 1 2 3

    store(pxyzw, v0);

    // 0 0 0 0 1 1 1 1 2 2 2 2 3 3 3 3
    for (int i = 0; i < FLOATVEC_SIZE; ++i) {
      REQUIRE(pxyzw[4*i+0] == static_cast<float>(i));
      REQUIRE(pxyzw[4*i+1] == static_cast<float>(i));
      REQUIRE(pxyzw[4*i+2] == static_cast<float>(i));
      REQUIRE(pxyzw[4*i+3] == static_cast<float>(i));
    }
  }

  aligned_free(space);
}
