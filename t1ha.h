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
 * The Future will Positive. Всё будет хорошо.
 *
 * ACKNOWLEDGEMENT:
 * The t1ha was originally developed by Leonid Yuriev (Леонид Юрьев)
 * for The 1Hippeus project - zerocopy messaging in the spirit of Sparta!
 */

#pragma once
#include <stddef.h>
#include <stdint.h>

#ifndef __has_attribute
#define __has_attribute(x) (0)
#endif

#ifndef __GNUC_PREREQ
#if defined(__GNUC__) && defined(__GNUC_MINOR__)
#define __GNUC_PREREQ(maj, min)                                                \
  ((__GNUC__ << 16) + __GNUC_MINOR__ >= ((maj) << 16) + (min))
#else
#define __GNUC_PREREQ(maj, min) 0
#endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* The main generic version of "Fast Positive Hash".
 *  - returns same result on all architectures and CPUs.
 *  - created for 64-bit little-endian platforms,
 *    in other cases may runs slowly. */
uint64_t t1ha(const void *data, size_t len, uint64_t seed);

/* The big-endian version.
 *  - runs faster on 64-bit big-endian platforms,
 *    in other cases may runs slowly.
 *  - returns same result on all architectures and CPUs,
 *    but it is differs from t1ha(). */
uint64_t t1ha_64be(const void *data, size_t len, uint64_t seed);

/* Just a nickname for the generic t1ha.
 * 't1ha_64le' mean that is for 64-bit little-endian platforms. */
static __inline uint64_t t1ha_64le(const void *data, size_t len,
                                   uint64_t seed) {
  return t1ha(data, len, seed);
}

/* The alternative little/big-endian versions, which were
 * designed for a 32-bit CPUs. In comparison to the main t1ha:
 *   - much faster on 32-bit architectures.
 *   - half of speed on 64-bit architectures.
 *   - useful in case primary target platform is 32-bit. */
uint64_t t1ha_32le(const void *data, size_t len, uint64_t seed);
uint64_t t1ha_32be(const void *data, size_t len, uint64_t seed);

#if (defined(__x86_64__) && (defined(__SSE4_2__) || __GNUC_PREREQ(4, 4) ||     \
                             __has_attribute(target))) ||                      \
    defined(_M_X64) || defined(_X86_64_)
/* Machine specific hash, which uses CRC32c hardware acceleration.
 * Available only on modern x86 CPUs with support for SSE 4.2. */
uint64_t t1ha_ia32crc(const void *data, size_t len, uint64_t seed);
#endif

#if ((defined(__AES__) || __GNUC_PREREQ(4, 4) || __has_attribute(target)) &&   \
     (defined(__x86_64__) || defined(__i386__))) ||                            \
    defined(_M_X64) || defined(_M_IX86)
/* Machine specific hash, which uses AES hardware acceleration.
 * Available only on modern x86 CPUs with AES-NI extension. */
uint64_t t1ha_ia32aes(const void *data, size_t len, uint64_t seed);
#endif /* __AES__ */

/* Machine-specific facade that selects the fastest hash for
 * the current processor.
 *
 * BE CAREFUL!!! This is mean that hash result could be differ,
 * when someone (CPU, BIOS, OS, compiler, source code) was changed.
 * Briefly, such hash-results and their derivatives, should
 * not be persist or transferred over a network. */
#if defined(__ELF__) && (__GNUC_PREREQ(4, 6) || __has_attribute(ifunc))
uint64_t t1ha_local(const void *data, size_t len, uint64_t seed);
#else
extern uint64_t (*t1ha_local_ptr)(const void *data, size_t len, uint64_t seed);
static __inline uint64_t t1ha_local(const void *data, size_t len,
                                    uint64_t seed) {
  return t1ha_local_ptr(data, len, seed);
}
#endif

#ifdef __cplusplus
}
#endif
