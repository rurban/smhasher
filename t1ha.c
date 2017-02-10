/*
 *  Copyright (c) 2016-2017 Positive Technologies, https://www.ptsecurity.com,
 *  Fast Positive Hash.
 *
 *  Portions Copyright (c) 2010-2017 Leonid Yuriev <leo@yuriev.ru>,
 *  The 1Hippeus project (t1h).
 *
 *  This software is provided 'as-is', without any express or implied
 *  warranty. In no event will the authors be held liable for any damages
 *  arising from the use of this software.
 *
 *  Permission is granted to anyone to use this software for any purpose,
 *  including commercial applications, and to alter it and redistribute it
 *  freely, subject to the following restrictions:
 *
 *  1. The origin of this software must not be misrepresented; you must not
 *     claim that you wrote the original software. If you use this software
 *     in a product, an acknowledgement in the product documentation would be
 *     appreciated but is not required.
 *  2. Altered source versions must be plainly marked as such, and must not be
 *     misrepresented as being the original software.
 *  3. This notice may not be removed or altered from any source distribution.
 */

/*
 * t1ha = { Fast Positive Hash, aka "Позитивный Хэш" }
 * by [Positive Technologies](https://www.ptsecurity.ru)
 *
 * Briefly, it is a 64-bit Hash Function:
 *  1. Created for 64-bit little-endian platforms, in predominantly for x86_64,
 *     but without penalties could runs on any 64-bit CPU.
 *  2. In most cases up to 15% faster than City64, xxHash, mum-hash, metro-hash
 *     and all others which are not use specific hardware tricks.
 *  3. Not suitable for cryptography.
 *
 * ACKNOWLEDGEMENT:
 * The t1ha was originally developed by Leonid Yuriev (Леонид Юрьев)
 * for The 1Hippeus project - zerocopy messaging in the spirit of Sparta!
 */

#include "t1ha.h"
#include <string.h> /* for memcpy() */

#if !defined(__BYTE_ORDER__) || !defined(__ORDER_LITTLE_ENDIAN__) ||           \
    !defined(__ORDER_BIG_ENDIAN__)
#ifndef _MSC_VER
#include <sys/param.h> /* for endianness */
#endif
#if defined(__BYTE_ORDER) && defined(__LITTLE_ENDIAN) && defined(__BIG_ENDIAN)
#define __ORDER_LITTLE_ENDIAN__ __LITTLE_ENDIAN
#define __ORDER_BIG_ENDIAN__ __BIG_ENDIAN
#define __BYTE_ORDER__ __BYTE_ORDER
#else
#define __ORDER_LITTLE_ENDIAN__ 1234
#define __ORDER_BIG_ENDIAN__ 4321
#if defined(__LITTLE_ENDIAN__) || defined(_LITTLE_ENDIAN) ||                   \
    defined(__ARMEL__) || defined(__THUMBEL__) || defined(__AARCH64EL__) ||    \
    defined(__MIPSEL__) || defined(_MIPSEL) || defined(__MIPSEL) ||            \
    defined(__i386) || defined(__x86_64__) || defined(_M_IX86) ||              \
    defined(_M_X64) || defined(i386) || defined(_X86_) || defined(__i386__) || \
    defined(_X86_64_) || defined(_M_ARM) || defined(__e2k__)
#define __BYTE_ORDER__ __ORDER_LITTLE_ENDIAN__
#elif defined(__BIG_ENDIAN__) || defined(_BIG_ENDIAN) || defined(__ARMEB__) || \
    defined(__THUMBEB__) || defined(__AARCH64EB__) || defined(__MIPSEB__) ||   \
    defined(_MIPSEB) || defined(__MIPSEB)
#define __BYTE_ORDER__ __ORDER_BIG_ENDIAN__
#else
#error __BYTE_ORDER__ should be defined.
#endif
#endif
#endif

#if __BYTE_ORDER__ != __ORDER_LITTLE_ENDIAN__ &&                               \
    __BYTE_ORDER__ != __ORDER_BIG_ENDIAN__
#error Unsupported byte order.
#endif

#if !defined(UNALIGNED_OK)
#if defined(__i386) || defined(__x86_64__) || defined(_M_IX86) ||              \
    defined(_M_X64) || defined(i386) || defined(_X86_) || defined(__i386__) || \
    defined(_X86_64_)
#define UNALIGNED_OK 1
#else
#define UNALIGNED_OK 0
#endif
#endif

#ifndef __has_builtin
#define __has_builtin(x) (0)
#endif

#if __GNUC_PREREQ(4, 4) || defined(__clang__)

#if defined(__i386__) || defined(__x86_64__)
#include <cpuid.h>
#include <x86intrin.h>
#endif
#define likely(cond) __builtin_expect(!!(cond), 1)
#define unlikely(cond) __builtin_expect(!!(cond), 0)
#if __GNUC_PREREQ(4, 5) || defined(__clang__)
#define unreachable() __builtin_unreachable()
#endif
#define bswap64(v) __builtin_bswap64(v)
#define bswap32(v) __builtin_bswap32(v)
#if __GNUC_PREREQ(4, 8) || __has_builtin(__builtin_bswap16)
#define bswap16(v) __builtin_bswap16(v)
#endif
#if __GNUC_PREREQ(4, 3) || __has_attribute(unused)
#define maybe_unused __attribute__((unused))
#endif

