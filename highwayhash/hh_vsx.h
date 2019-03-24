// Copyright 2015-2017 Google Inc. All Rights Reserved.
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

#ifndef HIGHWAYHASH_HH_VSX_H_
#define HIGHWAYHASH_HH_VSX_H_

// WARNING: this is a "restricted" header because it is included from
// translation units compiled with different flags. This header and its
// dependencies must not define any function unless it is static inline and/or
// within namespace HH_TARGET_NAME. See arch_specific.h for details.

#include "highwayhash/arch_specific.h"
#include "highwayhash/compiler_specific.h"
#include "highwayhash/hh_types.h"
#include "highwayhash/load3.h"

// For auto-dependency generation, we need to include all headers but not their
// contents
#ifndef HH_DISABLE_TARGET_SPECIFIC

#include <altivec.h>
#undef vector
#undef pixel
#undef bool

namespace highwayhash {

typedef __vector unsigned long long PPC_VEC_U64;  // NOLINT
typedef __vector unsigned int PPC_VEC_U32;
typedef __vector unsigned char PPC_VEC_U8;

// See vector128.h for why this namespace is necessary;
namespace HH_TARGET_NAME {

// Helper Functions

// gcc doesn't support vec_mule() and vec_mulo() for vector long.
// Use the generic version, which is defined here only for gcc.

#ifndef __clang__
static HH_INLINE PPC_VEC_U64 vec_mule(PPC_VEC_U32 a, PPC_VEC_U32 b) {  // NOLINT
  PPC_VEC_U64 result;                                                  // NOLINT
#ifdef __LITTLE_ENDIAN__
  asm("vmulouw %0, %1, %2" : "=v"(result) : "v"(a), "v"(b));
#else
  asm("vmuleuw %0, %1, %2" : "=v"(result) : "v"(a), "v"(b));
#endif
  return result;
}
#endif

// LoadUnaligned uses vec_vsx_ld(offset, address) format,
// Offset here is number of bytes and is 0 for this implementation.
static HH_INLINE PPC_VEC_U64
LoadUnaligned(const uint64_t* const HH_RESTRICT from) {
  const PPC_VEC_U64* const HH_RESTRICT p =
      reinterpret_cast<const PPC_VEC_U64*>(from);
  return vec_vsx_ld(0, p);
}

static HH_INLINE void StoreUnaligned(const PPC_VEC_U64& hash,
                                     uint64_t* const HH_RESTRICT to) {
  PPC_VEC_U64* HH_RESTRICT p = reinterpret_cast<PPC_VEC_U64 * HH_RESTRICT>(to);
  vec_vsx_st(hash, 0, p);
}

static HH_INLINE PPC_VEC_U64 MultiplyVectors(const PPC_VEC_U64& vec1,
                                             const PPC_VEC_U64& vec2) {
  return vec_mule(reinterpret_cast<const PPC_VEC_U32>(vec1),
                  reinterpret_cast<const PPC_VEC_U32>(vec2));
}

// J-lanes tree hashing: see https://doi.org/10.4236/jis.2014.53010
class HHStateVSX {
 public:
  explicit HH_INLINE HHStateVSX(const HHKey key) { Reset(key); }

  HH_INLINE void Reset(const HHKey key) {
    // "Nothing up my sleeve numbers";
    const PPC_VEC_U64 init0L = {0xdbe6d5d5fe4cce2full, 0xa4093822299f31d0ull};
    const PPC_VEC_U64 init0H = {0x13198a2e03707344ull, 0x243f6a8885a308d3ull};
    const PPC_VEC_U64 init1L = {0x3bd39e10cb0ef593ull, 0xc0acf169b5f18a8cull};
    const PPC_VEC_U64 init1H = {0xbe5466cf34e90c6cull, 0x452821e638d01377ull};
    const PPC_VEC_U64 keyL = LoadUnaligned(key);
    const PPC_VEC_U64 keyH = LoadUnaligned(key + 2);
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
    const PPC_VEC_U64 packetL = LoadUnaligned(packet);
    const PPC_VEC_U64 packetH = LoadUnaligned(packet + 2);
    Update(packetH, packetL);
  }

