#pragma once

#include <vectormath.h>

using vectormath::soa::FLOATVEC_SIZE;
using vectormath::soa::floatvec;
using vectormath::soa::Vec3;
using vectormath::soa::Vec4;
using vectormath::soa::load;
using vectormath::soa::store;

#ifdef __APPLE__
#  include <malloc/malloc.h>
#  define aligned_alloc(a,s) malloc_zone_memalign(malloc_default_zone(), (a), (s))
#endif

#if (defined(_MSC_VER) && _MSC_VER < 1900) || defined(__MINGW32__)
#  define aligned_alloc(a,s) _aligned_malloc(s, a)
#  define aligned_free(p) _aligned_free(p)
#else
#  define aligned_free(p) free(p)
#endif
