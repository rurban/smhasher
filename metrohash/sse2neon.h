#ifndef SSE2NEON_H
#define SSE2NEON_H

// This header file provides a simple API translation layer
// between SSE intrinsics to their corresponding Arm/Aarch64 NEON versions
//
// This header file does not yet translate all of the SSE intrinsics.
//
// Contributors to this work are:
//   John W. Ratcliff <jratcliffscarab@gmail.com>
//   Brandon Rowlett <browlett@nvidia.com>
//   Ken Fast <kfast@gdeb.com>
//   Eric van Beurden <evanbeurden@nvidia.com>
//   Alexander Potylitsin <apotylitsin@nvidia.com>
//   Hasindu Gamaarachchi <hasindu2008@gmail.com>
//   Jim Huang <jserv@biilabs.io>
//   Mark Cheng <marktwtn@biilabs.io>
//   Malcolm James MacLeod <malcolm@gulden.com>
//   Devin Hussey (easyaspi314) <husseydevin@gmail.com>
//   Sebastian Pop <spop@amazon.com>
//   Developer Ecosystem Engineering <DeveloperEcosystemEngineering@apple.com>
//   Danila Kutenin <danilak@google.com>
//   Franรงois Turban (JishinMaster) <francois.turban@gmail.com>
//   Pei-Hsuan Hung <afcidk@gmail.com>
//   Yang-Hao Yuan <yanghau@biilabs.io>

/*
 * sse2neon is freely redistributable under the MIT License.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/* Tunable configurations */

/* Enable precise implementation of _mm_min_ps and _mm_max_ps
 * This would slow down the computation a bit, but gives consistent result with
 * x86 SSE2. (e.g. would solve a hole or NaN pixel in the rendering result)
 */
#ifndef SSE2NEON_PRECISE_MINMAX
#define SSE2NEON_PRECISE_MINMAX (0)
#endif

#if defined(__GNUC__) || defined(__clang__)
#pragma push_macro("FORCE_INLINE")
#pragma push_macro("ALIGN_STRUCT")
#define FORCE_INLINE static inline __attribute__((always_inline))
#define ALIGN_STRUCT(x) __attribute__((aligned(x)))
#else
#error "Macro name collisions may happen with unsupported compiler."
#ifdef FORCE_INLINE
#undef FORCE_INLINE
#endif
#define FORCE_INLINE static inline
#ifndef ALIGN_STRUCT
#define ALIGN_STRUCT(x) __declspec(align(x))
#endif
#endif

#include <stdint.h>
#include <stdlib.h>

/* Architecture-specific build options */
/* FIXME: #pragma GCC push_options is only available on GCC */
#if defined(__GNUC__)
#if defined(__arm__) && __ARM_ARCH == 7
/* According to ARM C Language Extensions Architecture specification,
 * __ARM_NEON is defined to a value indicating the Advanced SIMD (NEON)
 * architecture supported.
 */
#if !defined(__ARM_NEON) || !defined(__ARM_NEON__)
#error "You must enable NEON instructions (e.g. -mfpu=neon) to use SSE2NEON."
#endif
#pragma GCC push_options
#pragma GCC target("fpu=neon")
#elif defined(__aarch64__)
#pragma GCC push_options
#pragma GCC target("+simd")
#else
#error "Unsupported target. Must be either ARMv7-A+NEON or ARMv8-A."
#endif
#endif

#include <arm_neon.h>

/* Rounding functions require either Aarch64 instructions or libm failback */
#if !defined(__aarch64__)
#include <math.h>
#endif

/* "__has_builtin" can be used to query support for built-in functions
 * provided by gcc/clang and other compilers that support it.
 */
#ifndef __has_builtin /* GCC prior to 10 or non-clang compilers */
/* Compatibility with gcc <= 9 */
#if __GNUC__ <= 9
#define __has_builtin(x) HAS##x
#define HAS__builtin_popcount 1
#define HAS__builtin_popcountll 1
#else
#define __has_builtin(x) 0
#endif
#endif

/**
 * MACRO for shuffle parameter for _mm_shuffle_ps().
 * Argument fp3 is a digit[0123] that represents the fp from argument "b"
 * of mm_shuffle_ps that will be placed in fp3 of result. fp2 is the same
 * for fp2 in result. fp1 is a digit[0123] that represents the fp from
 * argument "a" of mm_shuffle_ps that will be places in fp1 of result.
 * fp0 is the same for fp0 of result.
 */
#define _MM_SHUFFLE(fp3, fp2, fp1, fp0) \
    (((fp3) << 6) | ((fp2) << 4) | ((fp1) << 2) | ((fp0)))

/* Rounding mode macros. */
#define _MM_FROUND_TO_NEAREST_INT 0x00
#define _MM_FROUND_TO_NEG_INF 0x01
#define _MM_FROUND_TO_POS_INF 0x02
#define _MM_FROUND_TO_ZERO 0x03
#define _MM_FROUND_CUR_DIRECTION 0x04
#define _MM_FROUND_NO_EXC 0x08

/* indicate immediate constant argument in a given range */
#define __constrange(a, b) const

/* A few intrinsics accept traditional data types like ints or floats, but
 * most operate on data types that are specific to SSE.
 * If a vector type ends in d, it contains doubles, and if it does not have
 * a suffix, it contains floats. An integer vector type can contain any type
 * of integer, from chars to shorts to unsigned long longs.
 */
typedef int64x1_t __m64;
typedef float32x4_t __m128; /* 128-bit vector containing 4 floats */
// On ARM 32-bit architecture, the float64x2_t is not supported.
// The data type __m128d should be represented in a different way for related
// intrinsic conversion.
#if defined(__aarch64__)
typedef float64x2_t __m128d; /* 128-bit vector containing 2 doubles */
#else
typedef float32x4_t __m128d;
#endif
typedef int64x2_t __m128i; /* 128-bit vector containing integers */

/* type-safe casting between types */

#define vreinterpretq_m128_f16(x) vreinterpretq_f32_f16(x)
#define vreinterpretq_m128_f32(x) (x)
#define vreinterpretq_m128_f64(x) vreinterpretq_f32_f64(x)

#define vreinterpretq_m128_u8(x) vreinterpretq_f32_u8(x)
#define vreinterpretq_m128_u16(x) vreinterpretq_f32_u16(x)
#define vreinterpretq_m128_u32(x) vreinterpretq_f32_u32(x)
#define vreinterpretq_m128_u64(x) vreinterpretq_f32_u64(x)

#define vreinterpretq_m128_s8(x) vreinterpretq_f32_s8(x)
#define vreinterpretq_m128_s16(x) vreinterpretq_f32_s16(x)
#define vreinterpretq_m128_s32(x) vreinterpretq_f32_s32(x)
#define vreinterpretq_m128_s64(x) vreinterpretq_f32_s64(x)

#define vreinterpretq_f16_m128(x) vreinterpretq_f16_f32(x)
#define vreinterpretq_f32_m128(x) (x)
#define vreinterpretq_f64_m128(x) vreinterpretq_f64_f32(x)

#define vreinterpretq_u8_m128(x) vreinterpretq_u8_f32(x)
#define vreinterpretq_u16_m128(x) vreinterpretq_u16_f32(x)
#define vreinterpretq_u32_m128(x) vreinterpretq_u32_f32(x)
#define vreinterpretq_u64_m128(x) vreinterpretq_u64_f32(x)

#define vreinterpretq_s8_m128(x) vreinterpretq_s8_f32(x)
#define vreinterpretq_s16_m128(x) vreinterpretq_s16_f32(x)
#define vreinterpretq_s32_m128(x) vreinterpretq_s32_f32(x)
#define vreinterpretq_s64_m128(x) vreinterpretq_s64_f32(x)

#define vreinterpretq_m128i_s8(x) vreinterpretq_s64_s8(x)
#define vreinterpretq_m128i_s16(x) vreinterpretq_s64_s16(x)
#define vreinterpretq_m128i_s32(x) vreinterpretq_s64_s32(x)
#define vreinterpretq_m128i_s64(x) (x)

#define vreinterpretq_m128i_u8(x) vreinterpretq_s64_u8(x)
#define vreinterpretq_m128i_u16(x) vreinterpretq_s64_u16(x)
#define vreinterpretq_m128i_u32(x) vreinterpretq_s64_u32(x)
#define vreinterpretq_m128i_u64(x) vreinterpretq_s64_u64(x)

#define vreinterpretq_s8_m128i(x) vreinterpretq_s8_s64(x)
#define vreinterpretq_s16_m128i(x) vreinterpretq_s16_s64(x)
#define vreinterpretq_s32_m128i(x) vreinterpretq_s32_s64(x)
#define vreinterpretq_s64_m128i(x) (x)

#define vreinterpretq_u8_m128i(x) vreinterpretq_u8_s64(x)
#define vreinterpretq_u16_m128i(x) vreinterpretq_u16_s64(x)
#define vreinterpretq_u32_m128i(x) vreinterpretq_u32_s64(x)
#define vreinterpretq_u64_m128i(x) vreinterpretq_u64_s64(x)

