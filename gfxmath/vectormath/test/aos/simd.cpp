#include <catch.hpp>
#include <simdmath.h>

#if defined(__i386__)||defined(__x86_64__)
#  include <immintrin.h>
#endif

TEST_CASE("Check SIMD Ability", "simd_caps") {

#if defined(__SSE__)
  WARN("compiled with SSE");
#endif

#if defined(__SSE2__)
  WARN("compiled with SSE 2");
#endif

#if defined(__SSE3__)
  WARN("compiled with SSE 3");
#endif

#if defined(__SSSE3__)
  WARN("compiled with SSSE 3");
#endif

#if defined(__SSE4_1__)
  WARN("compiled with SSE 4.1");
#endif

#if defined(__AVX__)
  WARN("compiled with AVX");
#endif

#if defined(__FMA__)
  WARN("compiled with FMA");
#endif

#if defined(__AVX512F__)
  WARN("compiled with AVX 512");
#endif

#if defined(__AVX2__)
  WARN("compiled with AVX 2");
#endif

}