  HH_INLINE void UpdateRemainder(const char* bytes, const size_t size_mod32) {
    // 'Length padding' differentiates zero-valued inputs that have the same
    // size/32. mod32 is sufficient because each Update behaves as if a
    // counter were injected, because the state is large and mixed thoroughly.
    uint32_t size_rounded = static_cast<uint32_t>(size_mod32);
    PPC_VEC_U32 vsize_mod32 = {size_rounded, size_rounded, size_rounded,
                               size_rounded};
    // Equivalent to storing size_mod32 in packet.
    v0L += reinterpret_cast<PPC_VEC_U64>(vsize_mod32);
    v0H += reinterpret_cast<PPC_VEC_U64>(vsize_mod32);

    // Boosts the avalanche effect of mod32.
    Rotate32By(&v1H, &v1L, size_mod32);

    const size_t size_mod4 = size_mod32 & 3;
    const char* HH_RESTRICT remainder = bytes + (size_mod32 & ~3);

    if (HH_UNLIKELY(size_mod32 & 16)) {  // 16..31 bytes left
      const PPC_VEC_U64 packetL =
          vec_vsx_ld(0, reinterpret_cast<const PPC_VEC_U64*>(bytes));

      PPC_VEC_U64 packetH = LoadMultipleOfFour(bytes + 16, size_mod32);

      const uint32_t last4 =
          Load3()(Load3::AllowReadBeforeAndReturn(), remainder, size_mod4);

      // The upper four bytes of packetH are zero, so insert there.
      PPC_VEC_U32 packetH_32 = reinterpret_cast<PPC_VEC_U32>(packetH);
      packetH_32[3] = last4;
      packetH = reinterpret_cast<PPC_VEC_U64>(packetH_32);
      Update(packetH, packetL);
    } else {  // size_mod32 < 16
      const PPC_VEC_U64 packetL = LoadMultipleOfFour(bytes, size_mod32);

      const uint64_t last4 =
          Load3()(Load3::AllowUnordered(), remainder, size_mod4);

      // Rather than insert into packetL[3], it is faster to initialize
      // the otherwise empty packetH.
      const PPC_VEC_U64 packetH = {last4, 0};
      Update(packetH, packetL);
    }
  }

  HH_INLINE void Finalize(HHResult64* HH_RESTRICT result) {
    // Mix together all lanes.
    for (int n = 0; n < 4; n++) {
      PermuteAndUpdate();
    }
    const PPC_VEC_U64 hash = v0L + v1L + mul0L + mul1L;
    *result = hash[0];
  }

  HH_INLINE void Finalize(HHResult128* HH_RESTRICT result) {
    for (int n = 0; n < 6; n++) {
      PermuteAndUpdate();
    }
    const PPC_VEC_U64 hash = v0L + mul0L + v1H + mul1H;
    StoreUnaligned(hash, *result);
  }