#elif defined(_MSC_VER)

#if _MSC_FULL_VER < 190024215
#if _MSC_FULL_VER < 180040629 && defined(_M_IX86)
#error Please use Visual Studio 2015 (MSC 19.0) or newer for 32-bit target.
#else
#pragma message(                                                               \
    "It is recommended to use Visual Studio 2015 (MSC 19.0) or newer.")
#endif
#endif

#pragma warning(                                                               \
    disable : 4710) /* C4710: C4710: 'mux64': function not inlined */
#pragma warning(                                                               \
    disable : 4711) /* C4711: function 'x86_cpu_features' selected for         \
                       automatic inline expansion */

#include <intrin.h>
#include <stdlib.h>
#define likely(cond) (cond)
#define unlikely(cond) (cond)
#define unreachable() __assume(0)
#define bswap64(v) _byteswap_uint64(v)
#define bswap32(v) _byteswap_ulong(v)
#define bswap16(v) _byteswap_ushort(v)
#define rot64(v, s) _rotr64(v, s)
#define rot32(v, s) _rotr(v, s)
#define __inline __forceinline

#if defined(_M_ARM64) || defined(_M_X64)
#pragma intrinsic(_umul128)
#define mul_64x64_128(a, b, ph) _umul128(a, b, ph)
#pragma intrinsic(__umulh)
#define mul_64x64_high(a, b) __umulh(a, b)
#endif

#if defined(_M_IX86)
#pragma intrinsic(__emulu)
#define mul_32x32_64(a, b) __emulu(a, b)
#elif defined(_M_ARM)
#define mul_32x32_64(a, b) _arm_umull(a, b)
#endif

#endif /* Compiler */

#ifndef likely
#define likely(cond) (cond)
#endif
#ifndef unlikely
#define unlikely(cond) (cond)
#endif
#ifndef maybe_unused
#define maybe_unused
#endif
#ifndef unreachable
#define unreachable()                                                          \
  do {                                                                         \
  } while (1)
#endif

#ifndef bswap64
static __inline uint64_t bswap64(uint64_t v) {
  return v << 56 | v >> 56 | ((v << 40) & 0x00ff000000000000ull) |
         ((v << 24) & 0x0000ff0000000000ull) |
         ((v << 8) & 0x000000ff00000000ull) |
         ((v >> 8) & 0x00000000ff000000ull) |
         ((v >> 24) & 0x0000000000ff0000ull) |
         ((v >> 40) & 0x000000000000ff00ull);
}
#endif /* bswap64 */

#ifndef bswap32
static __inline uint32_t bswap32(uint32_t v) {
  return v << 24 | v >> 24 | ((v << 8) & 0x00ff0000) | ((v >> 8) & 0x0000ff00);
}
#endif /* bswap32 */

#ifndef bswap16
static __inline uint16_t bswap16(uint16_t v) { return v << 8 | v >> 8; }
#endif /* bswap16 */

#ifndef rot64
static __inline uint64_t rot64(uint64_t v, unsigned s) {
  return (v >> s) | (v << (64 - s));
}
#endif /* rot64 */

#ifndef rot32
static __inline uint32_t rot32(uint32_t v, unsigned s) {
  return (v >> s) | (v << (32 - s));
}
#endif /* rot32 */

#ifndef mul_32x32_64
static __inline uint64_t mul_32x32_64(uint32_t a, uint32_t b) {
  return a * (uint64_t)b;
}
#endif /* mul_32x32_64 */

/***************************************************************************/

static __inline uint64_t fetch64_le(const void *v) {
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
  return *(const uint64_t *)v;
#else
  return bswap64(*(const uint64_t *)v);
#endif
}

static __inline uint32_t fetch32_le(const void *v) {
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
  return *(const uint32_t *)v;
#else
  return bswap32(*(const uint32_t *)v);
#endif
}

static __inline uint16_t fetch16_le(const void *v) {
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
  return *(const uint16_t *)v;
#else
  return bswap16(*(const uint16_t *)v);
#endif
}

static __inline uint64_t tail64_le(const void *v, size_t tail) {
  const uint8_t *p = (const uint8_t *)v;
  uint64_t r = 0;
  switch (tail & 7) {
#if UNALIGNED_OK && __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
  /* For most CPUs this code is better when not needed
   * copying for alignment or byte reordering. */
  case 0:
    return fetch64_le(p);
  case 7:
    r = (uint64_t)p[6] << 8;
  case 6:
    r += p[5];
    r <<= 8;
  case 5:
    r += p[4];
    r <<= 32;
  case 4:
    return r + fetch32_le(p);
  case 3:
    r = (uint64_t)p[2] << 16;
  case 2:
    return r + fetch16_le(p);
  case 1:
    return p[0];
#else
  /* For most CPUs this code is better than a
   * copying for alignment and/or byte reordering. */
  case 0:
    r = p[7] << 8;
  case 7:
    r += p[6];
    r <<= 8;
  case 6:
    r += p[5];
    r <<= 8;
  case 5:
    r += p[4];
    r <<= 8;
  case 4:
    r += p[3];
    r <<= 8;
  case 3:
    r += p[2];
    r <<= 8;
  case 2:
    r += p[1];
    r <<= 8;
  case 1:
    return r + p[0];
#endif
  }
  unreachable();
}