#define vreinterpret_m64_s8(x) vreinterpret_s64_s8(x)
#define vreinterpret_m64_s16(x) vreinterpret_s64_s16(x)
#define vreinterpret_m64_s32(x) vreinterpret_s64_s32(x)
#define vreinterpret_m64_s64(x) (x)

#define vreinterpret_m64_u8(x) vreinterpret_s64_u8(x)
#define vreinterpret_m64_u16(x) vreinterpret_s64_u16(x)
#define vreinterpret_m64_u32(x) vreinterpret_s64_u32(x)
#define vreinterpret_m64_u64(x) vreinterpret_s64_u64(x)

#define vreinterpret_m64_f16(x) vreinterpret_s64_f16(x)
#define vreinterpret_m64_f32(x) vreinterpret_s64_f32(x)
#define vreinterpret_m64_f64(x) vreinterpret_s64_f64(x)

#define vreinterpret_u8_m64(x) vreinterpret_u8_s64(x)
#define vreinterpret_u16_m64(x) vreinterpret_u16_s64(x)
#define vreinterpret_u32_m64(x) vreinterpret_u32_s64(x)
#define vreinterpret_u64_m64(x) vreinterpret_u64_s64(x)

#define vreinterpret_s8_m64(x) vreinterpret_s8_s64(x)
#define vreinterpret_s16_m64(x) vreinterpret_s16_s64(x)
#define vreinterpret_s32_m64(x) vreinterpret_s32_s64(x)
#define vreinterpret_s64_m64(x) (x)

#define vreinterpret_f32_m64(x) vreinterpret_f32_s64(x)

#if defined(__aarch64__)
#define vreinterpretq_m128d_s32(x) vreinterpretq_f64_s32(x)
#define vreinterpretq_m128d_s64(x) vreinterpretq_f64_s64(x)

#define vreinterpretq_m128d_f64(x) (x)

#define vreinterpretq_s64_m128d(x) vreinterpretq_s64_f64(x)

#define vreinterpretq_f64_m128d(x) (x)
#else
#define vreinterpretq_m128d_s32(x) vreinterpretq_f32_s32(x)
#define vreinterpretq_m128d_s64(x) vreinterpretq_f32_s64(x)

#define vreinterpretq_m128d_f32(x) (x)

#define vreinterpretq_s64_m128d(x) vreinterpretq_s64_f32(x)

#define vreinterpretq_f32_m128d(x) (x)
#endif

// A struct is defined in this header file called 'SIMDVec' which can be used
// by applications which attempt to access the contents of an _m128 struct
// directly.  It is important to note that accessing the __m128 struct directly
// is bad coding practice by Microsoft: @see:
// https://msdn.microsoft.com/en-us/library/ayeb3ayc.aspx
//
// However, some legacy source code may try to access the contents of an __m128
// struct directly so the developer can use the SIMDVec as an alias for it.  Any
// casting must be done manually by the developer, as you cannot cast or
// otherwise alias the base NEON data type for intrinsic operations.
//
// union intended to allow direct access to an __m128 variable using the names
// that the MSVC compiler provides.  This union should really only be used when
// trying to access the members of the vector as integer values.  GCC/clang
// allow native access to the float members through a simple array access
// operator (in C since 4.6, in C++ since 4.8).
//
// Ideally direct accesses to SIMD vectors should not be used since it can cause
// a performance hit.  If it really is needed however, the original __m128
// variable can be aliased with a pointer to this union and used to access
// individual components.  The use of this union should be hidden behind a macro
// that is used throughout the codebase to access the members instead of always
// declaring this type of variable.
typedef union ALIGN_STRUCT(16) SIMDVec {
    float m128_f32[4];     // as floats - DON'T USE. Added for convenience.
    int8_t m128_i8[16];    // as signed 8-bit integers.
    int16_t m128_i16[8];   // as signed 16-bit integers.
    int32_t m128_i32[4];   // as signed 32-bit integers.
    int64_t m128_i64[2];   // as signed 64-bit integers.
    uint8_t m128_u8[16];   // as unsigned 8-bit integers.
    uint16_t m128_u16[8];  // as unsigned 16-bit integers.
    uint32_t m128_u32[4];  // as unsigned 32-bit integers.
    uint64_t m128_u64[2];  // as unsigned 64-bit integers.
} SIMDVec;

// casting using SIMDVec
#define vreinterpretq_nth_u64_m128i(x, n) (((SIMDVec *) &x)->m128_u64[n])
#define vreinterpretq_nth_u32_m128i(x, n) (((SIMDVec *) &x)->m128_u32[n])
#define vreinterpretq_nth_u8_m128i(x, n) (((SIMDVec *) &x)->m128_u8[n])

/* Backwards compatibility for compilers with lack of specific type support */

// Older gcc does not define vld1q_u8_x4 type
#if defined(__GNUC__) && !defined(__clang__)
#if __GNUC__ <= 9
FORCE_INLINE uint8x16x4_t vld1q_u8_x4(const uint8_t *p)
{
    uint8x16x4_t ret;
    ret.val[0] = vld1q_u8(p + 0);
    ret.val[1] = vld1q_u8(p + 16);
    ret.val[2] = vld1q_u8(p + 32);
    ret.val[3] = vld1q_u8(p + 48);
    return ret;
}
#endif
#endif

/* Function Naming Conventions
 * The naming convention of SSE intrinsics is straightforward. A generic SSE
 * intrinsic function is given as follows:
 *   _mm_<name>_<data_type>
 *
 * The parts of this format are given as follows:
 * 1. <name> describes the operation performed by the intrinsic
 * 2. <data_type> identifies the data type of the function's primary arguments
 *
 * This last part, <data_type>, is a little complicated. It identifies the
 * content of the input values, and can be set to any of the following values:
 * + ps - vectors contain floats (ps stands for packed single-precision)
 * + pd - vectors cantain doubles (pd stands for packed double-precision)
 * + epi8/epi16/epi32/epi64 - vectors contain 8-bit/16-bit/32-bit/64-bit
 *                            signed integers
 * + epu8/epu16/epu32/epu64 - vectors contain 8-bit/16-bit/32-bit/64-bit
 *                            unsigned integers
 * + si128 - unspecified 128-bit vector or 256-bit vector
 * + m128/m128i/m128d - identifies input vector types when they are different
 *                      than the type of the returned vector
 *
 * For example, _mm_setzero_ps. The _mm implies that the function returns
 * a 128-bit vector. The _ps at the end implies that the argument vectors
 * contain floats.
 *
 * A complete example: Byte Shuffle - pshufb (_mm_shuffle_epi8)
 *   // Set packed 16-bit integers. 128 bits, 8 short, per 16 bits
 *   __m128i v_in = _mm_setr_epi16(1, 2, 3, 4, 5, 6, 7, 8);
 *   // Set packed 8-bit integers
 *   // 128 bits, 16 chars, per 8 bits
 *   __m128i v_perm = _mm_setr_epi8(1, 0,  2,  3, 8, 9, 10, 11,
 *                                  4, 5, 12, 13, 6, 7, 14, 15);
 *   // Shuffle packed 8-bit integers
 *   __m128i v_out = _mm_shuffle_epi8(v_in, v_perm); // pshufb
 *
 * Data (Number, Binary, Byte Index):
    +------+------+-------------+------+------+-------------+
    |      1      |      2      |      3      |      4      | Number
    +------+------+------+------+------+------+------+------+
    | 0000 | 0001 | 0000 | 0010 | 0000 | 0011 | 0000 | 0100 | Binary
    +------+------+------+------+------+------+------+------+
    |    0 |    1 |    2 |    3 |    4 |    5 |    6 |    7 | Index
    +------+------+------+------+------+------+------+------+

    +------+------+------+------+------+------+------+------+
    |      5      |      6      |      7      |      8      | Number
    +------+------+------+------+------+------+------+------+
    | 0000 | 0101 | 0000 | 0110 | 0000 | 0111 | 0000 | 1000 | Binary
    +------+------+------+------+------+------+------+------+
    |    8 |    9 |   10 |   11 |   12 |   13 |   14 |   15 | Index
    +------+------+------+------+------+------+------+------+
 * Index (Byte Index):
    +------+------+------+------+------+------+------+------+
    |    1 |    0 |    2 |    3 |    8 |    9 |   10 |   11 |
    +------+------+------+------+------+------+------+------+

    +------+------+------+------+------+------+------+------+
    |    4 |    5 |   12 |   13 |    6 |    7 |   14 |   15 |
    +------+------+------+------+------+------+------+------+
 * Result:
    +------+------+------+------+------+------+------+------+
    |    1 |    0 |    2 |    3 |    8 |    9 |   10 |   11 | Index
    +------+------+------+------+------+------+------+------+
    | 0001 | 0000 | 0000 | 0010 | 0000 | 0101 | 0000 | 0110 | Binary
    +------+------+------+------+------+------+------+------+
    |     256     |      2      |      5      |      6      | Number
    +------+------+------+------+------+------+------+------+

    +------+------+------+------+------+------+------+------+
    |    4 |    5 |   12 |   13 |    6 |    7 |   14 |   15 | Index
    +------+------+------+------+------+------+------+------+
    | 0000 | 0011 | 0000 | 0111 | 0000 | 0100 | 0000 | 1000 | Binary
    +------+------+------+------+------+------+------+------+
    |      3      |      7      |      4      |      8      | Number
    +------+------+------+------+------+------+-------------+
 */