  HH_INLINE void Finalize(HHResult256* HH_RESTRICT result) {
    for (int n = 0; n < 10; n++) {
      PermuteAndUpdate();
    }
    const PPC_VEC_U64 sum0L = v0L + mul0L;
    const PPC_VEC_U64 sum1L = v1L + mul1L;
    const PPC_VEC_U64 sum0H = v0H + mul0H;
    const PPC_VEC_U64 sum1H = v1H + mul1H;
    const PPC_VEC_U64 hashL = ModularReduction(sum1L, sum0L);
    const PPC_VEC_U64 hashH = ModularReduction(sum1H, sum0H);
    StoreUnaligned(hashL, *result);
    StoreUnaligned(hashH, *result + 2);
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
  static HH_INLINE PPC_VEC_U64 Rotate64By32(const PPC_VEC_U64& v) {
    PPC_VEC_U64 shuffle_vec = {32, 32};
    return vec_rl(v, shuffle_vec);
  }

  // Rotates 32-bit lanes by "count" bits.
  static HH_INLINE void Rotate32By(PPC_VEC_U64* HH_RESTRICT vH,
                                   PPC_VEC_U64* HH_RESTRICT vL,
                                   const uint64_t count) {
    // WARNING: the shift count is 64 bits, so we can't reuse vsize_mod32,
    // which is broadcast into 32-bit lanes.
    uint32_t count_rl = uint32_t(count);
    PPC_VEC_U32 rot_left = {count_rl, count_rl, count_rl, count_rl};
    *vL = reinterpret_cast<PPC_VEC_U64>(vec_rl(PPC_VEC_U32(*vL), rot_left));
    *vH = reinterpret_cast<PPC_VEC_U64>(vec_rl(PPC_VEC_U32(*vH), rot_left));
  }

  static HH_INLINE PPC_VEC_U64 ZipperMerge(const PPC_VEC_U64& v) {
    // Multiplication mixes/scrambles bytes 0-7 of the 64-bit result to
    // varying degrees. In descending order of goodness, bytes
    // 3 4 2 5 1 6 0 7 have quality 228 224 164 160 100 96 36 32.
    // As expected, the upper and lower bytes are much worse.
    // For each 64-bit lane, our objectives are:
    // 1) maximizing and equalizing total goodness across each lane's bytes;
    // 2) mixing with bytes from the neighboring lane;
    // 3) placing the worst bytes in the upper 32 bits because those will not
    //    be used in the next 32x32 multiplication.

    const PPC_VEC_U64 mask = {0x000F010E05020C03ull, 0x070806090D0A040Bull};
    return vec_vperm(v, v, reinterpret_cast<const PPC_VEC_U8>(mask));
  }

  HH_INLINE void Update(const PPC_VEC_U64& packetH,
                        const PPC_VEC_U64& packetL) {
    // Tried rearranging the instructions below and benchmarks are similar
    v1L += packetL + mul0L;
    v1H += packetH + mul0H;
    mul0L ^= MultiplyVectors(v1L, Rotate64By32(v0L));
    mul0H ^= MultiplyVectors(v1H, v0H >> 32);
    v0L += mul1L;
    v0H += mul1H;
    mul1L ^= MultiplyVectors(v0L, Rotate64By32(v1L));
    mul1H ^= MultiplyVectors(v0H, v1H >> 32);
    v0L += ZipperMerge(v1L);
    v1L += ZipperMerge(v0L);
    v0H += ZipperMerge(v1H);
    v1H += ZipperMerge(v0H);
  }

  HH_INLINE void PermuteAndUpdate() {
    // Permutes v0L and V0H by swapping 32 bits halves of each lane
    Update(Rotate64By32(v0L), Rotate64By32(v0H));
  }

  // Returns zero-initialized vector with the lower "size" = 0, 4, 8 or 12
  // bytes loaded from "bytes". Serves as a replacement for AVX2 maskload_epi32.
  static HH_INLINE PPC_VEC_U64 LoadMultipleOfFour(const char* bytes,
                                                  const size_t size) {
    const uint32_t* words = reinterpret_cast<const uint32_t*>(bytes);
    // Updating the entries, as if done by vec_insert function call
    PPC_VEC_U32 ret = {0, 0, 0, 0};
    if (size & 8) {
      ret[0] = words[0];
      ret[1] = words[1];
      words += 2;
      if (size & 4) {
        ret[2] = words[0];
      }
    } else if (size & 4) {
      ret[0] = words[0];
    }
    return reinterpret_cast<PPC_VEC_U64>(ret);
  }

  // Modular reduction by the irreducible polynomial (x^128 + x^2 + x).
  // Input: a 256-bit number a3210.
  static HH_INLINE PPC_VEC_U64 ModularReduction(const PPC_VEC_U64& a32_unmasked,
                                                const PPC_VEC_U64& a10) {
    // See Lemire, https://arxiv.org/pdf/1503.03465v8.pdf.
    PPC_VEC_U64 out = a10;
    const PPC_VEC_U64 shifted1 = reinterpret_cast<PPC_VEC_U64>(
        vec_sll(reinterpret_cast<PPC_VEC_U32>(a32_unmasked), vec_splat_u8(1)));
    const PPC_VEC_U64 shifted2 = reinterpret_cast<PPC_VEC_U64>(
        vec_sll(reinterpret_cast<PPC_VEC_U32>(a32_unmasked), vec_splat_u8(2)));
    // The result must be as if the upper two bits of the input had been clear,
    // otherwise we're no longer computing a reduction.
    const PPC_VEC_U64 mask = {0xFFFFFFFFFFFFFFFFull, 0x7FFFFFFFFFFFFFFFull};
    const PPC_VEC_U64 shifted1_masked = shifted1 & mask;
    out ^= shifted1_masked ^ shifted2;
    return out;
  }

  PPC_VEC_U64 v0L;
  PPC_VEC_U64 v0H;
  PPC_VEC_U64 v1L;
  PPC_VEC_U64 v1H;
  PPC_VEC_U64 mul0L;
  PPC_VEC_U64 mul0H;
  PPC_VEC_U64 mul1L;
  PPC_VEC_U64 mul1H;
};

}  // namespace HH_TARGET_NAME
}  // namespace highwayhash

#endif  // HH_DISABLE_TARGET_SPECIFIC
#endif  // HIGHWAYHASH_HH_VSX_H_