/* 'magic' primes */
static const uint64_t p0 = 17048867929148541611ull;
static const uint64_t p1 = 9386433910765580089ull;
static const uint64_t p2 = 15343884574428479051ull;
static const uint64_t p3 = 13662985319504319857ull;
static const uint64_t p4 = 11242949449147999147ull;
static const uint64_t p5 = 13862205317416547141ull;
static const uint64_t p6 = 14653293970879851569ull;

/* rotations */
static const unsigned s0 = 41;
static const unsigned s1 = 17;
static const unsigned s2 = 31;

/* xor-mul-xor mixer */
static __inline uint64_t mix(uint64_t v, uint64_t p) {
  v *= p;
  return v ^ rot64(v, s0);
}

static maybe_unused __inline unsigned add_with_carry(uint64_t *sum,
                                                     uint64_t addend) {
  *sum += addend;
  return *sum < addend;
}

/* xor high and low parts of full 128-bit product */
static __inline uint64_t mux64(uint64_t v, uint64_t p) {
#ifdef __SIZEOF_INT128__
  __uint128_t r = (__uint128_t)v * (__uint128_t)p;
  /* modern GCC could nicely optimize this */
  return r ^ (r >> 64);
#elif defined(_INTEGRAL_MAX_BITS) && _INTEGRAL_MAX_BITS >= 128
  __uint128 r = (__uint128)v * (__uint128)p;
  return r ^ (r >> 64);
#elif defined(mul_64x64_128)
  uint64_t l, h;
  l = mul_64x64_128(v, p, &h);
  return l ^ h;
#elif defined(mul_64x64_high)
  uint64_t l, h;
  l = v * p;
  h = mul_64x64_high(v, p);
  return l ^ h;
#else
  /* performs 64x64 to 128 bit multiplication */
  uint64_t ll = mul_32x32_64((uint32_t)v, (uint32_t)p);
  uint64_t lh = mul_32x32_64(v >> 32, (uint32_t)p);
  uint64_t hl = mul_32x32_64(p >> 32, (uint32_t)v);
  uint64_t hh =
      mul_32x32_64(v >> 32, p >> 32) + (lh >> 32) + (hl >> 32) +
      /* Few simplification are possible here for 32-bit architectures,
       * but thus we would lost compatibility with the original 64-bit
       * version.  Think is very bad idea, because then 32-bit t1ha will
       * still (relatively) very slowly and well yet not compatible. */
      add_with_carry(&ll, lh << 32) + add_with_carry(&ll, hl << 32);
  return hh ^ ll;
#endif
}

uint64_t t1ha(const void *data, size_t len, uint64_t seed) {
  uint64_t a = seed;
  uint64_t b = len;

  const int need_align = (((uintptr_t)data) & 7) != 0 && !UNALIGNED_OK;
  uint64_t align[4];

  if (unlikely(len > 32)) {
    uint64_t c = rot64(len, s1) + seed;
    uint64_t d = len ^ rot64(seed, s1);
    const void *detent = (const uint8_t *)data + len - 31;
    do {
      const uint64_t *v = (const uint64_t *)data;
      if (unlikely(need_align))
        v = (const uint64_t *)memcpy(&align, v, 32);

      uint64_t w0 = fetch64_le(v + 0);
      uint64_t w1 = fetch64_le(v + 1);
      uint64_t w2 = fetch64_le(v + 2);
      uint64_t w3 = fetch64_le(v + 3);

      uint64_t d02 = w0 ^ rot64(w2 + d, s1);
      uint64_t c13 = w1 ^ rot64(w3 + c, s1);
      c += a ^ rot64(w0, s0);
      d -= b ^ rot64(w1, s2);
      a ^= p1 * (d02 + w3);
      b ^= p0 * (c13 + w2);
      data = (const uint64_t *)data + 4;
    } while (likely(data < detent));

    a ^= p6 * (rot64(c, s1) + d);
    b ^= p5 * (c + rot64(d, s1));
    len &= 31;
  }

  const uint64_t *v = (const uint64_t *)data;
  if (unlikely(need_align) && len > 8)
    v = (const uint64_t *)memcpy(&align, v, len);

  switch (len) {
  default:
    b += mux64(fetch64_le(v++), p4);
  case 24:
  case 23:
  case 22:
  case 21:
  case 20:
  case 19:
  case 18:
  case 17:
    a += mux64(fetch64_le(v++), p3);
  case 16:
  case 15:
  case 14:
  case 13:
  case 12:
  case 11:
  case 10:
  case 9:
    b += mux64(fetch64_le(v++), p2);
  case 8:
  case 7:
  case 6:
  case 5:
  case 4:
  case 3:
  case 2:
  case 1:
    a += mux64(tail64_le(v, len), p1);
  case 0:
    return mux64(rot64(a + b, s1), p4) + mix(a ^ b, p0);
  }
}

/***************************************************************************/

static maybe_unused __inline uint64_t fetch64_be(const void *v) {
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
  return *(const uint64_t *)v;
#else
  return bswap64(*(const uint64_t *)v);
#endif
}

