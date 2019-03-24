// Copyright 2015-2019 Google Inc. All Rights Reserved.
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

#ifndef HIGHWAYHASH_HH_NEON_H_
#define HIGHWAYHASH_HH_NEON_H_

// WARNING: this is a "restricted" header because it is included from
// translation units compiled with different flags. This header and its
// dependencies must not define any function unless it is static inline and/or
// within namespace HH_TARGET_NAME. See arch_specific.h for details.

#include "highwayhash/arch_specific.h"
#include "highwayhash/compiler_specific.h"
#include "highwayhash/hh_buffer.h"
#include "highwayhash/hh_types.h"
#include "highwayhash/load3.h"
#include "highwayhash/vector_neon.h"

// For auto-dependency generation, we need to include all headers but not their
// contents.
#ifndef HH_DISABLE_TARGET_SPECIFIC

namespace highwayhash {


// See vector_neon.h for why this namespace is necessary; matching it here makes
// it easier use the vector_neon symbols, but requires textual inclusion.
namespace HH_TARGET_NAME {

// J-lanes tree hashing: see https://doi.org/10.4236/jis.2014.53010
// Uses the same method that SSE4.1 uses, only with NEON used instead.
class HHStateNEON {
 public:
  explicit HH_INLINE HHStateNEON(const HHKey key) { Reset(key); }

  HH_INLINE void Reset(const HHKey key) {
    // "Nothing up my sleeve numbers"; see HHStateTAVX2.
    const V2x64U init0L(0xa4093822299f31d0ull, 0xdbe6d5d5fe4cce2full);
    const V2x64U init0H(0x243f6a8885a308d3ull, 0x13198a2e03707344ull);
    const V2x64U init1L(0xc0acf169b5f18a8cull, 0x3bd39e10cb0ef593ull);
    const V2x64U init1H(0x452821e638d01377ull, 0xbe5466cf34e90c6cull);
    const V2x64U keyL = LoadUnaligned<V2x64U>(key + 0);
    const V2x64U keyH = LoadUnaligned<V2x64U>(key + 2);
    v0L = keyL ^ init0L;
    v0H = keyH ^ init0H;
    v1L = Rotate64By32(keyL) ^ init1L;
    v1H = Rotate64By32(keyH) ^ init1H;
    mul0L = init0L;
    mul0H = init0H;
    mul1L = init1L;
    mul1H = init1H;
  }

  HH_INLINE void Update(const HHPacket& packet_bytes) {
    const uint64_t* HH_RESTRICT packet =
        reinterpret_cast<const uint64_t * HH_RESTRICT>(packet_bytes);
    const V2x64U packetL = LoadUnaligned<V2x64U>(packet + 0);
    const V2x64U packetH = LoadUnaligned<V2x64U>(packet + 2);
    Update(packetH, packetL);
  }

