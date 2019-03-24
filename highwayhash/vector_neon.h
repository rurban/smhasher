// Copyright 2016-2019 Google Inc. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef HIGHWAYHASH_VECTOR128_NEON_H_
#define HIGHWAYHASH_VECTOR128_NEON_H_

// Defines SIMD vector classes ("V2x64U") with overloaded arithmetic operators:
// const V2x64U masked_sum = (a + b) & m;
// This is shorter and more readable than compiler intrinsics:
// const uint64x2_t masked_sum = vandq_u64(vaddq_u64(a, b), m);
// There is typically no runtime cost for these abstractions.
//
// The naming convention is VNxBBT where N is the number of lanes, BB the
// number of bits per lane and T is the lane type: unsigned integer (U),
// signed integer (I), or floating-point (F). (Note: Floating point vectors are
// currently disabled).

// WARNING: this is a "restricted" header because it is included from
// translation units compiled with different flags. This header and its
// dependencies must not define any function unless it is static inline and/or
// within namespace HH_TARGET_NAME. See arch_specific.h for details.

#include "highwayhash/arch_specific.h"
#include "highwayhash/compiler_specific.h"

// For auto-dependency generation, we need to include all headers but not their
// contents.
#ifndef HH_DISABLE_TARGET_SPECIFIC
#include <arm_neon.h>  // NEON

namespace highwayhash {
// To prevent ODR violations when including this from multiple translation
// units (TU) that are compiled with different flags, the contents must reside
// in a namespace whose name is unique to the TU. NOTE: this behavior is
// incompatible with precompiled modules and requires textual inclusion instead.
namespace HH_TARGET_NAME {

// Polyfills for ARMv7. ARMv7 lacks a few important instructions which were added
// in aarch64, so we simulate them with these polyfills.
#if !defined(__aarch64__) && !defined(__arm64__)
#ifndef vqtbl1q_u8
// aarch64 allows a 128-bit lookup table with vtbl. ARMv7 needs to do two
// lookups for the same effect.
HH_INLINE uint8x16_t vqtbl1q_u8 (uint8x16_t t, uint8x16_t idx)
{
  // Prevents scalarizing by GCC.
  // NOT PORTABLE TO AARCH64! aarch64 uses two separate vectors instead of packing
  // them, so this reinterpret_cast would fail!
  uint8x8x2_t split = *reinterpret_cast<const uint8x8x2_t *>(&t);
  return vcombine_u8(
    vtbl2_u8(split, vget_low_u8(idx)),
    vtbl2_u8(split, vget_high_u8(idx))
  );
}
#endif
#ifndef vnegq_s64
// ARMv7 lacks this for some weird reason.
HH_INLINE int64x2_t vnegq_s64(int64x2_t v)
{
    const int64x2_t zero = vdupq_n_u64(0);
    return vsubq_s64(zero, v);
}
#endif
#ifndef vceqq_u64
HH_INLINE uint64x2_t vceqq_u64(uint64x2_t v1, uint64x2_t v2)
{
    uint32x4_t comparison = vceqq_u32(vreinterpretq_u32_u64(v1), vreinterpretq_u32_u64(v2));
    return vreinterpretq_u64_u32(vandq_u32(comparison, vrev64q_u32(comparison)));
}
#endif // vnegq_s64
#endif // !__aarch64__ && !__arm64__

// Pseudo-instructions.
// _mm_storel_epi64
HH_INLINE void vst1q_low_u64(uint64_t* a, uint64x2_t b)
{
  uint64x1_t lo = vget_low_u64(b);
  vst1_u8(reinterpret_cast<uint8_t*>(a), vreinterpret_u8_u64(lo));
}
// _mm_loadl_epi64
HH_INLINE uint64x2_t vld1q_low_u64(const uint64_t* p)
{
  return vcombine_u64(
    vld1_u64(p),
    vdup_n_u64(0)
  );
}
// _mm_slli_si128 (almost)
#define vshlq_n_u128(a, imm) ( \
  vreinterpretq_u64_u8( \
    vextq_u8( \
      vdupq_n_u8(0), \
      vreinterpretq_u8_u64(a), \
      16 - (imm) \
    ) \
  ) \
)

// Adapted from xsimd.
// arm_neon.h requires literals for their parameters in many
// functions, such as vshrq_n_u64, and it will complain even when
// the value is known at compile-time.
#define EXPAND(...) __VA_ARGS__

#define CASE(op, i, ...)                   \
    case i: v_ = op(__VA_ARGS__, i); break;

#define INTRINSIC_REPEAT_8_0(op, addx, ...)    \
    CASE(EXPAND(op), 1 + addx, __VA_ARGS__);       \
    CASE(EXPAND(op), 2 + addx, __VA_ARGS__);       \
    CASE(EXPAND(op), 3 + addx, __VA_ARGS__);       \
    CASE(EXPAND(op), 4 + addx, __VA_ARGS__);       \
    CASE(EXPAND(op), 5 + addx, __VA_ARGS__);       \
    CASE(EXPAND(op), 6 + addx, __VA_ARGS__);       \
    CASE(EXPAND(op), 7 + addx, __VA_ARGS__);

#define INTRINSIC_REPEAT_8_N(op, addx, ...)    \
    CASE(EXPAND(op), 0 + addx, __VA_ARGS__);       \
    INTRINSIC_REPEAT_8_0(op, addx, __VA_ARGS__);

#define INTRINSIC_REPEAT_8(op, ...)            \
    INTRINSIC_REPEAT_8_0(op, 0, __VA_ARGS__);

#define INTRINSIC_REPEAT_16_0(op, addx,...)   \
    INTRINSIC_REPEAT_8_0(op, 0 + addx, __VA_ARGS__);   \
    INTRINSIC_REPEAT_8_N(op, 8 + addx, __VA_ARGS__);

#define INTRINSIC_REPEAT_16_N(op, addx, ...)   \
    INTRINSIC_REPEAT_8_N(op, 0 + addx, __VA_ARGS__);   \
    INTRINSIC_REPEAT_8_N(op, 8 + addx, __VA_ARGS__);

#define INTRINSIC_EACH_16(op, ...)           \
    INTRINSIC_REPEAT_16_0(op, 0, __VA_ARGS__);

#define INTRINSIC_REPEAT_32_0(op, addx, ...)   \
    INTRINSIC_REPEAT_16_0(op, 0 + addx, __VA_ARGS__);  \
    INTRINSIC_REPEAT_16_N(op, 16 + addx, __VA_ARGS__);

#define INTRINSIC_REPEAT_32_N(op, addx, ...)   \
    INTRINSIC_REPEAT_16_N(op, 0 + addx, __VA_ARGS__);  \
    INTRINSIC_REPEAT_16_N(op, 16 + addx, __VA_ARGS__);

#define INTRINSIC_EACH_32(op, ...)           \
    INTRINSIC_REPEAT_32_0(op, 0, __VA_ARGS__);

#define INTRINSIC_EACH_64(op, ...)           \
    INTRINSIC_REPEAT_32_0(op, 0, __VA_ARGS__);         \
    INTRINSIC_REPEAT_32_N(op, 32, __VA_ARGS__);

// Primary template for 128-bit SSE4.1 vectors; only specializations are used.
template <typename T>
class V128 {};

template <>
class V128<uint8_t> {
 public:
  using Intrinsic = uint8x16_t;
  using T = uint8_t;
  static constexpr size_t N = 16;