/* Set/get methods */

/* Constants for use with _mm_prefetch.  */
enum _mm_hint {
    _MM_HINT_NTA = 0,  /* load data to L1 and L2 cache, mark it as NTA */
    _MM_HINT_T0 = 1,   /* load data to L1 and L2 cache */
    _MM_HINT_T1 = 2,   /* load data to L2 cache only */
    _MM_HINT_T2 = 3,   /* load data to L2 cache only, mark it as NTA */
    _MM_HINT_ENTA = 4, /* exclusive version of _MM_HINT_NTA */
    _MM_HINT_ET0 = 5,  /* exclusive version of _MM_HINT_T0 */
    _MM_HINT_ET1 = 6,  /* exclusive version of _MM_HINT_T1 */
    _MM_HINT_ET2 = 7   /* exclusive version of _MM_HINT_T2 */
};

// Loads one cache line of data from address p to a location closer to the
// processor. https://msdn.microsoft.com/en-us/library/84szxsww(v=vs.100).aspx
FORCE_INLINE void _mm_prefetch(const void *p, int i)
{
    (void) i;
    __builtin_prefetch(p);
}

// Copy the lower single-precision (32-bit) floating-point element of a to dst.
//
//   dst[31:0] := a[31:0]
//
// https://software.intel.com/sites/landingpage/IntrinsicsGuide/#text=_mm_cvtss_f32
FORCE_INLINE float _mm_cvtss_f32(__m128 a)
{
    return vgetq_lane_f32(vreinterpretq_f32_m128(a), 0);
}

// Sets the 128-bit value to zero
// https://msdn.microsoft.com/en-us/library/vstudio/ys7dw0kh(v=vs.100).aspx
FORCE_INLINE __m128i _mm_setzero_si128(void)
{
    return vreinterpretq_m128i_s32(vdupq_n_s32(0));
}

// Clears the four single-precision, floating-point values.
// https://msdn.microsoft.com/en-us/library/vstudio/tk1t2tbz(v=vs.100).aspx
FORCE_INLINE __m128 _mm_setzero_ps(void)
{
    return vreinterpretq_m128_f32(vdupq_n_f32(0));
}

// Sets the four single-precision, floating-point values to w.
//
//   r0 := r1 := r2 := r3 := w
//
// https://msdn.microsoft.com/en-us/library/vstudio/2x1se8ha(v=vs.100).aspx
FORCE_INLINE __m128 _mm_set1_ps(float _w)
{
    return vreinterpretq_m128_f32(vdupq_n_f32(_w));
}

// Sets the four single-precision, floating-point values to w.
// https://msdn.microsoft.com/en-us/library/vstudio/2x1se8ha(v=vs.100).aspx
FORCE_INLINE __m128 _mm_set_ps1(float _w)
{
    return vreinterpretq_m128_f32(vdupq_n_f32(_w));
}

// Sets the four single-precision, floating-point values to the four inputs.
// https://msdn.microsoft.com/en-us/library/vstudio/afh0zf75(v=vs.100).aspx
FORCE_INLINE __m128 _mm_set_ps(float w, float z, float y, float x)
{
    float ALIGN_STRUCT(16) data[4] = {x, y, z, w};
    return vreinterpretq_m128_f32(vld1q_f32(data));
}

// Copy single-precision (32-bit) floating-point element a to the lower element
// of dst, and zero the upper 3 elements.
// https://software.intel.com/sites/landingpage/IntrinsicsGuide/#text=_mm_set_ss
FORCE_INLINE __m128 _mm_set_ss(float a)
{
    float ALIGN_STRUCT(16) data[4] = {a, 0, 0, 0};
    return vreinterpretq_m128_f32(vld1q_f32(data));
}

// Sets the four single-precision, floating-point values to the four inputs in
// reverse order.
// https://msdn.microsoft.com/en-us/library/vstudio/d2172ct3(v=vs.100).aspx
FORCE_INLINE __m128 _mm_setr_ps(float w, float z, float y, float x)
{
    float ALIGN_STRUCT(16) data[4] = {w, z, y, x};
    return vreinterpretq_m128_f32(vld1q_f32(data));
}

// Sets the 8 signed 16-bit integer values in reverse order.
//
// Return Value
//   r0 := w0
//   r1 := w1
//   ...
//   r7 := w7
FORCE_INLINE __m128i _mm_setr_epi16(short w0,
                                    short w1,
                                    short w2,
                                    short w3,
                                    short w4,
                                    short w5,
                                    short w6,
                                    short w7)
{
    int16_t ALIGN_STRUCT(16) data[8] = {w0, w1, w2, w3, w4, w5, w6, w7};
    return vreinterpretq_m128i_s16(vld1q_s16((int16_t *) data));
}

// Sets the 4 signed 32-bit integer values in reverse order
// https://technet.microsoft.com/en-us/library/security/27yb3ee5(v=vs.90).aspx
FORCE_INLINE __m128i _mm_setr_epi32(int i3, int i2, int i1, int i0)
{
    int32_t ALIGN_STRUCT(16) data[4] = {i3, i2, i1, i0};
    return vreinterpretq_m128i_s32(vld1q_s32(data));
}

// Set packed 64-bit integers in dst with the supplied values in reverse order.
// https://software.intel.com/sites/landingpage/IntrinsicsGuide/#text=_mm_setr_epi64
FORCE_INLINE __m128i _mm_setr_epi64(__m64 e1, __m64 e0)
{
    return vreinterpretq_m128i_s64(vcombine_s64(e1, e0));
}

// Sets the 16 signed 8-bit integer values to b.
//
//   r0 := b
//   r1 := b
//   ...
//   r15 := b
//
// https://msdn.microsoft.com/en-us/library/6e14xhyf(v=vs.100).aspx
FORCE_INLINE __m128i _mm_set1_epi8(signed char w)
{
    return vreinterpretq_m128i_s8(vdupq_n_s8(w));
}

// Sets the 8 signed 16-bit integer values to w.
//
//   r0 := w
//   r1 := w
//   ...
//   r7 := w
//
// https://msdn.microsoft.com/en-us/library/k0ya3x0e(v=vs.90).aspx
FORCE_INLINE __m128i _mm_set1_epi16(short w)
{
    return vreinterpretq_m128i_s16(vdupq_n_s16(w));
}

// Sets the 16 signed 8-bit integer values.
// https://msdn.microsoft.com/en-us/library/x0cx8zd3(v=vs.90).aspx
FORCE_INLINE __m128i _mm_set_epi8(signed char b15,
                                  signed char b14,
                                  signed char b13,
                                  signed char b12,
                                  signed char b11,
                                  signed char b10,
                                  signed char b9,
                                  signed char b8,
                                  signed char b7,
                                  signed char b6,
                                  signed char b5,
                                  signed char b4,
                                  signed char b3,
                                  signed char b2,
                                  signed char b1,
                                  signed char b0)
{
    int8_t ALIGN_STRUCT(16)
        data[16] = {(int8_t) b0,  (int8_t) b1,  (int8_t) b2,  (int8_t) b3,
                    (int8_t) b4,  (int8_t) b5,  (int8_t) b6,  (int8_t) b7,
                    (int8_t) b8,  (int8_t) b9,  (int8_t) b10, (int8_t) b11,
                    (int8_t) b12, (int8_t) b13, (int8_t) b14, (int8_t) b15};
    return (__m128i) vld1q_s8(data);
}

// Sets the 8 signed 16-bit integer values.
// https://msdn.microsoft.com/en-au/library/3e0fek84(v=vs.90).aspx
FORCE_INLINE __m128i _mm_set_epi16(short i7,
                                   short i6,
                                   short i5,
                                   short i4,
                                   short i3,
                                   short i2,
                                   short i1,
                                   short i0)
{
    int16_t ALIGN_STRUCT(16) data[8] = {i0, i1, i2, i3, i4, i5, i6, i7};
    return vreinterpretq_m128i_s16(vld1q_s16(data));
}

// Sets the 16 signed 8-bit integer values in reverse order.
// https://msdn.microsoft.com/en-us/library/2khb9c7k(v=vs.90).aspx
FORCE_INLINE __m128i _mm_setr_epi8(signed char b0,
                                   signed char b1,
                                   signed char b2,
                                   signed char b3,
                                   signed char b4,
                                   signed char b5,
                                   signed char b6,
                                   signed char b7,
                                   signed char b8,
                                   signed char b9,
                                   signed char b10,
                                   signed char b11,
                                   signed char b12,
                                   signed char b13,
                                   signed char b14,
                                   signed char b15)
{
    int8_t ALIGN_STRUCT(16)
        data[16] = {(int8_t) b0,  (int8_t) b1,  (int8_t) b2,  (int8_t) b3,
                    (int8_t) b4,  (int8_t) b5,  (int8_t) b6,  (int8_t) b7,
                    (int8_t) b8,  (int8_t) b9,  (int8_t) b10, (int8_t) b11,
                    (int8_t) b12, (int8_t) b13, (int8_t) b14, (int8_t) b15};
    return (__m128i) vld1q_s8(data);
}