static maybe_unused __inline uint32_t fetch32_be(const void *v) {
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
  return *(const uint32_t *)v;
#else
  return bswap32(*(const uint32_t *)v);
#endif
}

static maybe_unused __inline uint16_t fetch16_be(const void *v) {
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
  return *(const uint16_t *)v;
#else
  return bswap16(*(const uint16_t *)v);
#endif
}

static maybe_unused __inline uint64_t tail64_be(const void *v, size_t tail) {
  const uint8_t *p = (const uint8_t *)v;
  switch (tail & 7) {
#if UNALIGNED_OK && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
  /* For most CPUs this code is better when not needed
   * copying for alignment or byte reordering. */
  case 1:
    return p[0];
  case 2:
    return fetch16_be(p);
  case 3:
    return (uint32_t)fetch16_be(p) << 8 | p[2];
  case 4:
    return fetch32_be(p);
  case 5:
    return (uint64_t)fetch32_be(p) << 8 | p[4];
  case 6:
    return (uint64_t)fetch32_be(p) << 16 | fetch16_be(p + 4);
  case 7:
    return (uint64_t)fetch32_be(p) << 24 | (uint32_t)fetch16_be(p + 4) << 8 |
           p[6];
  case 0:
    return fetch64_be(p);
#else
  /* For most CPUs this code is better than a
   * copying for alignment and/or byte reordering. */
  case 1:
    return p[0];
  case 2:
    return p[1] | (uint32_t)p[0] << 8;
  case 3:
    return p[2] | (uint32_t)p[1] << 8 | (uint32_t)p[0] << 16;
  case 4:
    return p[3] | (uint32_t)p[2] << 8 | (uint32_t)p[1] << 16 |
           (uint32_t)p[0] << 24;
  case 5:
    return p[4] | (uint32_t)p[3] << 8 | (uint32_t)p[2] << 16 |
           (uint32_t)p[1] << 24 | (uint64_t)p[0] << 32;
  case 6:
    return p[5] | (uint32_t)p[4] << 8 | (uint32_t)p[3] << 16 |
           (uint32_t)p[2] << 24 | (uint64_t)p[1] << 32 | (uint64_t)p[0] << 40;
  case 7:
    return p[6] | (uint32_t)p[5] << 8 | (uint32_t)p[4] << 16 |
           (uint32_t)p[3] << 24 | (uint64_t)p[2] << 32 | (uint64_t)p[1] << 40 |
           (uint64_t)p[0] << 48;
  case 0:
    return p[7] | (uint32_t)p[6] << 8 | (uint32_t)p[5] << 16 |
           (uint32_t)p[4] << 24 | (uint64_t)p[3] << 32 | (uint64_t)p[2] << 40 |
           (uint64_t)p[1] << 48 | (uint64_t)p[0] << 56;
#endif
  }
  unreachable();
}

uint64_t t1ha_64be(const void *data, size_t len, uint64_t seed) {
  uint64_t a = seed;
  uint64_t b = len;

  const int need_align = (((uintptr_t)data) & 7) != 0 && !UNALIGNED_OK;
  uint64_t align[4];

  if (unlikely(len > 32)) {
    uint64_t c = rot64(len, s1) + seed;
    uint64_t d = len ^ rot64(seed, s1);
    const void *detent = (const uint8_t *)data + len - 31;
    do {
      const uint64_t *v = (const uint64_t *)data;
      if (unlikely(need_align))
        v = (const uint64_t *)memcpy(&align, v, 32);

      uint64_t w0 = fetch64_be(v + 0);
      uint64_t w1 = fetch64_be(v + 1);
      uint64_t w2 = fetch64_be(v + 2);
      uint64_t w3 = fetch64_be(v + 3);

      uint64_t d02 = w0 ^ rot64(w2 + d, s1);
      uint64_t c13 = w1 ^ rot64(w3 + c, s1);
      c += a ^ rot64(w0, s0);
      d -= b ^ rot64(w1, s2);
      a ^= p1 * (d02 + w3);
      b ^= p0 * (c13 + w2);
      data = (const uint64_t *)data + 4;
    } while (likely(data < detent));

    a ^= p6 * (rot64(c, s1) + d);
    b ^= p5 * (c + rot64(d, s1));
    len &= 31;
  }

  const uint64_t *v = (const uint64_t *)data;
  if (unlikely(need_align) && len > 8)
    v = (const uint64_t *)memcpy(&align, v, len);

  switch (len) {
  default:
    b += mux64(fetch64_be(v++), p4);
  case 24:
  case 23:
  case 22:
  case 21:
  case 20:
  case 19:
  case 18:
  case 17:
    a += mux64(fetch64_be(v++), p3);
  case 16:
  case 15:
  case 14:
  case 13:
  case 12:
  case 11:
  case 10:
  case 9:
    b += mux64(fetch64_be(v++), p2);
  case 8:
  case 7:
  case 6:
  case 5:
  case 4:
  case 3:
  case 2:
  case 1:
    a += mux64(tail64_be(v, len), p1);
  case 0:
    return mux64(rot64(a + b, s1), p4) + mix(a ^ b, p0);
  }
}

/***************************************************************************/