  // Leaves v_ uninitialized - typically used for output parameters.
  HH_INLINE V128() {}

  // Sets all lanes to the same value.
  HH_INLINE explicit V128(T i) : v_(vdupq_n_u8(i)) {}

  // Copy from other vector.
  HH_INLINE explicit V128(const V128& other) : v_(other.v_) {}

  // C-style cast because vector casts are stupid on NEON.
  template <typename U>
  HH_INLINE explicit V128(const V128<U>& other) : v_((const uint8x16_t)(other)) {}
  HH_INLINE V128& operator=(const V128& other) {
    v_ = other.v_;
    return *this;
  }

  // Convert from/to intrinsics.
  HH_INLINE V128(const Intrinsic& v) : v_(v) {}

  HH_INLINE V128& operator=(const Intrinsic& v) {
    v_ = v;
    return *this;
  }
  HH_INLINE operator Intrinsic() const { return v_; }

  // There are no greater-than comparison instructions for unsigned T.
  HH_INLINE V128 operator==(const V128& other) const {
    return V128(vceqq_u8(v_, other.v_));
  }

  HH_INLINE V128& operator+=(const V128& other) {
    v_ = vaddq_u8(v_, other.v_);
    return *this;
  }
  HH_INLINE V128& operator-=(const V128& other) {
    v_ = vsubq_u8(v_, other.v_);
    return *this;
  }

  HH_INLINE V128& operator&=(const V128& other) {
    v_ = vandq_u8(v_, other.v_);
    return *this;
  }
  HH_INLINE V128& operator|=(const V128& other) {
    v_ = vorrq_u8(v_, other.v_);
    return *this;
  }
  HH_INLINE V128& operator^=(const V128& other) {
    v_ = veorq_u8(v_, other.v_);
    return *this;
  }

  HH_INLINE V128& AndNot(const Intrinsic &neg_mask) {
    v_ = vbicq_u8(v_, neg_mask);
    return *this;
  }
 private:
  Intrinsic v_;
};

template <>
class V128<uint16_t> {
 public:
  using Intrinsic = uint16x8_t;
  using T = uint16_t;
  static constexpr size_t N = 8;