// Sets the 4 signed 32-bit integer values to i.
//
//   r0 := i
//   r1 := i
//   r2 := i
//   r3 := I
//
// https://msdn.microsoft.com/en-us/library/vstudio/h4xscxat(v=vs.100).aspx
FORCE_INLINE __m128i _mm_set1_epi32(int _i)
{
    return vreinterpretq_m128i_s32(vdupq_n_s32(_i));
}

// Sets the 2 signed 64-bit integer values to i.
// https://docs.microsoft.com/en-us/previous-versions/visualstudio/visual-studio-2010/whtfzhzk(v=vs.100)
FORCE_INLINE __m128i _mm_set1_epi64(int64_t _i)
{
    return vreinterpretq_m128i_s64(vdupq_n_s64(_i));
}

// Sets the 2 signed 64-bit integer values to i.
// https://software.intel.com/sites/landingpage/IntrinsicsGuide/#text=_mm_set1_epi64x
FORCE_INLINE __m128i _mm_set1_epi64x(int64_t _i)
{
    return vreinterpretq_m128i_s64(vdupq_n_s64(_i));
}

// Sets the 4 signed 32-bit integer values.
// https://msdn.microsoft.com/en-us/library/vstudio/019beekt(v=vs.100).aspx
FORCE_INLINE __m128i _mm_set_epi32(int i3, int i2, int i1, int i0)
{
    int32_t ALIGN_STRUCT(16) data[4] = {i0, i1, i2, i3};
    return vreinterpretq_m128i_s32(vld1q_s32(data));
}

// Returns the __m128i structure with its two 64-bit integer values
// initialized to the values of the two 64-bit integers passed in.
// https://msdn.microsoft.com/en-us/library/dk2sdw0h(v=vs.120).aspx
FORCE_INLINE __m128i _mm_set_epi64x(int64_t i1, int64_t i2)
{
    int64_t ALIGN_STRUCT(16) data[2] = {i2, i1};
    return vreinterpretq_m128i_s64(vld1q_s64(data));
}

// Returns the __m128i structure with its two 64-bit integer values
// initialized to the values of the two 64-bit integers passed in.
// https://msdn.microsoft.com/en-us/library/dk2sdw0h(v=vs.120).aspx
FORCE_INLINE __m128i _mm_set_epi64(__m64 i1, __m64 i2)
{
    return _mm_set_epi64x((int64_t) i1, (int64_t) i2);
}

// Set packed double-precision (64-bit) floating-point elements in dst with the
// supplied values.
// https://software.intel.com/sites/landingpage/IntrinsicsGuide/#text=_mm_set_pd
FORCE_INLINE __m128d _mm_set_pd(double e1, double e0)
{
    double ALIGN_STRUCT(16) data[2] = {e0, e1};
#if defined(__aarch64__)
    return vreinterpretq_m128d_f64(vld1q_f64((float64_t *) data));
#else
    return vreinterpretq_m128d_f32(vld1q_f32((float32_t *) data));
#endif
}

// Stores four single-precision, floating-point values.
// https://msdn.microsoft.com/en-us/library/vstudio/s3h4ay6y(v=vs.100).aspx
FORCE_INLINE void _mm_store_ps(float *p, __m128 a)
{
    vst1q_f32(p, vreinterpretq_f32_m128(a));
}

// Stores four single-precision, floating-point values.
// https://msdn.microsoft.com/en-us/library/44e30x22(v=vs.100).aspx
FORCE_INLINE void _mm_storeu_ps(float *p, __m128 a)
{
    vst1q_f32(p, vreinterpretq_f32_m128(a));
}

// Stores four 32-bit integer values as (as a __m128i value) at the address p.
// https://msdn.microsoft.com/en-us/library/vstudio/edk11s13(v=vs.100).aspx
FORCE_INLINE void _mm_store_si128(__m128i *p, __m128i a)
{
    vst1q_s32((int32_t *) p, vreinterpretq_s32_m128i(a));
}

// Stores four 32-bit integer values as (as a __m128i value) at the address p.
// https://msdn.microsoft.com/en-us/library/vstudio/edk11s13(v=vs.100).aspx
FORCE_INLINE void _mm_storeu_si128(__m128i *p, __m128i a)
{
    vst1q_s32((int32_t *) p, vreinterpretq_s32_m128i(a));
}

// Stores the lower single - precision, floating - point value.
// https://msdn.microsoft.com/en-us/library/tzz10fbx(v=vs.100).aspx
FORCE_INLINE void _mm_store_ss(float *p, __m128 a)
{
    vst1q_lane_f32(p, vreinterpretq_f32_m128(a), 0);
}

// Store 128-bits (composed of 2 packed double-precision (64-bit) floating-point
// elements) from a into memory. mem_addr must be aligned on a 16-byte boundary
// or a general-protection exception may be generated.
// https://software.intel.com/sites/landingpage/IntrinsicsGuide/#text=_mm_store_pd
FORCE_INLINE void _mm_store_pd(double *mem_addr, __m128d a)
{
#if defined(__aarch64__)
    vst1q_f64((float64_t *) mem_addr, vreinterpretq_f64_m128d(a));
#else
    vst1q_f32((float32_t *) mem_addr, vreinterpretq_f32_m128d(a));
#endif
}

// Store 128-bits (composed of 2 packed double-precision (64-bit) floating-point
// elements) from a into memory. mem_addr does not need to be aligned on any
// particular boundary.
// https://software.intel.com/sites/landingpage/IntrinsicsGuide/#text=_mm_storeu_pd
FORCE_INLINE void _mm_storeu_pd(double *mem_addr, __m128d a)
{
    _mm_store_pd(mem_addr, a);
}

// Reads the lower 64 bits of b and stores them into the lower 64 bits of a.
// https://msdn.microsoft.com/en-us/library/hhwf428f%28v=vs.90%29.aspx
FORCE_INLINE void _mm_storel_epi64(__m128i *a, __m128i b)
{
    uint64x1_t hi = vget_high_u64(vreinterpretq_u64_m128i(*a));
    uint64x1_t lo = vget_low_u64(vreinterpretq_u64_m128i(b));
    *a = vreinterpretq_m128i_u64(vcombine_u64(lo, hi));
}

// Stores the lower two single-precision floating point values of a to the
// address p.
//
//   *p0 := a0
//   *p1 := a1
//
// https://msdn.microsoft.com/en-us/library/h54t98ks(v=vs.90).aspx
FORCE_INLINE void _mm_storel_pi(__m64 *p, __m128 a)
{
    *p = vreinterpret_m64_f32(vget_low_f32(a));
}

// Stores the upper two single-precision, floating-point values of a to the
// address p.
//
//   *p0 := a2
//   *p1 := a3
//
// https://msdn.microsoft.com/en-us/library/a7525fs8(v%3dvs.90).aspx
FORCE_INLINE void _mm_storeh_pi(__m64 *p, __m128 a)
{
    *p = vreinterpret_m64_f32(vget_high_f32(a));
}

// Loads a single single-precision, floating-point value, copying it into all
// four words
// https://msdn.microsoft.com/en-us/library/vstudio/5cdkf716(v=vs.100).aspx
FORCE_INLINE __m128 _mm_load1_ps(const float *p)
{
    return vreinterpretq_m128_f32(vld1q_dup_f32(p));
}
#define _mm_load_ps1 _mm_load1_ps

// Sets the lower two single-precision, floating-point values with 64
// bits of data loaded from the address p; the upper two values are passed
// through from a.
//
// Return Value
//   r0 := *p0
//   r1 := *p1
//   r2 := a2
//   r3 := a3
//
// https://msdn.microsoft.com/en-us/library/s57cyak2(v=vs.100).aspx
FORCE_INLINE __m128 _mm_loadl_pi(__m128 a, __m64 const *p)
{
    return vreinterpretq_m128_f32(
        vcombine_f32(vld1_f32((const float32_t *) p), vget_high_f32(a)));
}

// Sets the upper two single-precision, floating-point values with 64
// bits of data loaded from the address p; the lower two values are passed
// through from a.
//
//   r0 := a0
//   r1 := a1
//   r2 := *p0
//   r3 := *p1
//
// https://msdn.microsoft.com/en-us/library/w92wta0x(v%3dvs.100).aspx
FORCE_INLINE __m128 _mm_loadh_pi(__m128 a, __m64 const *p)
{
    return vreinterpretq_m128_f32(
        vcombine_f32(vget_low_f32(a), vld1_f32((const float32_t *) p)));
}

// Loads four single-precision, floating-point values.
// https://msdn.microsoft.com/en-us/library/vstudio/zzd50xxt(v=vs.100).aspx
FORCE_INLINE __m128 _mm_load_ps(const float *p)
{
    return vreinterpretq_m128_f32(vld1q_f32(p));
}

// Loads four single-precision, floating-point values.
// https://msdn.microsoft.com/en-us/library/x1b16s7z%28v=vs.90%29.aspx
FORCE_INLINE __m128 _mm_loadu_ps(const float *p)
{
    // for neon, alignment doesn't matter, so _mm_load_ps and _mm_loadu_ps are
    // equivalent for neon
    return vreinterpretq_m128_f32(vld1q_f32(p));
}