static __inline uint32_t tail32_le(const void *v, size_t tail) {
  const uint8_t *p = (const uint8_t *)v;
  uint32_t r = 0;
  switch (tail & 3) {
#if UNALIGNED_OK && __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
  /* For most CPUs this code is better when not needed
   * copying for alignment or byte reordering. */
  case 0:
    return fetch32_le(p);
  case 3:
    r = (uint32_t)p[2] << 16;
  case 2:
    return r + fetch16_le(p);
  case 1:
    return p[0];
#else
  /* For most CPUs this code is better than a
   * copying for alignment and/or byte reordering. */
  case 0:
    r += p[3];
    r <<= 8;
  case 3:
    r += p[2];
    r <<= 8;
  case 2:
    r += p[1];
    r <<= 8;
  case 1:
    return r + p[0];
#endif
  }
  unreachable();
}

static __inline uint32_t tail32_be(const void *v, size_t tail) {
  const uint8_t *p = (const uint8_t *)v;
  switch (tail & 3) {
#if UNALIGNED_OK && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
  /* For most CPUs this code is better when not needed
   * copying for alignment or byte reordering. */
  case 1:
    return p[0];
  case 2:
    return fetch16_be(p);
  case 3:
    return fetch16_be(p) << 8 | p[2];
  case 0:
    return fetch32_be(p);
#else
  /* For most CPUs this code is better than a
   * copying for alignment and/or byte reordering. */
  case 1:
    return p[0];
  case 2:
    return p[1] | (uint32_t)p[0] << 8;
  case 3:
    return p[2] | (uint32_t)p[1] << 8 | (uint32_t)p[0] << 16;
  case 0:
    return p[3] | (uint32_t)p[2] << 8 | (uint32_t)p[1] << 16 |
           (uint32_t)p[0] << 24;
#endif
  }
  unreachable();
}

static __inline uint64_t remix32(uint32_t a, uint32_t b) {
  a ^= rot32(b, 13);
  uint64_t l = a | (uint64_t)b << 32;
  l *= p0;
  l ^= l >> 41;
  return l;
}

static __inline void mixup32(uint32_t *a, uint32_t *b, uint32_t v, uint32_t p) {
  uint64_t l = mul_32x32_64(*b + v, p);
  *a ^= (uint32_t)l;
  *b += (uint32_t)(l >> 32);
}

/* 32-bit 'magic' primes */
static const uint32_t q0 = 0x92D78269;
static const uint32_t q1 = 0xCA9B4735;
static const uint32_t q2 = 0xA4ABA1C3;
static const uint32_t q3 = 0xF6499843;
static const uint32_t q4 = 0x86F0FD61;
static const uint32_t q5 = 0xCA2DA6FB;
static const uint32_t q6 = 0xC4BB3575;

uint64_t t1ha_32le(const void *data, size_t len, uint64_t seed) {
  uint32_t a = rot32((uint32_t)len, s1) + (uint32_t)seed;
  uint32_t b = (uint32_t)len ^ (uint32_t)(seed >> 32);

  const int need_align = (((uintptr_t)data) & 3) != 0 && !UNALIGNED_OK;
  uint32_t align[4];

  if (unlikely(len > 16)) {
    uint32_t c = ~a;
    uint32_t d = rot32(b, 5);
    const void *detent = (const uint8_t *)data + len - 15;
    do {
      const uint32_t *v = (const uint32_t *)data;
      if (unlikely(need_align))
        v = (const uint32_t *)memcpy(&align, v, 16);

      uint32_t w0 = fetch32_le(v + 0);
      uint32_t w1 = fetch32_le(v + 1);
      uint32_t w2 = fetch32_le(v + 2);
      uint32_t w3 = fetch32_le(v + 3);

      uint32_t c02 = w0 ^ rot32(w2 + c, 11);
      uint32_t d13 = w1 + rot32(w3 + d, s1);
      c ^= rot32(b + w1, 7);
      d ^= rot32(a + w0, 3);
      b = q1 * (c02 + w3);
      a = q0 * (d13 ^ w2);

      data = (const uint32_t *)data + 4;
    } while (likely(data < detent));

    c += a;
    d += b;
    a ^= q6 * (rot32(c, 16) + d);
    b ^= q5 * (c + rot32(d, 16));

    len &= 15;
  }

  const uint8_t *v = (const uint8_t *)data;
  if (unlikely(need_align) && len > 4)
    v = (const uint8_t *)memcpy(&align, v, len);

  switch (len) {
  default:
    mixup32(&a, &b, fetch32_le(v), q4);
    v += 4;
  case 12:
  case 11:
  case 10:
  case 9:
    mixup32(&b, &a, fetch32_le(v), q3);
    v += 4;
  case 8:
  case 7:
  case 6:
  case 5:
    mixup32(&a, &b, fetch32_le(v), q2);
    v += 4;
  case 4:
  case 3:
  case 2:
  case 1:
    mixup32(&b, &a, tail32_le(v, len), q1);
  case 0:
    return remix32(a, b);
  }
}