  // Leaves v_ uninitialized - typically used for output parameters.
  HH_INLINE V128() {}

  // Lane 0 (p_0) is the lowest.
  HH_INLINE V128(T p_7, T p_6, T p_5, T p_4, T p_3, T p_2, T p_1, T p_0) {
    alignas(16) const uint16_t data[8] = {
      p_0, p_1, p_2, p_3, p_4, p_5, p_6, p_7
    };
    v_ = vld1q_u16(data);
  }

  // Broadcasts i to all lanes (usually by loading from memory).
  HH_INLINE explicit V128(T i) : v_(vdupq_n_u16(i)) {}

  // Copy from other vector.
  HH_INLINE explicit V128(const V128& other) : v_(other.v_) {}
  template <typename U>
  HH_INLINE explicit V128(const V128<U>& other) : v_(other) {}
  HH_INLINE V128& operator=(const V128& other) {
    v_ = other.v_;
    return *this;
  }

  // Convert from/to intrinsics.
  HH_INLINE V128(const Intrinsic& v) : v_(v) {}
  HH_INLINE V128& operator=(const Intrinsic& v) {
    v_ = v;
    return *this;
  }
  HH_INLINE operator Intrinsic() const { return v_; }

  // There are no greater-than comparison instructions for unsigned T.
  HH_INLINE V128 operator==(const V128& other) const {
    return V128(vceqq_u16(v_, other.v_));
  }

  HH_INLINE V128& operator+=(const V128& other) {
    v_ = vaddq_u16(v_, other.v_);
    return *this;
  }
  HH_INLINE V128& operator-=(const V128& other) {
    v_ = vsubq_u16(v_, other.v_);
    return *this;
  }

  HH_INLINE V128& operator&=(const V128& other) {
    v_ = vandq_u16(v_, other.v_);
    return *this;
  }
  HH_INLINE V128& operator|=(const V128& other) {
    v_ = vorrq_u16(v_, other.v_);
    return *this;
  }
  HH_INLINE V128& operator^=(const V128& other) {
    v_ = veorq_u16(v_, other.v_);
    return *this;
  }

  HH_INLINE V128& operator<<=(const int count) {
    switch (count) {
      INTRINSIC_EACH_16(vshlq_n_u16, v_)
    }
    return *this;
  }
  HH_INLINE V128& operator<<=(const Intrinsic& count) {
    v_ = vshlq_u16(v_, vreinterpretq_s16_u16(count));
    return *this;
  }

  HH_INLINE V128& operator>>=(const int count) {
    switch (count) {
      INTRINSIC_EACH_16(vshrq_n_u16, v_)
    }
    return *this;
  }
  HH_INLINE V128& operator>>=(const Intrinsic& count) {
    v_ = vshlq_u16(v_, vnegq_s16(vreinterpretq_s16_u16(count)));
    return *this;
  }

  HH_INLINE V128& ShiftRightInsert(const Intrinsic &value, const int count) {
    switch (count) {
      INTRINSIC_EACH_16(vsriq_n_u16, v_, value)
    }
    return *this;
  }

  HH_INLINE V128& AndNot(const Intrinsic &neg_mask) {
    v_ = vbicq_u16(v_, neg_mask);
    return *this;
  }
 private:
  Intrinsic v_;
};

template <>
class V128<uint32_t> {
 public:
  using Intrinsic = uint32x4_t;
  using T = uint32_t;
  static constexpr size_t N = 4;

  // Leaves v_ uninitialized - typically used for output parameters.
  HH_INLINE V128() {}

  // Lane 0 (p_0) is the lowest.
  HH_INLINE V128(T p_3, T p_2, T p_1, T p_0) {
    alignas(16) const T data[4] = {
      p_0, p_1, p_2, p_3
    };
    v_ = vld1q_u32(data);
  }

  // Broadcasts i to all lanes (usually by loading from memory).
  HH_INLINE explicit V128(T i) : v_(vdupq_n_u32(i)) {}

  // Copy from other vector.
  HH_INLINE explicit V128(const V128& other) : v_(other.v_) {}
  template <typename U>
  HH_INLINE explicit V128(const V128<U>& other) : v_((const uint32x4_t)other) {}
  HH_INLINE V128& operator=(const V128& other) {
    v_ = other.v_;
    return *this;
  }

  // Convert from/to intrinsics.
  HH_INLINE V128(const Intrinsic& v) : v_(v) {}
  HH_INLINE V128& operator=(const Intrinsic& v) {
    v_ = v;
    return *this;
  }
  HH_INLINE operator Intrinsic() const { return v_; }

  // There are no greater-than comparison instructions for unsigned T.
  HH_INLINE V128 operator==(const V128& other) const {
    return V128(vceqq_u32(v_, other.v_));
  }