  HH_INLINE void UpdateRemainder(const char* bytes, const size_t size_mod32) {
    // 'Length padding' differentiates zero-valued inputs that have the same
    // size/32. mod32 is sufficient because each Update behaves as if a
    // counter were injected, because the state is large and mixed thoroughly.

    // We can't use vshl/vsra because it needs a constant expression.
    // In order to do this right now, we would need a switch statement.
    const int32x4_t vsize_mod32(vdupq_n_s32(static_cast<int32_t>(size_mod32)));
    // -32 - size_mod32
    const int32x4_t shift_right_amt = vdupq_n_s32(static_cast<int32_t>(size_mod32) + (~32 + 1));
    // Equivalent to storing size_mod32 in packet.
    v0L += V2x64U(vreinterpretq_u64_s32(vsize_mod32));
    v0H += V2x64U(vreinterpretq_u64_s32(vsize_mod32));

    // Boosts the avalanche effect of mod32.
    v1L = V2x64U(vreinterpretq_u64_u32(
      vorrq_u32(
        vshlq_u32(vreinterpretq_u32_u64(v1L), vsize_mod32),
        vshlq_u32(vreinterpretq_u32_u64(v1L), shift_right_amt)
      )
    ));
    v1H = V2x64U(vreinterpretq_u64_u32(
      vorrq_u32(
        vshlq_u32(vreinterpretq_u32_u64(v1H), vsize_mod32),
        vshlq_u32(vreinterpretq_u32_u64(v1H), shift_right_amt)
      )
    ));

    const size_t size_mod4 = size_mod32 & 3;
    const char* HH_RESTRICT remainder = bytes + (size_mod32 & ~3);

    if (HH_UNLIKELY(size_mod32 & 16)) {  // 16..31 bytes left
      const V2x64U packetL =
          LoadUnaligned<V2x64U>(reinterpret_cast<const uint64_t*>(bytes));

      V2x64U packetH = LoadMultipleOfFour(bytes + 16, size_mod32);

      const uint32_t last4 =
          Load3()(Load3::AllowReadBeforeAndReturn(), remainder, size_mod4);

      // The upper four bytes of packetH are zero, so insert there.
      packetH = V2x64U(
        vreinterpretq_u64_u32(
          vsetq_lane_u32(last4, vreinterpretq_u32_u64(packetH), 3)
        )
      );
      Update(packetH, packetL);
    } else {  // size_mod32 < 16
      const V2x64U packetL = LoadMultipleOfFour(bytes, size_mod32);

      const uint64_t last4 =
          Load3()(Load3::AllowUnordered(), remainder, size_mod4);

      // Rather than insert into packetL[3], it is faster to initialize
      // the otherwise empty packetH.
      HH_ALIGNAS(16) uint64_t tmp[2] = { last4, 0 };
      const V2x64U packetH(vld1q_u64(tmp));
      Update(packetH, packetL);
    }
  }

  HH_INLINE void Finalize(HHResult64* HH_RESTRICT result) {
    // Mix together all lanes.
    for (int n = 0; n < 4; n++) {
      PermuteAndUpdate();
    }

    const V2x64U sum0 = v0L + mul0L;
    const V2x64U sum1 = v1L + mul1L;
    const V2x64U hash = sum0 + sum1;
    vst1q_low_u64(reinterpret_cast<uint64_t*>(result), hash);
  }

  HH_INLINE void Finalize(HHResult128* HH_RESTRICT result) {
    for (int n = 0; n < 6; n++) {
      PermuteAndUpdate();
    }

    const V2x64U sum0 = v0L + mul0L;
    const V2x64U sum1 = v1H + mul1H;
    const V2x64U hash = sum0 + sum1;
    StoreUnaligned(hash, &(*result)[0]);
  }

  HH_INLINE void Finalize(HHResult256* HH_RESTRICT result) {
    for (int n = 0; n < 10; n++) {
      PermuteAndUpdate();
    }

    const V2x64U sum0L = v0L + mul0L;
    const V2x64U sum1L = v1L + mul1L;
    const V2x64U sum0H = v0H + mul0H;
    const V2x64U sum1H = v1H + mul1H;
    const V2x64U hashL = ModularReduction(sum1L, sum0L);
    const V2x64U hashH = ModularReduction(sum1H, sum0H);
    StoreUnaligned(hashL, &(*result)[0]);
    StoreUnaligned(hashH, &(*result)[2]);
  }

  static HH_INLINE void ZeroInitialize(char* HH_RESTRICT buffer_bytes) {
    for (size_t i = 0; i < sizeof(HHPacket); ++i) {
      buffer_bytes[i] = 0;
    }
  }

  static HH_INLINE void CopyPartial(const char* HH_RESTRICT from,
                                    const size_t size_mod32,
                                    char* HH_RESTRICT buffer) {
    for (size_t i = 0; i < size_mod32; ++i) {
      buffer[i] = from[i];
    }
  }

  static HH_INLINE void AppendPartial(const char* HH_RESTRICT from,
                                      const size_t size_mod32,
                                      char* HH_RESTRICT buffer,
                                      const size_t buffer_valid) {
    for (size_t i = 0; i < size_mod32; ++i) {
      buffer[buffer_valid + i] = from[i];
    }
  }