uint64_t t1ha_32be(const void *data, size_t len, uint64_t seed) {
  uint32_t a = rot32((uint32_t)len, s1) + (uint32_t)seed;
  uint32_t b = (uint32_t)len ^ (uint32_t)(seed >> 32);

  const int need_align = (((uintptr_t)data) & 3) != 0 && !UNALIGNED_OK;
  uint32_t align[4];

  if (unlikely(len > 16)) {
    uint32_t c = ~a;
    uint32_t d = rot32(b, 5);
    const void *detent = (const uint8_t *)data + len - 15;
    do {
      const uint32_t *v = (const uint32_t *)data;
      if (unlikely(need_align))
        v = (const uint32_t *)memcpy(&align, v, 16);

      uint32_t w0 = fetch32_be(v + 0);
      uint32_t w1 = fetch32_be(v + 1);
      uint32_t w2 = fetch32_be(v + 2);
      uint32_t w3 = fetch32_be(v + 3);

      uint32_t c02 = w0 ^ rot32(w2 + c, 11);
      uint32_t d13 = w1 + rot32(w3 + d, s1);
      c ^= rot32(b + w1, 7);
      d ^= rot32(a + w0, 3);
      b = q1 * (c02 + w3);
      a = q0 * (d13 ^ w2);

      data = (const uint32_t *)data + 4;
    } while (likely(data < detent));

    c += a;
    d += b;
    a ^= q6 * (rot32(c, 16) + d);
    b ^= q5 * (c + rot32(d, 16));

    len &= 15;
  }

  const uint8_t *v = (const uint8_t *)data;
  if (unlikely(need_align) && len > 4)
    v = (const uint8_t *)memcpy(&align, v, len);

  switch (len) {
  default:
    mixup32(&a, &b, fetch32_be(v), q4);
    v += 4;
  case 12:
  case 11:
  case 10:
  case 9:
    mixup32(&b, &a, fetch32_be(v), q3);
    v += 4;
  case 8:
  case 7:
  case 6:
  case 5:
    mixup32(&a, &b, fetch32_be(v), q2);
    v += 4;
  case 4:
  case 3:
  case 2:
  case 1:
    mixup32(&b, &a, tail32_be(v, len), q1);
  case 0:
    return remix32(a, b);
  }
}

/***************************************************************************/

#if (defined(__x86_64__) && (defined(__SSE4_2__) || __GNUC_PREREQ(4, 4) ||     \
                             __has_attribute(target))) ||                      \
    defined(_M_X64) || defined(_X86_64_)
#include <nmmintrin.h>

uint64_t
#if __GNUC_PREREQ(4, 4) || __has_attribute(target)
    __attribute__((target("sse4.2")))
#endif
    t1ha_ia32crc(const void *data, size_t len, uint64_t seed) {
  uint64_t a = seed;
  uint64_t b = len;
  const uint64_t *v = (const uint64_t *)data;

  if (unlikely(len > 32)) {
    const void *detent = (const uint8_t *)data + len - 31;
    uint32_t x = (uint32_t)b;
    uint32_t y = (uint32_t)a;
    uint32_t z = (uint32_t)(~a ^ b);

    do {
      uint32_t t = (uint32_t)a + x;
      a = rot64(a, 17) + p0 * v[0];
      x += (uint32_t)_mm_crc32_u64(y, v[1]);
      y += (uint32_t)_mm_crc32_u64(z, v[2]);
      z += (uint32_t)_mm_crc32_u64(t, v[3]);
      v += 4;
    } while (likely(detent > (const void *)v));

    a ^= x * p5 + y * p6 + z;
    b = x + y * p5 + z * p6;
    len &= 31;
  }

  switch (len) {
  default:
    b += mux64(*v++, p4);
  case 24:
  case 23:
  case 22:
  case 21:
  case 20:
  case 19:
  case 18:
  case 17:
    a += mux64(*v++, p3);
  case 16:
  case 15:
  case 14:
  case 13:
  case 12:
  case 11:
  case 10:
  case 9:
    b += mux64(*v++, p2);
  case 8:
  case 7:
  case 6:
  case 5:
  case 4:
  case 3:
  case 2:
  case 1:
    a += mux64(tail64_le(v, len), p1);
  case 0:
    return mux64(rot64(a + b, s1), p4) + mix(a ^ b, p0);
  }
}

#endif /* __x86_64__ */

/***************************************************************************/

#if defined(__x86_64__) || defined(_M_IX86) || defined(_M_X64) ||              \
    defined(i386) || defined(_X86_) || defined(__i386__) || defined(_X86_64_)
static uint32_t x86_cpu_features(void) {
#ifdef __GNUC__
  uint32_t eax, ebx, ecx, edx;
  if (__get_cpuid_max(0, NULL) < 1)
    return 0;
  __cpuid_count(1, 0, eax, ebx, ecx, edx);
  return ecx;
#elif defined(_MSC_VER)
  int info[4];
  __cpuid(info, 0);
  if (info[0] < 1)
    return 0;
  __cpuidex(info, 1, 0);
  return info[2];
#else
  return 0;
#endif
}
#endif

#if defined(__i386__) || defined(__x86_64__) || defined(_M_IX86) ||            \
    defined(_M_X64) || defined(i386) || defined(_X86_) || defined(_X86_64_)
#include <emmintrin.h>
#include <smmintrin.h>
#include <wmmintrin.h>

#if defined(__x86_64__) && defined(__ELF__) &&                                 \
    (__GNUC_PREREQ(4, 6) || __has_attribute(ifunc)) && __has_attribute(target)
uint64_t t1ha_ia32aes(const void *data, size_t len, uint64_t seed)
    __attribute__((ifunc("t1ha_aes_resolve")));