// Loads a double-precision, floating-point value.
// The upper double-precision, floating-point is set to zero. The address p does
// not need to be 16-byte aligned.
// https://docs.microsoft.com/en-us/previous-versions/visualstudio/visual-studio-2010/574w9fdd(v%3dvs.100)
FORCE_INLINE __m128d _mm_load_sd(const double *p)
{
#if defined(__aarch64__)
    return vsetq_lane_f64(*p, vdupq_n_f64(0), 0);
#else
    const float *fp = (const float *) p;
    float ALIGN_STRUCT(16) data[4] = {fp[0], fp[1], 0, 0};
    return vld1q_f32(data);
#endif
}

// Loads two double-precision from 16-byte aligned memory, floating-point
// values.
// https://software.intel.com/sites/landingpage/IntrinsicsGuide/#text=load_pd
FORCE_INLINE __m128d _mm_load_pd(const double *p)
{
#if defined(__aarch64__)
    return (__m128d)(vld1q_f64(p));
#else
    const float *fp = (const float *) p;
    float ALIGN_STRUCT(16) data[4] = {fp[0], fp[1], fp[2], fp[3]};
    return vld1q_f32(data);
#endif
}

// Loads two double-precision from unaligned memory, floating-point values.
// https://software.intel.com/sites/landingpage/IntrinsicsGuide/#text=loadu_pd
FORCE_INLINE __m128d _mm_loadu_pd(const double *p)
{
#if defined(__aarch64__)
    return (__m128d)(vld1q_f64(p));
#else
    return _mm_load_pd(p);
#endif
}

// Loads an single - precision, floating - point value into the low word and
// clears the upper three words.
// https://msdn.microsoft.com/en-us/library/548bb9h4%28v=vs.90%29.aspx
FORCE_INLINE __m128 _mm_load_ss(const float *p)
{
    return vreinterpretq_m128_f32(vsetq_lane_f32(*p, vdupq_n_f32(0), 0));
}

FORCE_INLINE __m128i _mm_loadl_epi64(__m128i const *p)
{
    /* Load the lower 64 bits of the value pointed to by p into the
     * lower 64 bits of the result, zeroing the upper 64 bits of the result.
     */
    return vreinterpretq_m128i_s32(
        vcombine_s32(vld1_s32((int32_t const *) p), vcreate_s32(0)));
}

// Sets the low word to the single-precision, floating-point value of b
// https://docs.microsoft.com/en-us/previous-versions/visualstudio/visual-studio-2010/35hdzazd(v=vs.100)
FORCE_INLINE __m128 _mm_move_ss(__m128 a, __m128 b)
{
    return vreinterpretq_m128_f32(
        vsetq_lane_f32(vgetq_lane_f32(vreinterpretq_f32_m128(b), 0),
                       vreinterpretq_f32_m128(a), 0));
}

// Copy the lower 64-bit integer in a to the lower element of dst, and zero the
// upper element.
//
//   dst[63:0] := a[63:0]
//   dst[127:64] := 0
//
// https://software.intel.com/sites/landingpage/IntrinsicsGuide/#text=_mm_move_epi64
FORCE_INLINE __m128i _mm_move_epi64(__m128i a)
{
    return vreinterpretq_m128i_s64(
        vsetq_lane_s64(0, vreinterpretq_s64_m128i(a), 1));
}

// Return vector of type __m128 with undefined elements.
// https://software.intel.com/sites/landingpage/IntrinsicsGuide/#text=_mm_undefined_ps
FORCE_INLINE __m128 _mm_undefined_ps(void)
{
    __m128 a;
    return a;
}

/* Logic/Binary operations */

// Computes the bitwise AND-NOT of the four single-precision, floating-point
// values of a and b.
//
//   r0 := ~a0 & b0
//   r1 := ~a1 & b1
//   r2 := ~a2 & b2
//   r3 := ~a3 & b3
//
// https://msdn.microsoft.com/en-us/library/vstudio/68h7wd02(v=vs.100).aspx
FORCE_INLINE __m128 _mm_andnot_ps(__m128 a, __m128 b)
{
    return vreinterpretq_m128_s32(
        vbicq_s32(vreinterpretq_s32_m128(b),
                  vreinterpretq_s32_m128(a)));  // *NOTE* argument swap
}

// Compute the bitwise NOT of packed double-precision (64-bit) floating-point
// elements in a and then AND with b, and store the results in dst.
//
//   FOR j := 0 to 1
// 	     i := j*64
// 	     dst[i+63:i] := ((NOT a[i+63:i]) AND b[i+63:i])
//   ENDFOR
//
// https://software.intel.com/sites/landingpage/IntrinsicsGuide/#text=_mm_andnot_pd
FORCE_INLINE __m128d _mm_andnot_pd(__m128d a, __m128d b)
{
    // *NOTE* argument swap
    return vreinterpretq_m128d_s64(
        vbicq_s64(vreinterpretq_s64_m128d(b), vreinterpretq_s64_m128d(a)));
}

// Computes the bitwise AND of the 128-bit value in b and the bitwise NOT of the
// 128-bit value in a.
//
//   r := (~a) & b
//
// https://msdn.microsoft.com/en-us/library/vstudio/1beaceh8(v=vs.100).aspx
FORCE_INLINE __m128i _mm_andnot_si128(__m128i a, __m128i b)
{
    return vreinterpretq_m128i_s32(
        vbicq_s32(vreinterpretq_s32_m128i(b),
                  vreinterpretq_s32_m128i(a)));  // *NOTE* argument swap
}

// Computes the bitwise AND of the 128-bit value in a and the 128-bit value in
// b.
//
//   r := a & b
//
// https://msdn.microsoft.com/en-us/library/vstudio/6d1txsa8(v=vs.100).aspx
FORCE_INLINE __m128i _mm_and_si128(__m128i a, __m128i b)
{
    return vreinterpretq_m128i_s32(
        vandq_s32(vreinterpretq_s32_m128i(a), vreinterpretq_s32_m128i(b)));
}

// Computes the bitwise AND of the four single-precision, floating-point values
// of a and b.
//
//   r0 := a0 & b0
//   r1 := a1 & b1
//   r2 := a2 & b2
//   r3 := a3 & b3
//
// https://msdn.microsoft.com/en-us/library/vstudio/73ck1xc5(v=vs.100).aspx
FORCE_INLINE __m128 _mm_and_ps(__m128 a, __m128 b)
{
    return vreinterpretq_m128_s32(
        vandq_s32(vreinterpretq_s32_m128(a), vreinterpretq_s32_m128(b)));
}

// Compute the bitwise AND of packed double-precision (64-bit) floating-point
// elements in a and b, and store the results in dst.
//
//   FOR j := 0 to 1
//     i := j*64
//     dst[i+63:i] := a[i+63:i] AND b[i+63:i]
//   ENDFOR
//
// https://software.intel.com/sites/landingpage/IntrinsicsGuide/#text=_mm_and_pd
FORCE_INLINE __m128d _mm_and_pd(__m128d a, __m128d b)
{
    return vreinterpretq_m128d_s64(
        vandq_s64(vreinterpretq_s64_m128d(a), vreinterpretq_s64_m128d(b)));
}

// Computes the bitwise OR of the four single-precision, floating-point values
// of a and b.
// https://msdn.microsoft.com/en-us/library/vstudio/7ctdsyy0(v=vs.100).aspx
FORCE_INLINE __m128 _mm_or_ps(__m128 a, __m128 b)
{
    return vreinterpretq_m128_s32(
        vorrq_s32(vreinterpretq_s32_m128(a), vreinterpretq_s32_m128(b)));
}

// Computes bitwise EXOR (exclusive-or) of the four single-precision,
// floating-point values of a and b.
// https://msdn.microsoft.com/en-us/library/ss6k3wk8(v=vs.100).aspx
FORCE_INLINE __m128 _mm_xor_ps(__m128 a, __m128 b)
{
    return vreinterpretq_m128_s32(
        veorq_s32(vreinterpretq_s32_m128(a), vreinterpretq_s32_m128(b)));
}

// Compute the bitwise XOR of packed double-precision (64-bit) floating-point
// elements in a and b, and store the results in dst.
//
//   FOR j := 0 to 1
//      i := j*64
//      dst[i+63:i] := a[i+63:i] XOR b[i+63:i]
//   ENDFOR
//
// https://software.intel.com/sites/landingpage/IntrinsicsGuide/#text=_mm_xor_pd
FORCE_INLINE __m128d _mm_xor_pd(__m128d a, __m128d b)
{
    return vreinterpretq_m128d_s64(
        veorq_s64(vreinterpretq_s64_m128d(a), vreinterpretq_s64_m128d(b)));
}

// Computes the bitwise OR of the 128-bit value in a and the 128-bit value in b.
//
//   r := a | b
//
// https://msdn.microsoft.com/en-us/library/vstudio/ew8ty0db(v=vs.100).aspx
FORCE_INLINE __m128i _mm_or_si128(__m128i a, __m128i b)
{
    return vreinterpretq_m128i_s32(
        vorrq_s32(vreinterpretq_s32_m128i(a), vreinterpretq_s32_m128i(b)));
}