  HH_INLINE V128& operator+=(const V128& other) {
    v_ = vaddq_u32(v_, other.v_);
    return *this;
  }
  HH_INLINE V128& operator-=(const V128& other) {
    v_ = vsubq_u32(v_, other.v_);
    return *this;
  }

  HH_INLINE V128& operator&=(const V128& other) {
    v_ = vandq_u32(v_, other.v_);
    return *this;
  }
  HH_INLINE V128& operator|=(const V128& other) {
    v_ = vorrq_u32(v_, other.v_);
    return *this;
  }
  HH_INLINE V128& operator^=(const V128& other) {
    v_ = veorq_u32(v_, other.v_);
    return *this;
  }

  HH_INLINE V128& operator<<=(const int count) {
    switch (count) {
      INTRINSIC_EACH_32(vshlq_n_u32, v_)
    }
    return *this;
  }
  HH_INLINE V128& operator<<=(const Intrinsic& count) {
    v_ = vshlq_u32(v_, vreinterpretq_s32_u32(count));
    return *this;
  }

  HH_INLINE V128& operator>>=(const int count) {
    switch (count) {
      INTRINSIC_EACH_32(vshrq_n_u32, v_)
    }
    return *this;
  }
  HH_INLINE V128& operator>>=(const Intrinsic& count) {
    v_ = vshlq_u32(v_, vnegq_s32(vreinterpretq_s32_u32(count)));
    return *this;
  }

  HH_INLINE V128& ShiftRightInsert(const Intrinsic &value, const int count) {
    switch (count) {
      INTRINSIC_EACH_32(vsriq_n_u32, v_, value)
    }
    return *this;
  }

  HH_INLINE V128& AndNot(const Intrinsic &neg_mask) {
    v_ = vbicq_u32(v_, neg_mask);
    return *this;
  }
 private:
  Intrinsic v_;
};

template <>
class V128<uint64_t> {
 public:
  using Intrinsic = uint64x2_t;
  using T = uint64_t;
  static constexpr size_t N = 2;

  // Leaves v_ uninitialized - typically used for output parameters.
  HH_INLINE V128() {}

  // Lane 0 (p_0) is the lowest.
  HH_INLINE V128(T p_1, T p_0) {
    alignas(16) const T data[2] = {
      p_0, p_1
    };
    v_ = vld1q_u64(data);
  }

  // Broadcasts i to all lanes (usually by loading from memory).
  HH_INLINE explicit V128(T i) : v_(vdupq_n_u64(i)) {}

  // Copy from other vector.
  HH_INLINE explicit V128(const V128& other) : v_(other.v_) {}
  template <typename U>
  HH_INLINE explicit V128(const V128<U>& other) : v_(other) {}
  HH_INLINE V128& operator=(const V128& other) {
    v_ = other.v_;
    return *this;
  }

  // Convert from/to intrinsics.
  HH_INLINE V128(const Intrinsic& v) : v_(v) {}
  HH_INLINE V128& operator=(const Intrinsic& v) {
    v_ = v;
    return *this;
  }
  HH_INLINE operator Intrinsic() const { return v_; }

  // There are no greater-than comparison instructions for unsigned T.
  HH_INLINE V128 operator==(const V128& other) const {
    return V128(vceqq_u64(v_, other.v_));
  }

  HH_INLINE V128& operator+=(const V128& other) {
    v_ = vaddq_u64(v_, other.v_);
    // Prevent Clang from converting to vaddhn when nearby vmovn, which
    // causes four spills in the main loop on ARMv7a.
#ifdef __GNUC__
    __asm__("" : "+w" (v_));
#endif
    return *this;
  }
  HH_INLINE V128& operator-=(const V128& other) {
    v_ = vsubq_u64(v_, other.v_);
    return *this;
  }

  HH_INLINE V128& operator&=(const V128& other) {
    v_ = vandq_u64(v_, other.v_);
    return *this;
  }
  HH_INLINE V128& operator|=(const V128& other) {
    v_ = vorrq_u64(v_, other.v_);
    return *this;
  }
  HH_INLINE V128& operator^=(const V128& other) {
    v_ = veorq_u64(v_, other.v_);
    return *this;
  }

  HH_INLINE V128& operator<<=(const int count) {
    switch (count) {
      INTRINSIC_EACH_64(vshlq_n_u64, v_)
    }
    return *this;
  }
  HH_INLINE V128& operator<<=(const Intrinsic& count) {
    v_ = vshlq_u64(v_, vreinterpretq_s64_u64(count));
    return *this;
  }

  HH_INLINE V128& operator>>=(const int count) {
    switch (count) {
      INTRINSIC_EACH_64(vshrq_n_u64, v_)
    }
    return *this;
  }
  HH_INLINE V128& operator>>=(const Intrinsic& count) {
    v_ = vshlq_u64(v_, vnegq_s64(vreinterpretq_s64_u64(count)));
    return *this;
  }