static uint64_t t1ha_ia32aes_avx(const void *data, size_t len, uint64_t seed);
static uint64_t t1ha_ia32aes_noavx(const void *data, size_t len, uint64_t seed);

static uint64_t (*t1ha_aes_resolve(void))(const void *, size_t, uint64_t) {
  uint32_t features = x86_cpu_features();
  if ((features & 0x01A000000) == 0x01A000000)
    return t1ha_ia32aes_avx;
  return t1ha_ia32aes_noavx;
}

static uint64_t __attribute__((target("aes,avx")))
t1ha_ia32aes_avx(const void *data, size_t len, uint64_t seed) {
  uint64_t a = seed;
  uint64_t b = len;

  if (unlikely(len > 32)) {
    __m128i x = _mm_set_epi64x(a, b);
    __m128i y = _mm_aesenc_si128(x, _mm_set_epi64x(p0, p1));

    const __m128i *v = (const __m128i *)data;
    const __m128i *const detent =
        (const __m128i *)((const uint8_t *)data + (len & ~15ul));
    data = detent;

    if (len & 16) {
      x = _mm_add_epi64(x, _mm_loadu_si128(v++));
      y = _mm_aesenc_si128(x, y);
    }
    len &= 15;

    if (v + 7 < detent) {
      __m128i salt = y;
      do {
        __m128i t = _mm_aesenc_si128(_mm_loadu_si128(v++), salt);
        t = _mm_aesdec_si128(t, _mm_loadu_si128(v++));
        t = _mm_aesdec_si128(t, _mm_loadu_si128(v++));
        t = _mm_aesdec_si128(t, _mm_loadu_si128(v++));

        t = _mm_aesdec_si128(t, _mm_loadu_si128(v++));
        t = _mm_aesdec_si128(t, _mm_loadu_si128(v++));
        t = _mm_aesdec_si128(t, _mm_loadu_si128(v++));
        t = _mm_aesdec_si128(t, _mm_loadu_si128(v++));

        salt = _mm_add_epi64(salt, _mm_set_epi64x(p2, p3));
        t = _mm_aesenc_si128(x, t);
        x = _mm_add_epi64(y, x);
        y = t;
      } while (v + 7 < detent);
    }

    while (v < detent) {
      __m128i v0y = _mm_add_epi64(y, _mm_loadu_si128(v++));
      __m128i v1x = _mm_sub_epi64(x, _mm_loadu_si128(v++));
      x = _mm_aesdec_si128(x, v0y);
      y = _mm_aesdec_si128(y, v1x);
    }

    x = _mm_add_epi64(_mm_aesdec_si128(x, _mm_aesenc_si128(y, x)), y);
#if defined(__x86_64__) || defined(_M_X64)
    a = _mm_cvtsi128_si64(x);
#if defined(__SSE4_1__)
    b = _mm_extract_epi64(x, 1);
#else
    b = _mm_cvtsi128_si64(_mm_unpackhi_epi64(x, x));
#endif
#else
    a = (uint32_t)_mm_cvtsi128_si32(x);
#if defined(__SSE4_1__)
    a |= (uint64_t)_mm_extract_epi32(x, 1) << 32;
    b = (uint32_t)_mm_extract_epi32(x, 2) |
        (uint64_t)_mm_extract_epi32(x, 3) << 32;
#else
    a |= (uint64_t)_mm_cvtsi128_si32(_mm_shuffle_epi32(x, 1)) << 32;
    x = _mm_unpackhi_epi64(x, x);
    b = (uint32_t)_mm_cvtsi128_si32(x);
    b |= (uint64_t)_mm_cvtsi128_si32(_mm_shuffle_epi32(x, 1)) << 32;
#endif
#endif
  }

  const uint64_t *v = (const uint64_t *)data;
  switch (len) {
  default:
    b += mux64(*v++, p4);
  case 24:
  case 23:
  case 22:
  case 21:
  case 20:
  case 19:
  case 18:
  case 17:
    a += mux64(*v++, p3);
  case 16:
  case 15:
  case 14:
  case 13:
  case 12:
  case 11:
  case 10:
  case 9:
    b += mux64(*v++, p2);
  case 8:
  case 7:
  case 6:
  case 5:
  case 4:
  case 3:
  case 2:
  case 1:
    a += mux64(tail64_le(v, len), p1);
  case 0:
    return mux64(rot64(a + b, s1), p4) + mix(a ^ b, p0);
  }
}

static uint64_t
#if __GNUC_PREREQ(4, 4) || __has_attribute(target)
    __attribute__((target("aes")))