// Computes the bitwise XOR of the 128-bit value in a and the 128-bit value in
// b.  https://msdn.microsoft.com/en-us/library/fzt08www(v=vs.100).aspx
FORCE_INLINE __m128i _mm_xor_si128(__m128i a, __m128i b)
{
    return vreinterpretq_m128i_s32(
        veorq_s32(vreinterpretq_s32_m128i(a), vreinterpretq_s32_m128i(b)));
}

// Duplicate odd-indexed single-precision (32-bit) floating-point elements
// from a, and store the results in dst.
// https://software.intel.com/sites/landingpage/IntrinsicsGuide/#text=_mm_movehdup_ps
FORCE_INLINE __m128 _mm_movehdup_ps(__m128 a)
{
#if __has_builtin(__builtin_shufflevector)
    return vreinterpretq_m128_f32(__builtin_shufflevector(
        vreinterpretq_f32_m128(a), vreinterpretq_f32_m128(a), 1, 1, 3, 3));
#else
    float32_t a1 = vgetq_lane_f32(vreinterpretq_f32_m128(a), 1);
    float32_t a3 = vgetq_lane_f32(vreinterpretq_f32_m128(a), 3);
    float ALIGN_STRUCT(16) data[4] = {a1, a1, a3, a3};
    return vreinterpretq_m128_f32(vld1q_f32(data));
#endif
}

// Duplicate even-indexed single-precision (32-bit) floating-point elements
// from a, and store the results in dst.
// https://software.intel.com/sites/landingpage/IntrinsicsGuide/#text=_mm_moveldup_ps
FORCE_INLINE __m128 _mm_moveldup_ps(__m128 a)
{
#if __has_builtin(__builtin_shufflevector)
    return vreinterpretq_m128_f32(__builtin_shufflevector(
        vreinterpretq_f32_m128(a), vreinterpretq_f32_m128(a), 0, 0, 2, 2));
#else
    float32_t a0 = vgetq_lane_f32(vreinterpretq_f32_m128(a), 0);
    float32_t a2 = vgetq_lane_f32(vreinterpretq_f32_m128(a), 2);
    float ALIGN_STRUCT(16) data[4] = {a0, a0, a2, a2};
    return vreinterpretq_m128_f32(vld1q_f32(data));
#endif
}

// Moves the upper two values of B into the lower two values of A.
//
//   r3 := a3
//   r2 := a2
//   r1 := b3
//   r0 := b2
FORCE_INLINE __m128 _mm_movehl_ps(__m128 __A, __m128 __B)
{
    float32x2_t a32 = vget_high_f32(vreinterpretq_f32_m128(__A));
    float32x2_t b32 = vget_high_f32(vreinterpretq_f32_m128(__B));
    return vreinterpretq_m128_f32(vcombine_f32(b32, a32));
}

// Moves the lower two values of B into the upper two values of A.
//
//   r3 := b1
//   r2 := b0
//   r1 := a1
//   r0 := a0
FORCE_INLINE __m128 _mm_movelh_ps(__m128 __A, __m128 __B)
{
    float32x2_t a10 = vget_low_f32(vreinterpretq_f32_m128(__A));
    float32x2_t b10 = vget_low_f32(vreinterpretq_f32_m128(__B));
    return vreinterpretq_m128_f32(vcombine_f32(a10, b10));
}

// Compute the absolute value of packed signed 32-bit integers in a, and store
// the unsigned results in dst.
//
//   FOR j := 0 to 3
//     i := j*32
//     dst[i+31:i] := ABS(a[i+31:i])
//   ENDFOR
//
// https://software.intel.com/sites/landingpage/IntrinsicsGuide/#text=_mm_abs_epi32
FORCE_INLINE __m128i _mm_abs_epi32(__m128i a)
{
    return vreinterpretq_m128i_s32(vabsq_s32(vreinterpretq_s32_m128i(a)));
}

// Compute the absolute value of packed signed 16-bit integers in a, and store
// the unsigned results in dst.
//
//   FOR j := 0 to 7
//     i := j*16
//     dst[i+15:i] := ABS(a[i+15:i])
//   ENDFOR
//
// https://software.intel.com/sites/landingpage/IntrinsicsGuide/#text=_mm_abs_epi16
FORCE_INLINE __m128i _mm_abs_epi16(__m128i a)
{
    return vreinterpretq_m128i_s16(vabsq_s16(vreinterpretq_s16_m128i(a)));
}

// Compute the absolute value of packed signed 8-bit integers in a, and store
// the unsigned results in dst.
//
//   FOR j := 0 to 15
//     i := j*8
//     dst[i+7:i] := ABS(a[i+7:i])
//   ENDFOR
//
// https://software.intel.com/sites/landingpage/IntrinsicsGuide/#text=_mm_abs_epi8
FORCE_INLINE __m128i _mm_abs_epi8(__m128i a)
{
    return vreinterpretq_m128i_s8(vabsq_s8(vreinterpretq_s8_m128i(a)));
}

// Compute the absolute value of packed signed 32-bit integers in a, and store
// the unsigned results in dst.
//
//   FOR j := 0 to 1
//     i := j*32
//     dst[i+31:i] := ABS(a[i+31:i])
//   ENDFOR
//
// https://software.intel.com/sites/landingpage/IntrinsicsGuide/#text=_mm_abs_pi32
FORCE_INLINE __m64 _mm_abs_pi32(__m64 a)
{
    return vreinterpret_m64_s32(vabs_s32(vreinterpret_s32_m64(a)));
}

// Compute the absolute value of packed signed 16-bit integers in a, and store
// the unsigned results in dst.
//
//   FOR j := 0 to 3
//     i := j*16
//     dst[i+15:i] := ABS(a[i+15:i])
//   ENDFOR
//
// https://software.intel.com/sites/landingpage/IntrinsicsGuide/#text=_mm_abs_pi16
FORCE_INLINE __m64 _mm_abs_pi16(__m64 a)
{
    return vreinterpret_m64_s16(vabs_s16(vreinterpret_s16_m64(a)));
}

// Compute the absolute value of packed signed 8-bit integers in a, and store
// the unsigned results in dst.
//
//   FOR j := 0 to 7
//     i := j*8
//     dst[i+7:i] := ABS(a[i+7:i])
//   ENDFOR
//
// https://software.intel.com/sites/landingpage/IntrinsicsGuide/#text=_mm_abs_pi8
FORCE_INLINE __m64 _mm_abs_pi8(__m64 a)
{
    return vreinterpret_m64_s8(vabs_s8(vreinterpret_s8_m64(a)));
}

// Takes the upper 64 bits of a and places it in the low end of the result
// Takes the lower 64 bits of b and places it into the high end of the result.
FORCE_INLINE __m128 _mm_shuffle_ps_1032(__m128 a, __m128 b)
{
    float32x2_t a32 = vget_high_f32(vreinterpretq_f32_m128(a));
    float32x2_t b10 = vget_low_f32(vreinterpretq_f32_m128(b));
    return vreinterpretq_m128_f32(vcombine_f32(a32, b10));
}

// takes the lower two 32-bit values from a and swaps them and places in high
// end of result takes the higher two 32 bit values from b and swaps them and
// places in low end of result.
FORCE_INLINE __m128 _mm_shuffle_ps_2301(__m128 a, __m128 b)
{
    float32x2_t a01 = vrev64_f32(vget_low_f32(vreinterpretq_f32_m128(a)));
    float32x2_t b23 = vrev64_f32(vget_high_f32(vreinterpretq_f32_m128(b)));
    return vreinterpretq_m128_f32(vcombine_f32(a01, b23));
}

FORCE_INLINE __m128 _mm_shuffle_ps_0321(__m128 a, __m128 b)
{
    float32x2_t a21 = vget_high_f32(
        vextq_f32(vreinterpretq_f32_m128(a), vreinterpretq_f32_m128(a), 3));
    float32x2_t b03 = vget_low_f32(
        vextq_f32(vreinterpretq_f32_m128(b), vreinterpretq_f32_m128(b), 3));
    return vreinterpretq_m128_f32(vcombine_f32(a21, b03));
}

FORCE_INLINE __m128 _mm_shuffle_ps_2103(__m128 a, __m128 b)
{
    float32x2_t a03 = vget_low_f32(
        vextq_f32(vreinterpretq_f32_m128(a), vreinterpretq_f32_m128(a), 3));
    float32x2_t b21 = vget_high_f32(
        vextq_f32(vreinterpretq_f32_m128(b), vreinterpretq_f32_m128(b), 3));
    return vreinterpretq_m128_f32(vcombine_f32(a03, b21));
}

FORCE_INLINE __m128 _mm_shuffle_ps_1010(__m128 a, __m128 b)
{
    float32x2_t a10 = vget_low_f32(vreinterpretq_f32_m128(a));
    float32x2_t b10 = vget_low_f32(vreinterpretq_f32_m128(b));
    return vreinterpretq_m128_f32(vcombine_f32(a10, b10));
}

FORCE_INLINE __m128 _mm_shuffle_ps_1001(__m128 a, __m128 b)
{
    float32x2_t a01 = vrev64_f32(vget_low_f32(vreinterpretq_f32_m128(a)));
    float32x2_t b10 = vget_low_f32(vreinterpretq_f32_m128(b));
    return vreinterpretq_m128_f32(vcombine_f32(a01, b10));
}