  HH_INLINE V128& AndNot(const Intrinsic &neg_mask) {
    v_ = vbicq_u64(v_, neg_mask);
    return *this;
  }

  HH_INLINE V128& ShiftRightInsert(const Intrinsic &value, const int count) {
    switch (count) {
      INTRINSIC_EACH_64(vsriq_n_u64, v_, value)
    }
    return *this;
  }

 private:
  Intrinsic v_;
};

// TODO: Enable. For now, this is disabled for the following reasons:
//   1. ARMv7a lacks float64x2_t.
//   2. ARMv7a's float32x4_t is not IEE-754 compliant
//   3. We don't actually use the float vectors right now.
#if 0
template <>
class V128<float> {
 public:
  using Intrinsic = float32x4_t;
  using T = float;
  static constexpr size_t N = 4;

  // Leaves v_ uninitialized - typically used for output parameters.
  HH_INLINE V128() {}

  // Lane 0 (p_0) is the lowest.
  HH_INLINE V128(T p_3, T p_2, T p_1, T p_0) {
    HH_ALIGNAS(16) float tmp[4] = { p_0, p_1, p_2, p_3 };
    v_ = vld1q_f32(tmp);
  }

  // Broadcasts to all lanes.
  HH_INLINE explicit V128(T f) : v_(vdupq_n_f32(f)) {}

  // Copy from other vector.
  HH_INLINE explicit V128(const V128& other) : v_(other.v_) {}
  template <typename U>
  HH_INLINE explicit V128(const V128<U>& other) : v_(other) {}
  HH_INLINE V128& operator=(const V128& other) {
    v_ = other.v_;
    return *this;
  }

  // Convert from/to intrinsics.
  HH_INLINE V128(const Intrinsic& v) : v_(v) {}
  HH_INLINE V128& operator=(const Intrinsic& v) {
    v_ = v;
    return *this;
  }
  HH_INLINE operator Intrinsic() const { return v_; }

  HH_INLINE V128 operator==(const V128& other) const {
    return V128(vceqq_f32(v_, other.v_));
  }
  HH_INLINE V128 operator<(const V128& other) const {
    return V128(vcltq_f32(v_, other.v_));
  }
  HH_INLINE V128 operator>(const V128& other) const {
    return V128(vcltq_f32(other.v_, v_));
  }

  HH_INLINE V128& operator*=(const V128& other) {
    v_ = vmulq_f32(v_, other.v_);
    return *this;
  }
  HH_INLINE V128& operator/=(const V128& other) {
    v_ = vdivq_f32(v_, other.v_);
    return *this;
  }
  HH_INLINE V128& operator+=(const V128& other) {
    v_ = vaddq_f32(v_, other.v_);
    return *this;
  }
  HH_INLINE V128& operator-=(const V128& other) {
    v_ = vsubq_f32(v_, other.v_);
    return *this;
  }

  HH_INLINE V128& operator&=(const V128& other) {
    v_ = vreinterpretq_f32_u32(vandq_u32(vreinterpretq_u32_f32(v_), vreinterpretq_u32_f32(other.v_)));
    return *this;
  }
  HH_INLINE V128& operator|=(const V128& other) {
    v_ = vreinterpretq_f32_u32(vorrq_u32(vreinterpretq_u32_f32(v_), vreinterpretq_u32_f32(other.v_)));
    return *this;
  }
  HH_INLINE V128& operator^=(const V128& other) {
    v_ = vreinterpretq_f32_u32(veorq_u32(vreinterpretq_u32_f32(v_), vreinterpretq_u32_f32(other.v_)));
    return *this;
  }

 private:
  Intrinsic v_;
};

template <>
class V128<double> {
 public:
  using Intrinsic = float64x2_t;
  using T = double;
  static constexpr size_t N = 2;

  // Leaves v_ uninitialized - typically used for output parameters.
  HH_INLINE V128() {}

  // Lane 0 (p_0) is the lowest.
  HH_INLINE V128(T p_1, T p_0) {
    HH_ALIGNAS(16) double tmp[2] = { p_0, p_1 };
    v_ = vld1q_f64(tmp);
  }

  // Broadcasts to all lanes.
  HH_INLINE explicit V128(T f) : v_(vdupq_n_f64(f)) {}

  // Copy from other vector.
  HH_INLINE explicit V128(const V128& other) : v_(other.v_) {}
  template <typename U>
  HH_INLINE explicit V128(const V128<U>& other) : v_(other) {}
  HH_INLINE V128& operator=(const V128& other) {
    v_ = other.v_;
    return *this;
  }

  // Convert from/to intrinsics.
  HH_INLINE V128(const Intrinsic& v) : v_(v) {}
  HH_INLINE V128& operator=(const Intrinsic& v) {
    v_ = v;
    return *this;
  }
  HH_INLINE operator Intrinsic() const { return v_; }