#endif
    t1ha_ia32aes_noavx(const void *data, size_t len, uint64_t seed) {

#else /* ELF && ifunc */

uint64_t
#if __GNUC_PREREQ(4, 4) || __has_attribute(target)
    __attribute__((target("aes")))
#endif
    t1ha_ia32aes(const void *data, size_t len, uint64_t seed) {
#endif
  uint64_t a = seed;
  uint64_t b = len;

  if (unlikely(len > 32)) {
    __m128i x = _mm_set_epi64x(a, b);
    __m128i y = _mm_aesenc_si128(x, _mm_set_epi64x(p0, p1));

    const __m128i *v = (const __m128i *)data;
    const __m128i *const detent =
        (const __m128i *)((const uint8_t *)data + (len & ~15ul));
    data = detent;

    if (len & 16) {
      x = _mm_add_epi64(x, _mm_loadu_si128(v++));
      y = _mm_aesenc_si128(x, y);
    }
    len &= 15;

    if (v + 7 < detent) {
      __m128i salt = y;
      do {
        __m128i t = _mm_aesenc_si128(_mm_loadu_si128(v++), salt);
        t = _mm_aesdec_si128(t, _mm_loadu_si128(v++));
        t = _mm_aesdec_si128(t, _mm_loadu_si128(v++));
        t = _mm_aesdec_si128(t, _mm_loadu_si128(v++));

        t = _mm_aesdec_si128(t, _mm_loadu_si128(v++));
        t = _mm_aesdec_si128(t, _mm_loadu_si128(v++));
        t = _mm_aesdec_si128(t, _mm_loadu_si128(v++));
        t = _mm_aesdec_si128(t, _mm_loadu_si128(v++));

        salt = _mm_add_epi64(salt, _mm_set_epi64x(p2, p3));
        t = _mm_aesenc_si128(x, t);
        x = _mm_add_epi64(y, x);
        y = t;
      } while (v + 7 < detent);
    }

    while (v < detent) {
      __m128i v0y = _mm_add_epi64(y, _mm_loadu_si128(v++));
      __m128i v1x = _mm_sub_epi64(x, _mm_loadu_si128(v++));
      x = _mm_aesdec_si128(x, v0y);
      y = _mm_aesdec_si128(y, v1x);
    }

    x = _mm_add_epi64(_mm_aesdec_si128(x, _mm_aesenc_si128(y, x)), y);
#if defined(__x86_64__) || defined(_M_X64)
    a = _mm_cvtsi128_si64(x);
#if defined(__SSE4_1__)
    b = _mm_extract_epi64(x, 1);
#else
    b = _mm_cvtsi128_si64(_mm_unpackhi_epi64(x, x));
#endif
#else
    a = (uint32_t)_mm_cvtsi128_si32(x);
#if defined(__SSE4_1__)
    a |= (uint64_t)_mm_extract_epi32(x, 1) << 32;
    b = (uint32_t)_mm_extract_epi32(x, 2) |
        (uint64_t)_mm_extract_epi32(x, 3) << 32;
#else
    a |= (uint64_t)_mm_cvtsi128_si32(_mm_shuffle_epi32(x, 1)) << 32;
    x = _mm_unpackhi_epi64(x, x);
    b = (uint32_t)_mm_cvtsi128_si32(x);
    b |= (uint64_t)_mm_cvtsi128_si32(_mm_shuffle_epi32(x, 1)) << 32;
#endif
#endif
  }

  const uint64_t *v = (const uint64_t *)data;
  switch (len) {
  default:
    b += mux64(*v++, p4);
  case 24:
  case 23:
  case 22:
  case 21:
  case 20:
  case 19:
  case 18:
  case 17:
    a += mux64(*v++, p3);
  case 16:
  case 15:
  case 14:
  case 13:
  case 12:
  case 11:
  case 10:
  case 9:
    b += mux64(*v++, p2);
  case 8:
  case 7:
  case 6:
  case 5:
  case 4:
  case 3:
  case 2:
  case 1:
    a += mux64(tail64_le(v, len), p1);
  case 0:
    return mux64(rot64(a + b, s1), p4) + mix(a ^ b, p0);
  }
}

#endif /* __i386__ || __x86_64__ */

/***************************************************************************/

static uint64_t (*t1ha_local_resolve(void))(const void *, size_t, uint64_t) {
#if defined(__x86_64) || defined(_M_IX86) || defined(_M_X64) ||                \
    defined(i386) || defined(_X86_) || defined(__i386__) || defined(_X86_64_)

  uint32_t features = x86_cpu_features();
  if (features & (1l << 25))
    return t1ha_ia32aes;

#if defined(__x86_64) || defined(_M_X64) || defined(_X86_64_)
  if (features & (1l << 20))
    return t1ha_ia32crc;
#endif

#endif /* x86 */

#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
  return (sizeof(long) >= 8) ? t1ha_64be : t1ha_32be;
#else
  return (sizeof(long) >= 8) ? t1ha_64le : t1ha_32le;
#endif
}

#if defined(__ELF__) && (__GNUC_PREREQ(4, 6) || __has_attribute(ifunc))

uint64_t t1ha_local(const void *data, size_t len, uint64_t seed)
    __attribute__((ifunc("t1ha_local_resolve")));

#elif __GNUC_PREREQ(4, 0) || __has_attribute(constructor)

uint64_t (*t1ha_local_ptr)(const void *, size_t, uint64_t);

static void __attribute__((constructor)) t1ha_local_init(void) {
  t1ha_local_ptr = t1ha_local_resolve();
}

#else /* ELF && ifunc */

static uint64_t t1ha_local_proxy(const void *data, size_t len, uint64_t seed) {
  t1ha_local_ptr = t1ha_local_resolve();
  return t1ha_local_ptr(data, len, seed);
}

uint64_t (*t1ha_local_ptr)(const void *, size_t, uint64_t) = t1ha_local_proxy;

#endif