FORCE_INLINE __m128 _mm_shuffle_ps_0101(__m128 a, __m128 b)
{
    float32x2_t a01 = vrev64_f32(vget_low_f32(vreinterpretq_f32_m128(a)));
    float32x2_t b01 = vrev64_f32(vget_low_f32(vreinterpretq_f32_m128(b)));
    return vreinterpretq_m128_f32(vcombine_f32(a01, b01));
}

// keeps the low 64 bits of b in the low and puts the high 64 bits of a in the
// high
FORCE_INLINE __m128 _mm_shuffle_ps_3210(__m128 a, __m128 b)
{
    float32x2_t a10 = vget_low_f32(vreinterpretq_f32_m128(a));
    float32x2_t b32 = vget_high_f32(vreinterpretq_f32_m128(b));
    return vreinterpretq_m128_f32(vcombine_f32(a10, b32));
}

FORCE_INLINE __m128 _mm_shuffle_ps_0011(__m128 a, __m128 b)
{
    float32x2_t a11 = vdup_lane_f32(vget_low_f32(vreinterpretq_f32_m128(a)), 1);
    float32x2_t b00 = vdup_lane_f32(vget_low_f32(vreinterpretq_f32_m128(b)), 0);
    return vreinterpretq_m128_f32(vcombine_f32(a11, b00));
}

FORCE_INLINE __m128 _mm_shuffle_ps_0022(__m128 a, __m128 b)
{
    float32x2_t a22 =
        vdup_lane_f32(vget_high_f32(vreinterpretq_f32_m128(a)), 0);
    float32x2_t b00 = vdup_lane_f32(vget_low_f32(vreinterpretq_f32_m128(b)), 0);
    return vreinterpretq_m128_f32(vcombine_f32(a22, b00));
}

FORCE_INLINE __m128 _mm_shuffle_ps_2200(__m128 a, __m128 b)
{
    float32x2_t a00 = vdup_lane_f32(vget_low_f32(vreinterpretq_f32_m128(a)), 0);
    float32x2_t b22 =
        vdup_lane_f32(vget_high_f32(vreinterpretq_f32_m128(b)), 0);
    return vreinterpretq_m128_f32(vcombine_f32(a00, b22));
}

FORCE_INLINE __m128 _mm_shuffle_ps_3202(__m128 a, __m128 b)
{
    float32_t a0 = vgetq_lane_f32(vreinterpretq_f32_m128(a), 0);
    float32x2_t a22 =
        vdup_lane_f32(vget_high_f32(vreinterpretq_f32_m128(a)), 0);
    float32x2_t a02 = vset_lane_f32(a0, a22, 1); /* TODO: use vzip ?*/
    float32x2_t b32 = vget_high_f32(vreinterpretq_f32_m128(b));
    return vreinterpretq_m128_f32(vcombine_f32(a02, b32));
}

FORCE_INLINE __m128 _mm_shuffle_ps_1133(__m128 a, __m128 b)
{
    float32x2_t a33 =
        vdup_lane_f32(vget_high_f32(vreinterpretq_f32_m128(a)), 1);
    float32x2_t b11 = vdup_lane_f32(vget_low_f32(vreinterpretq_f32_m128(b)), 1);
    return vreinterpretq_m128_f32(vcombine_f32(a33, b11));
}

FORCE_INLINE __m128 _mm_shuffle_ps_2010(__m128 a, __m128 b)
{
    float32x2_t a10 = vget_low_f32(vreinterpretq_f32_m128(a));
    float32_t b2 = vgetq_lane_f32(vreinterpretq_f32_m128(b), 2);
    float32x2_t b00 = vdup_lane_f32(vget_low_f32(vreinterpretq_f32_m128(b)), 0);
    float32x2_t b20 = vset_lane_f32(b2, b00, 1);
    return vreinterpretq_m128_f32(vcombine_f32(a10, b20));
}

FORCE_INLINE __m128 _mm_shuffle_ps_2001(__m128 a, __m128 b)
{
    float32x2_t a01 = vrev64_f32(vget_low_f32(vreinterpretq_f32_m128(a)));
    float32_t b2 = vgetq_lane_f32(b, 2);
    float32x2_t b00 = vdup_lane_f32(vget_low_f32(vreinterpretq_f32_m128(b)), 0);
    float32x2_t b20 = vset_lane_f32(b2, b00, 1);
    return vreinterpretq_m128_f32(vcombine_f32(a01, b20));
}

FORCE_INLINE __m128 _mm_shuffle_ps_2032(__m128 a, __m128 b)
{
    float32x2_t a32 = vget_high_f32(vreinterpretq_f32_m128(a));
    float32_t b2 = vgetq_lane_f32(b, 2);
    float32x2_t b00 = vdup_lane_f32(vget_low_f32(vreinterpretq_f32_m128(b)), 0);
    float32x2_t b20 = vset_lane_f32(b2, b00, 1);
    return vreinterpretq_m128_f32(vcombine_f32(a32, b20));
}

// NEON does not support a general purpose permute intrinsic
// Selects four specific single-precision, floating-point values from a and b,
// based on the mask i.
//
// C equivalent:
//   __m128 _mm_shuffle_ps_default(__m128 a, __m128 b,
//                                 __constrange(0, 255) int imm) {
//       __m128 ret;
//       ret[0] = a[imm        & 0x3];   ret[1] = a[(imm >> 2) & 0x3];
//       ret[2] = b[(imm >> 4) & 0x03];  ret[3] = b[(imm >> 6) & 0x03];
//       return ret;
//   }
//
// https://msdn.microsoft.com/en-us/library/vstudio/5f0858x0(v=vs.100).aspx
#define _mm_shuffle_ps_default(a, b, imm)                                  \
    __extension__({                                                        \
        float32x4_t ret;                                                   \
        ret = vmovq_n_f32(                                                 \
            vgetq_lane_f32(vreinterpretq_f32_m128(a), (imm) & (0x3)));     \
        ret = vsetq_lane_f32(                                              \
            vgetq_lane_f32(vreinterpretq_f32_m128(a), ((imm) >> 2) & 0x3), \
            ret, 1);                                                       \
        ret = vsetq_lane_f32(                                              \
            vgetq_lane_f32(vreinterpretq_f32_m128(b), ((imm) >> 4) & 0x3), \
            ret, 2);                                                       \
        ret = vsetq_lane_f32(                                              \
            vgetq_lane_f32(vreinterpretq_f32_m128(b), ((imm) >> 6) & 0x3), \
            ret, 3);                                                       \
        vreinterpretq_m128_f32(ret);                                       \
    })

// FORCE_INLINE __m128 _mm_shuffle_ps(__m128 a, __m128 b, __constrange(0,255)
// int imm)
#if __has_builtin(__builtin_shufflevector)
#define _mm_shuffle_ps(a, b, imm)                                \
    __extension__({                                              \
        float32x4_t _input1 = vreinterpretq_f32_m128(a);         \
        float32x4_t _input2 = vreinterpretq_f32_m128(b);         \
        float32x4_t _shuf = __builtin_shufflevector(             \
            _input1, _input2, (imm) & (0x3), ((imm) >> 2) & 0x3, \
            (((imm) >> 4) & 0x3) + 4, (((imm) >> 6) & 0x3) + 4); \
        vreinterpretq_m128_f32(_shuf);                           \
    })
#else  // generic
#define _mm_shuffle_ps(a, b, imm)                          \
    __extension__({                                        \
        __m128 ret;                                        \
        switch (imm) {                                     \
        case _MM_SHUFFLE(1, 0, 3, 2):                      \
            ret = _mm_shuffle_ps_1032((a), (b));           \
            break;                                         \
        case _MM_SHUFFLE(2, 3, 0, 1):                      \
            ret = _mm_shuffle_ps_2301((a), (b));           \
            break;                                         \
        case _MM_SHUFFLE(0, 3, 2, 1):                      \
            ret = _mm_shuffle_ps_0321((a), (b));           \
            break;                                         \
        case _MM_SHUFFLE(2, 1, 0, 3):                      \
            ret = _mm_shuffle_ps_2103((a), (b));           \
            break;                                         \
        case _MM_SHUFFLE(1, 0, 1, 0):                      \
            ret = _mm_movelh_ps((a), (b));                 \
            break;                                         \
        case _MM_SHUFFLE(1, 0, 0, 1):                      \
            ret = _mm_shuffle_ps_1001((a), (b));           \
            break;                                         \
        case _MM_SHUFFLE(0, 1, 0, 1):                      \
            ret = _mm_shuffle_ps_0101((a), (b));           \
            break;                                         \
        case _MM_SHUFFLE(3, 2, 1, 0):                      \
            ret = _mm_shuffle_ps_3210((a), (b));           \
            break;                                         \
        case _MM_SHUFFLE(0, 0, 1, 1):                      \
            ret = _mm_shuffle_ps_0011((a), (b));           \
            break;                                         \
        case _MM_SHUFFLE(0, 0, 2, 2):                      \
            ret = _mm_shuffle_ps_0022((a), (b));           \
            break;                                         \
        case _MM_SHUFFLE(2, 2, 0, 0):                      \
            ret = _mm_shuffle_ps_2200((a), (b));           \
            break;                                         \
        case _MM_SHUFFLE(3, 2, 0, 2):                      \
            ret = _mm_shuffle_ps_3202((a), (b));           \
            break;                                         \
        case _MM_SHUFFLE(3, 2, 3, 2):                      \
            ret = _mm_movehl_ps((b), (a));                 \
            break;                                         \
        case _MM_SHUFFLE(1, 1, 3, 3):                      \
            ret = _mm_shuffle_ps_1133((a), (b));           \
            break;                                         \
        case _MM_SHUFFLE(2, 0, 1, 0):                      \
            ret = _mm_shuffle_ps_2010((a), (b));           \
            break;                                         \
        case _MM_SHUFFLE(2, 0, 0, 1):                      \
            ret = _mm_shuffle_ps_2001((a), (b));           \
            break;                                         \
        case _MM_SHUFFLE(2, 0, 3, 2):                      \
            ret = _mm_shuffle_ps_2032((a), (b));           \
            break;                                         \
        default:                                           \
            ret = _mm_shuffle_ps_default((a), (b), (imm)); \
            break;                                         \
        }                                                  \
        ret;                                               \
    })