  HH_INLINE V128 operator==(const V128& other) const {
    return V128(vceqq_f64(v_, other.v_));
  }
  HH_INLINE V128 operator<(const V128& other) const {
    return V128(vcltq_f64(v_, other.v_));
  }
  HH_INLINE V128 operator>(const V128& other) const {
    return V128(vcltq_f64(other.v_, v_));
  }

  HH_INLINE V128& operator*=(const V128& other) {
    v_ = vmulq_f64(v_, other.v_);
    return *this;
  }
  HH_INLINE V128& operator/=(const V128& other) {
    v_ = vdivq_f64(v_, other.v_);
    return *this;
  }
  HH_INLINE V128& operator+=(const V128& other) {
    v_ = vaddq_f64(v_, other.v_);
    return *this;
  }
  HH_INLINE V128& operator-=(const V128& other) {
    v_ = vsubq_f64(v_, other.v_);
    return *this;
  }

  HH_INLINE V128& operator&=(const V128& other) {
    v_ = vreinterpretq_f64_u64(vandq_u64(vreinterpretq_u64_f64(v_), vreinterpretq_u64_f64(other.v_)));
    return *this;
  }
  HH_INLINE V128& operator|=(const V128& other) {
    v_ = vreinterpretq_f64_u64(vorrq_u64(vreinterpretq_u64_f64(v_), vreinterpretq_u64_f64(other.v_)));
    return *this;
  }
  HH_INLINE V128& operator^=(const V128& other) {
    v_ = vreinterpretq_f64_u64(veorq_u64(vreinterpretq_u64_f64(v_), vreinterpretq_u64_f64(other.v_)));
    return *this;
  }