  HH_INLINE void AppendAndUpdate(const char* HH_RESTRICT from,
                                 const size_t size_mod32,
                                 const char* HH_RESTRICT buffer,
                                 const size_t buffer_valid) {
    HHPacket tmp HH_ALIGNAS(32);
    for (size_t i = 0; i < buffer_valid; ++i) {
      tmp[i] = buffer[i];
    }
    for (size_t i = 0; i < size_mod32; ++i) {
      tmp[buffer_valid + i] = from[i];
    }
    Update(tmp);
  }

 private:
  // Swap 32-bit halves of each lane (caller swaps 128-bit halves)
  static HH_INLINE V2x64U Rotate64By32(const V2x64U& v) {
    return V2x64U(
      vreinterpretq_u64_u32(
        vrev64q_u32(
          vreinterpretq_u32_u64(v)
        )
      )
    );
  }

  // Rotates 32-bit lanes by "count" bits.
  static HH_INLINE void Rotate32By(V2x64U* HH_RESTRICT vH,
                                   V2x64U* HH_RESTRICT vL,
                                   const uint64_t count) {
    // WARNING: the shift count is 64 bits, so we can't reuse vsize_mod32,
    // which is broadcast into 32-bit lanes.
    *vL = V2x64U((V4x32U(*vL) << int(count)).ShiftRightInsert(*vL, 32-count));
    *vH = V2x64U((V4x32U(*vH) << int(count)).ShiftRightInsert(*vH, 32-count));
  }

  static HH_INLINE V2x64U ZipperMerge(const V2x64U& v) {
    // Multiplication mixes/scrambles bytes 0-7 of the 64-bit result to
    // varying degrees. In descending order of goodness, bytes
    // 3 4 2 5 1 6 0 7 have quality 228 224 164 160 100 96 36 32.
    // As expected, the upper and lower bytes are much worse.
    // For each 64-bit lane, our objectives are:
    // 1) maximizing and equalizing total goodness across each lane's bytes;
    // 2) mixing with bytes from the neighboring lane;
    // 3) placing the worst bytes in the upper 32 bits because those will not
    //    be used in the next 32x32 multiplication.

    // The positions of each byte in the new vector.
    const uint8_t shuffle_positions[] = {
       3, 12,  2,  5, 14, 1, 15, 0,
       11, 4, 10, 13,  9, 6,  8, 7
    };
    const uint8x16_t tbl = vld1q_u8(shuffle_positions);

    // Note: vqtbl1q_u8 is polyfilled for ARMv7a in vector_neon.h.
    return V2x64U(
      vreinterpretq_u64_u8(
        vqtbl1q_u8(
          vreinterpretq_u8_u64(v),
          tbl
        )
      )
    );
  }

  HH_INLINE void Update(const V2x64U& packetH, const V2x64U& packetL) {
    v1L += packetL;
    v1H += packetH;
    v1L += mul0L;
    v1H += mul0H;
    // mul0L ^= (v1L & 0xFFFFFFFF) * (v0L >> 32);
    mul0L ^= V2x64U(vmull_u32(vmovn_u64(v1L), vshrn_n_u64(v0L, 32)));
    // mul0H ^= (v1H & 0xFFFFFFFF) * (v0H >> 32);
    mul0H ^= V2x64U(vmull_u32(vmovn_u64(v1H), vshrn_n_u64(v0H, 32)));
    v0L += mul1L;
    v0H += mul1H;
    // mul1L ^= (v0L & 0xFFFFFFFF) * (v1L >> 32);
    mul1L ^= V2x64U(vmull_u32(vmovn_u64(v0L), vshrn_n_u64(v1L, 32)));
    // mul1H ^= (v0H & 0xFFFFFFFF) * (v1H >> 32);
    mul1H ^= V2x64U(vmull_u32(vmovn_u64(v0H), vshrn_n_u64(v1H, 32)));
    v0L += ZipperMerge(v1L);
    v0H += ZipperMerge(v1H);
    v1L += ZipperMerge(v0L);
    v1H += ZipperMerge(v0H);
  }

  HH_INLINE void PermuteAndUpdate() {
    // It is slightly better to permute v0 than v1; it will be added to v1.
    Update(Rotate64By32(v0L), Rotate64By32(v0H));
  }