#endif

// Takes the upper 64 bits of a and places it in the low end of the result
// Takes the lower 64 bits of a and places it into the high end of the result.
FORCE_INLINE __m128i _mm_shuffle_epi_1032(__m128i a)
{
    int32x2_t a32 = vget_high_s32(vreinterpretq_s32_m128i(a));
    int32x2_t a10 = vget_low_s32(vreinterpretq_s32_m128i(a));
    return vreinterpretq_m128i_s32(vcombine_s32(a32, a10));
}

// takes the lower two 32-bit values from a and swaps them and places in low end
// of result takes the higher two 32 bit values from a and swaps them and places
// in high end of result.
FORCE_INLINE __m128i _mm_shuffle_epi_2301(__m128i a)
{
    int32x2_t a01 = vrev64_s32(vget_low_s32(vreinterpretq_s32_m128i(a)));
    int32x2_t a23 = vrev64_s32(vget_high_s32(vreinterpretq_s32_m128i(a)));
    return vreinterpretq_m128i_s32(vcombine_s32(a01, a23));
}

// rotates the least significant 32 bits into the most signficant 32 bits, and
// shifts the rest down
FORCE_INLINE __m128i _mm_shuffle_epi_0321(__m128i a)
{
    return vreinterpretq_m128i_s32(
        vextq_s32(vreinterpretq_s32_m128i(a), vreinterpretq_s32_m128i(a), 1));
}

// rotates the most significant 32 bits into the least signficant 32 bits, and
// shifts the rest up
FORCE_INLINE __m128i _mm_shuffle_epi_2103(__m128i a)
{
    return vreinterpretq_m128i_s32(
        vextq_s32(vreinterpretq_s32_m128i(a), vreinterpretq_s32_m128i(a), 3));
}

// gets the lower 64 bits of a, and places it in the upper 64 bits
// gets the lower 64 bits of a and places it in the lower 64 bits
FORCE_INLINE __m128i _mm_shuffle_epi_1010(__m128i a)
{
    int32x2_t a10 = vget_low_s32(vreinterpretq_s32_m128i(a));
    return vreinterpretq_m128i_s32(vcombine_s32(a10, a10));
}

// gets the lower 64 bits of a, swaps the 0 and 1 elements, and places it in the
// lower 64 bits gets the lower 64 bits of a, and places it in the upper 64 bits
FORCE_INLINE __m128i _mm_shuffle_epi_1001(__m128i a)
{
    int32x2_t a01 = vrev64_s32(vget_low_s32(vreinterpretq_s32_m128i(a)));
    int32x2_t a10 = vget_low_s32(vreinterpretq_s32_m128i(a));
    return vreinterpretq_m128i_s32(vcombine_s32(a01, a10));
}

// gets the lower 64 bits of a, swaps the 0 and 1 elements and places it in the
// upper 64 bits gets the lower 64 bits of a, swaps the 0 and 1 elements, and
// places it in the lower 64 bits
FORCE_INLINE __m128i _mm_shuffle_epi_0101(__m128i a)
{
    int32x2_t a01 = vrev64_s32(vget_low_s32(vreinterpretq_s32_m128i(a)));
    return vreinterpretq_m128i_s32(vcombine_s32(a01, a01));
}

FORCE_INLINE __m128i _mm_shuffle_epi_2211(__m128i a)
{
    int32x2_t a11 = vdup_lane_s32(vget_low_s32(vreinterpretq_s32_m128i(a)), 1);
    int32x2_t a22 = vdup_lane_s32(vget_high_s32(vreinterpretq_s32_m128i(a)), 0);
    return vreinterpretq_m128i_s32(vcombine_s32(a11, a22));
}

FORCE_INLINE __m128i _mm_shuffle_epi_0122(__m128i a)
{
    int32x2_t a22 = vdup_lane_s32(vget_high_s32(vreinterpretq_s32_m128i(a)), 0);
    int32x2_t a01 = vrev64_s32(vget_low_s32(vreinterpretq_s32_m128i(a)));
    return vreinterpretq_m128i_s32(vcombine_s32(a22, a01));
}

FORCE_INLINE __m128i _mm_shuffle_epi_3332(__m128i a)
{
    int32x2_t a32 = vget_high_s32(vreinterpretq_s32_m128i(a));
    int32x2_t a33 = vdup_lane_s32(vget_high_s32(vreinterpretq_s32_m128i(a)), 1);
    return vreinterpretq_m128i_s32(vcombine_s32(a32, a33));
}

// Shuffle packed 8-bit integers in a according to shuffle control mask in the
// corresponding 8-bit element of b, and store the results in dst.
// https://software.intel.com/sites/landingpage/IntrinsicsGuide/#text=_mm_shuffle_epi8
FORCE_INLINE __m128i _mm_shuffle_epi8(__m128i a, __m128i b)
{
    int8x16_t tbl = vreinterpretq_s8_m128i(a);   // input a
    uint8x16_t idx = vreinterpretq_u8_m128i(b);  // input b
    uint8x16_t idx_masked =
        vandq_u8(idx, vdupq_n_u8(0x8F));  // avoid using meaningless bits
#if defined(__aarch64__)
    return vreinterpretq_m128i_s8(vqtbl1q_s8(tbl, idx_masked));
#elif defined(__GNUC__)
    int8x16_t ret;
    // %e and %f represent the even and odd D registers
    // respectively.
    __asm__ __volatile__(
        "vtbl.8  %e[ret], {%e[tbl], %f[tbl]}, %e[idx]\n"
        "vtbl.8  %f[ret], {%e[tbl], %f[tbl]}, %f[idx]\n"
        : [ret] "=&w"(ret)
        : [tbl] "w"(tbl), [idx] "w"(idx_masked));
    return vreinterpretq_m128i_s8(ret);
#else
    // use this line if testing on aarch64
    int8x8x2_t a_split = {vget_low_s8(tbl), vget_high_s8(tbl)};
    return vreinterpretq_m128i_s8(
        vcombine_s8(vtbl2_s8(a_split, vget_low_u8(idx_masked)),
                    vtbl2_s8(a_split, vget_high_u8(idx_masked))));
#endif
}

// C equivalent:
//   __m128i _mm_shuffle_epi32_default(__m128i a,
//                                     __constrange(0, 255) int imm) {
//       __m128i ret;
//       ret[0] = a[imm        & 0x3];   ret[1] = a[(imm >> 2) & 0x3];
//       ret[2] = a[(imm >> 4) & 0x03];  ret[3] = a[(imm >> 6) & 0x03];
//       return ret;
//   }
#define _mm_shuffle_epi32_default(a, imm)                                   \
    __extension__({                                                         \
        int32x4_t ret;                                                      \
        ret = vmovq_n_s32(                                                  \
            vgetq_lane_s32(vreinterpretq_s32_m128i(a), (imm) & (0x3)));     \
        ret = vsetq_lane_s32(                                               \
            vgetq_lane_s32(vreinterpretq_s32_m128i(a), ((imm) >> 2) & 0x3), \
            ret, 1);                                                        \
        ret = vsetq_lane_s32(                                               \
            vgetq_lane_s32(vreinterpretq_s32_m128i(a), ((imm) >> 4) & 0x3), \
            ret, 2);                                                        \
        ret = vsetq_lane_s32(                                               \
            vgetq_lane_s32(vreinterpretq_s32_m128i(a), ((imm) >> 6) & 0x3), \
            ret, 3);                                                        \
        vreinterpretq_m128i_s32(ret);                                       \
    })

// FORCE_INLINE __m128i _mm_shuffle_epi32_splat(__m128i a, __constrange(0,255)
// int imm)
#if defined(__aarch64__)
#define _mm_shuffle_epi32_splat(a, imm)                          \
    __extension__({                                              \
        vreinterpretq_m128i_s32(                                 \
            vdupq_laneq_s32(vreinterpretq_s32_m128i(a), (imm))); \
    })
#else
#define _mm_shuffle_epi32_splat(a, imm)                                      \
    __extension__({                                                          \
        vreinterpretq_m128i_s32(                                             \
            vdupq_n_s32(vgetq_lane_s32(vreinterpretq_s32_m128i(a), (imm)))); 