 private:
  Intrinsic v_;
};
#endif

// Nonmember functions for any V128 via member functions.

template <typename T>
HH_INLINE V128<T> operator*(const V128<T>& left, const V128<T>& right) {
  V128<T> t(left);
  return t *= right;
}

template <typename T>
HH_INLINE V128<T> operator/(const V128<T>& left, const V128<T>& right) {
  V128<T> t(left);
  return t /= right;
}

template <typename T>
HH_INLINE V128<T> operator+(const V128<T>& left, const V128<T>& right) {
  V128<T> t(left);
  return t += right;
}

template <typename T>
HH_INLINE V128<T> operator-(const V128<T>& left, const V128<T>& right) {
  V128<T> t(left);
  return t -= right;
}

template <typename T>
HH_INLINE V128<T> operator&(const V128<T>& left, const V128<T>& right) {
  V128<T> t(left);
  return t &= right;
}

template <typename T>
HH_INLINE V128<T> operator|(const V128<T>& left, const V128<T>& right) {
  V128<T> t(left);
  return t |= right;
}

template <typename T>
HH_INLINE V128<T> operator^(const V128<T>& left, const V128<T>& right) {
  V128<T> t(left);
  return t ^= right;
}

template <typename T>
HH_INLINE V128<T> operator<<(const V128<T>& v, const int count) {
  V128<T> t(v);
  return t <<= count;
}

template <typename T>
HH_INLINE V128<T> operator>>(const V128<T>& v, const int count) {
  V128<T> t(v);
  return t >>= count;
}

using V16x8U = V128<uint8_t>;
using V8x16U = V128<uint16_t>;
using V4x32U = V128<uint32_t>;
using V2x64U = V128<uint64_t>;
using V4x32F = V128<float>;
using V2x64F = V128<double>;

// Load/Store for any V128.

// We differentiate between targets' vector types via template specialization.
// Calling Load<V>(floats) is more natural than Load(V8x32F(), floats) and may
// generate better code in unoptimized builds. Only declare the primary
// templates to avoid needing mutual exclusion with vector256.

template <class V>
HH_INLINE V Load(const typename V::T* const HH_RESTRICT from);

template <class V>
HH_INLINE V LoadUnaligned(const typename V::T* const HH_RESTRICT from);

// "from" must be vector-aligned.
#ifdef __GNUC__
#define HH_ALIGN(x) __builtin_assume_aligned((x), 16)
#else
#define HH_ALIGN(x) (x)
#endif
template <>
HH_INLINE V16x8U Load<V16x8U>(const V16x8U::T* const HH_RESTRICT from) {
  const uint8_t* const HH_RESTRICT p = reinterpret_cast<const uint8_t*>(HH_ALIGN(from));
  return V16x8U(vld1q_u8(p));
}
template <>
HH_INLINE V8x16U Load<V8x16U>(const V8x16U::T* const HH_RESTRICT from) {
  const uint16_t* const HH_RESTRICT p = reinterpret_cast<const uint16_t*>(HH_ALIGN(from));
  return V8x16U(vld1q_u16(p));
}
template <>
HH_INLINE V4x32U Load<V4x32U>(const V4x32U::T* const HH_RESTRICT from) {
  const uint32_t* const HH_RESTRICT p = reinterpret_cast<const uint32_t*>(HH_ALIGN(from));
  return V4x32U(vld1q_u32(p));
}
template <>
HH_INLINE V2x64U Load<V2x64U>(const V2x64U::T* const HH_RESTRICT from) {
  const uint64_t* const HH_RESTRICT p = reinterpret_cast<const uint64_t*>(HH_ALIGN(from));
  return V2x64U(vld1q_u64(p));
}
#if 0
template <>
HH_INLINE V4x32F Load<V4x32F>(const V4x32F::T* const HH_RESTRICT from) {
  return V4x32F(vld1q_f32(from));
}
template <>
HH_INLINE V2x64F Load<V2x64F>(const V2x64F::T* const HH_RESTRICT from) {
  return V2x64F(vld1q_f64(from));
}
#endif
// GCC for ARM 32-bit flips out on unaligned reads after a cast.
// Only vld1q_u8 is safe on unaligned pointers.
template <>
HH_INLINE V16x8U
LoadUnaligned<V16x8U>(const V16x8U::T* const HH_RESTRICT from) {
  const uint8_t* const HH_RESTRICT p = reinterpret_cast<const uint8_t*>(from);
  return V16x8U(vld1q_u8(p));
}
template <>
HH_INLINE V8x16U
LoadUnaligned<V8x16U>(const V8x16U::T* const HH_RESTRICT from) {
  const uint8_t* const HH_RESTRICT p = reinterpret_cast<const uint8_t*>(from);
  return V8x16U(vreinterpretq_u16_u8(vld1q_u8(p)));
}
template <>
HH_INLINE V4x32U
LoadUnaligned<V4x32U>(const V4x32U::T* const HH_RESTRICT from) {
  const uint8_t* const HH_RESTRICT p = reinterpret_cast<const uint8_t*>(from);
  return V4x32U(vreinterpretq_u32_u8(vld1q_u8(p)));
}
template <>
HH_INLINE V2x64U
LoadUnaligned<V2x64U>(const V2x64U::T* const HH_RESTRICT from) {
  const uint8_t* const HH_RESTRICT p = reinterpret_cast<const uint8_t*>(from);
  return V2x64U(vreinterpretq_u64_u8(vld1q_u8(p)));
}
#if 0
template <>
HH_INLINE V4x32F
LoadUnaligned<V4x32F>(const V4x32F::T* const HH_RESTRICT from) {
  return V4x32F(vld1q_f32(from));
}
template <>
HH_INLINE V2x64F
LoadUnaligned<V2x64F>(const V2x64F::T* const HH_RESTRICT from) {
  return V2x64F(vld1q_f64(from));
}
#endif

// "to" must be vector-aligned.
template <typename T>
HH_INLINE void Store(const V128<T>& v, T* const HH_RESTRICT to);

template<>
HH_INLINE void Store<uint8_t>(const V128<uint8_t>& v, uint8_t* HH_RESTRICT to) {
  uint8_t *const HH_RESTRICT p = reinterpret_cast<uint8_t *>(HH_ALIGN(to));
  vst1q_u8(p, v);
}

template<>
HH_INLINE void Store<uint16_t>(const V128<uint16_t>& v, uint16_t* const HH_RESTRICT to) {
  uint16_t *const HH_RESTRICT p = reinterpret_cast<uint16_t *>(HH_ALIGN(to));
  vst1q_u16(p, v);
}

template<>
HH_INLINE void Store<uint32_t>(const V128<uint32_t>& v, uint32_t* const HH_RESTRICT to) {
  uint32_t *const HH_RESTRICT p = reinterpret_cast<uint32_t *>(HH_ALIGN(to));
  vst1q_u32(p, v);
}

template<>
HH_INLINE void Store<uint64_t>(const V128<uint64_t>& v, uint64_t* const HH_RESTRICT to) {
  uint64_t *const HH_RESTRICT p = reinterpret_cast<uint64_t *>(HH_ALIGN(to));
  vst1q_u64(p, v);
}
#undef HH_ALIGN

#if 0
HH_INLINE void Store(const V128<float>& v, float* const HH_RESTRICT to) {
  vst1q_f32(to, v);
}
HH_INLINE void Store(const V128<double>& v, double* const HH_RESTRICT to) {
  vst1q_f64(to, v);
}
#endif
template <typename T>
HH_INLINE void StoreUnaligned(const V128<T>& v, T* const HH_RESTRICT to);

template<>
HH_INLINE void StoreUnaligned<uint8_t>(const V128<uint8_t>& v, uint8_t* const HH_RESTRICT to) {
  vst1q_u8(to, v);
}

template<>
HH_INLINE void StoreUnaligned<uint16_t>(const V128<uint16_t>& v, uint16_t* const HH_RESTRICT to) {
  vst1q_u8(reinterpret_cast<uint8_t *>(to), vreinterpretq_u8_u16(v));
}

template<>
HH_INLINE void StoreUnaligned<uint32_t>(const V128<uint32_t>& v, uint32_t* const HH_RESTRICT to) {
  vst1q_u8(reinterpret_cast<uint8_t *>(to), vreinterpretq_u8_u32(v));
}

template<>
HH_INLINE void StoreUnaligned<uint64_t>(const V128<uint64_t>& v, uint64_t* const HH_RESTRICT to) {
  vst1q_u8(reinterpret_cast<uint8_t *>(to), vreinterpretq_u8_u64(v));
}
#if 0
HH_INLINE void StoreUnaligned(const V128<float>& v,
                              float* const HH_RESTRICT to) {
  _mm_storeu_ps(to, v);
}
HH_INLINE void StoreUnaligned(const V128<double>& v,
                              double* const HH_RESTRICT to) {
  _mm_storeu_pd(to, v);
}
#endif

// TODO: Enable.
#if 0
// Writes directly to (aligned) memory, bypassing the cache. This is useful for
// data that will not be read again in the near future.
template <typename T>
HH_INLINE void Stream(const V128<T>& v, T* const HH_RESTRICT to) {
  _mm_stream_si128(reinterpret_cast<__m128i * HH_RESTRICT>(to), v);
}
HH_INLINE void Stream(const V128<float>& v, float* const HH_RESTRICT to) {
  _mm_stream_ps(to, v);
}
HH_INLINE void Stream(const V128<double>& v, double* const HH_RESTRICT to) {
  _mm_stream_pd(to, v);
}
#endif

// Miscellaneous functions.

template <typename T>
HH_INLINE V128<T> RotateLeft(const V128<T>& v, const int count) {
  const size_t num_bits = sizeof(T) * 8;
  const V128<T>& tmp = v << count;
  return tmp.ShiftRightAccumulate(v, num_bits - count);
}

template <typename T>
HH_INLINE V128<T> AndNot(const V128<T>& neg_mask, const V128<T>& values) {
  V128<T> tmp = values;
  return tmp.AndNot(neg_mask);
}
#if 0
template <>
HH_INLINE V128<float> AndNot(const V128<float>& neg_mask,
                             const V128<float>& values) {
  return V128<float>(_mm_andnot_ps(neg_mask, values));
}
template <>
HH_INLINE V128<double> AndNot(const V128<double>& neg_mask,
                              const V128<double>& values) {
  return V128<double>(_mm_andnot_pd(neg_mask, values));
}
#endif
HH_INLINE V4x32U Select(const V4x32U& a, const V4x32U& b, const V4x32U& mask) {
  return V4x32U(vbslq_u32(mask, a, b));
}

HH_INLINE V2x64U Select(const V2x64U& a, const V2x64U& b, const V2x64U& mask) {
  return V2x64U(vbslq_u64(mask, a, b));
}

// Min/Max

HH_INLINE V16x8U Min(const V16x8U& v0, const V16x8U& v1) {
  return V16x8U(vminq_u8(v0, v1));
}

HH_INLINE V16x8U Max(const V16x8U& v0, const V16x8U& v1) {
  return V16x8U(vmaxq_u8(v0, v1));
}

HH_INLINE V8x16U Min(const V8x16U& v0, const V8x16U& v1) {
  return V8x16U(vminq_u16(v0, v1));
}

HH_INLINE V8x16U Max(const V8x16U& v0, const V8x16U& v1) {
  return V8x16U(vmaxq_u16(v0, v1));
}

HH_INLINE V4x32U Min(const V4x32U& v0, const V4x32U& v1) {
  return V4x32U(vminq_u32(v0, v1));
}

HH_INLINE V4x32U Max(const V4x32U& v0, const V4x32U& v1) {
  return V4x32U(vmaxq_u32(v0, v1));
}


#if 0
HH_INLINE V4x32F Min(const V4x32F& v0, const V4x32F& v1) {
  return V4x32F(vminq_f32(v0, v1));
}

HH_INLINE V4x32F Max(const V4x32F& v0, const V4x32F& v1) {
  return V4x32F(vmaxq_f32(v0, v1));
}

HH_INLINE V2x64F Min(const V2x64F& v0, const V2x64F& v1) {
  return V2x64F(vminq_f64(v0, v1));
}

HH_INLINE V2x64F Max(const V2x64F& v0, const V2x64F& v1) {
  return V2x64F(vmaxq_f64(v0, v1));
}
#endif

}  // namespace HH_TARGET_NAME
}  // namespace highwayhash

#endif  // HH_DISABLE_TARGET_SPECIFIC
#endif  // HIGHWAYHASH_VECTOR128_H_