  // Returns zero-initialized vector with the lower "size" = 0, 4, 8 or 12
  // bytes loaded from "bytes". Serves as a replacement for AVX2 maskload_epi32.
  static HH_INLINE V2x64U LoadMultipleOfFour(const char* bytes,
                                             const size_t size) {
    const uint32_t* words = reinterpret_cast<const uint32_t*>(bytes);
    // Mask of 1-bits where the final 4 bytes should be inserted (replacement
    // for variable shift/insert using broadcast+blend).
    alignas(16) const uint64_t mask_pattern[2] = { 0xFFFFFFFFULL, 0 };
    V2x64U mask4(vld1q_u64(mask_pattern));  // 'insert' into lane 0
    V2x64U ret(vdupq_n_u64(0));
    if (size & 8) {
      ret = V2x64U(vld1q_low_u64(reinterpret_cast<const uint64_t*>(words)));
      // mask4 = 0 ~0 0 0 ('insert' into lane 2)
      mask4 = V2x64U(vshlq_n_u128(mask4, 8));
      words += 2;
    }
    // Final 4 (possibly after the 8 above); 'insert' into lane 0 or 2 of ret.
    if (size & 4) {
      // = 0 word2 0 word2; mask4 will select which lane to keep.
      const V2x64U broadcast(vreinterpretq_u64_u32(vdupq_n_u32(words[0])));
      // (slightly faster than blendv_epi8)
      ret |= V2x64U(broadcast & mask4);
    }
    return ret;
  }

  // XORs x << 1 and x << 2 into *out after clearing the upper two bits of x.
  // Bit shifts are only possible on independent 64-bit lanes. We therefore
  // insert the upper bits of x[0] that were lost into x[1].
  // Thanks to D. Lemire for helpful comments!
  static HH_INLINE void XorByShift128Left12(const V2x64U& x,
                                            V2x64U* HH_RESTRICT out) {
    const V4x32U zero(vdupq_n_u32(0));
    const V2x64U sign_bit128(vreinterpretq_u64_u32(vsetq_lane_u32(0x80000000u, zero, 3)));
    const V2x64U top_bits2 = x >> (64 - 2);
    HH_COMPILER_FENCE;
    const V2x64U shifted1_unmasked = x + x;  // (avoids needing port0)

    // Only the lower half of top_bits1 will be used, so we
    // can compute it before clearing the upper two bits of x.
    const V2x64U top_bits1 = x >> (64 - 1);
    const V2x64U shifted2 = shifted1_unmasked + shifted1_unmasked;
    HH_COMPILER_FENCE;

    const V2x64U new_low_bits2(vshlq_n_u128(top_bits2, 8));
    *out ^= shifted2;
    // The result must be as if the upper two bits of the input had been clear,
    // otherwise we're no longer computing a reduction.
    const V2x64U shifted1 = AndNot(sign_bit128, shifted1_unmasked);
    HH_COMPILER_FENCE;

    const V2x64U new_low_bits1(vshlq_n_u128(top_bits1, 8));
    *out ^= new_low_bits2;
    *out ^= shifted1;
    *out ^= new_low_bits1;
  }

  // Modular reduction by the irreducible polynomial (x^128 + x^2 + x).
  // Input: a 256-bit number a3210.
  static HH_INLINE V2x64U ModularReduction(const V2x64U& a32_unmasked,
                                           const V2x64U& a10) {
    // See Lemire, https://arxiv.org/pdf/1503.03465v8.pdf.
    V2x64U out = a10;
    XorByShift128Left12(a32_unmasked, &out);
    return out;
  }

  V2x64U v0L;
  V2x64U v0H;
  V2x64U v1L;
  V2x64U v1H;
  V2x64U mul0L;
  V2x64U mul0H;
  V2x64U mul1L;
  V2x64U mul1H;
};

}  // namespace HH_TARGET_NAME
}  // namespace highwayhash

#endif  // HH_DISABLE_TARGET_SPECIFIC
#endif  // HIGHWAYHASH_HH_NEON_H_
