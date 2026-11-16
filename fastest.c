/* fastest - cross-platform bit-exact SIMD hardware-accelerated hash & XOF/stream cipher.
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include "fastest.h"
#include <string.h>

#if defined(_MSC_VER) && (defined(_M_X64) || defined(_M_ARM64))
#  include <intrin.h>
#endif

#if defined(__BYTE_ORDER__) && defined(__ORDER_BIG_ENDIAN__) && (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
#  define FASTEST_BIG_ENDIAN 1
#elif defined(__BIG_ENDIAN__) || defined(__ARMEB__) || defined(__THUMBEB__) || defined(__AARCH64EB__) || \
      defined(_MIPSEB) || defined(__MIPSEB) || defined(__MIPSEB__) || defined(__s390x__)
#  define FASTEST_BIG_ENDIAN 1
#else
#  define FASTEST_BIG_ENDIAN 0
#endif

#if !defined(FASTEST_PORTABLE) && !FASTEST_BIG_ENDIAN
#  if defined(__VAES__) && defined(__AVX512F__) && defined(__AVX512BW__) && defined(__AES__) && \
      (defined(__x86_64__) || defined(_M_X64))
#    define FASTEST_SIMD_AVX512 1
#    include <immintrin.h>
#  elif defined(__VAES__) && defined(__AVX2__) && defined(__AES__) && \
        (defined(__x86_64__) || defined(__i386__))
#    define FASTEST_SIMD_AVX2 1
#    include <immintrin.h>
#  elif defined(__AES__) && defined(__SSE2__) && (defined(__x86_64__) || defined(__i386__))
#    define FASTEST_SIMD_AESNI 1
#    include <emmintrin.h>
#    include <wmmintrin.h>
#    ifdef __SSSE3__
#      include <tmmintrin.h>
#    endif
#  elif defined(__aarch64__) && defined(__ARM_FEATURE_CRYPTO)
#    define FASTEST_SIMD_NEON 1
#    include <arm_neon.h>
#  endif
#endif

#define FASTEST_K_SEED0 UINT64_C(0x35f576a4e31cf92b)
#define FASTEST_K_SEED1 UINT64_C(0xfd215dc05e8bfeb3)
#define FASTEST_M_LEN   UINT64_C(0xae64446b1387b6bb)
#define FASTEST_M_CTR   UINT64_C(0xd1b54a32d192ed03)
static const uint64_t FASTEST_KS[2] = { UINT64_C(0xa1f80fbb79a83d7c), UINT64_C(0xffb8bc6c760570b5) };
static const uint64_t FASTEST_KF[8] = {
  UINT64_C(0xd9dbf43c4823fe8b), UINT64_C(0x65931a2b9fd09d61),
  UINT64_C(0x069baa639afee924), UINT64_C(0x66465c06aaea1a9e),
  UINT64_C(0x9a453c05c2f375c9), UINT64_C(0xbd58ddef438226aa),
  UINT64_C(0x7fadfbfe8e30ed5d), UINT64_C(0x6cdb053e8a02d7e9),
};
static const uint8_t FASTEST_LANE[16][16] = {
  { 0x6e, 0x90, 0x53, 0xe7, 0x97, 0xa2, 0x3a, 0x0c, 0x0d, 0x92, 0x4b, 0xb3, 0x93, 0xbb, 0x87, 0xd2 },
  { 0x6d, 0xc5, 0xb4, 0x56, 0x2c, 0x2e, 0x25, 0x2c, 0x3f, 0x90, 0x3f, 0x6e, 0x83, 0xc1, 0x28, 0xc0 },
  { 0xcb, 0xda, 0x80, 0xdf, 0x3d, 0x4b, 0x3f, 0x24, 0xcb, 0xe5, 0x78, 0x8e, 0xca, 0x71, 0x3d, 0x84 },
  { 0x69, 0x59, 0x22, 0xd6, 0x91, 0xb7, 0x88, 0xac, 0x99, 0x79, 0xa4, 0xcd, 0x0d, 0x8c, 0x6f, 0x20 },
  { 0x90, 0x03, 0xb1, 0x0f, 0xd2, 0x41, 0xb9, 0x17, 0x42, 0xde, 0x8a, 0x68, 0x88, 0x96, 0x5c, 0xbe },
  { 0x10, 0x56, 0x31, 0x0f, 0x43, 0x0e, 0x5d, 0x69, 0x53, 0x42, 0xd3, 0xd1, 0x3b, 0x63, 0x30, 0x23 },
  { 0x9a, 0x94, 0x3a, 0xbc, 0xf2, 0x36, 0xc5, 0x0c, 0x7b, 0x22, 0x65, 0xfd, 0x4c, 0xe4, 0x99, 0xa2 },
  { 0xfb, 0xb5, 0x1a, 0xc5, 0xa5, 0x0f, 0xba, 0xd7, 0x9f, 0x49, 0x2c, 0xaf, 0x4f, 0x1f, 0x45, 0x0e },
  { 0x41, 0x6d, 0x43, 0x29, 0x60, 0x15, 0xb9, 0x41, 0x13, 0x02, 0xdc, 0x8a, 0x19, 0xd6, 0xf1, 0x8c },
  { 0xb9, 0x99, 0x54, 0xbb, 0x91, 0x1b, 0xc3, 0x8b, 0xf3, 0x2a, 0x4f, 0x69, 0x22, 0x8d, 0x8e, 0x4f },
  { 0x9f, 0x59, 0x54, 0xf8, 0xdf, 0x72, 0xcd, 0x09, 0xea, 0x97, 0x21, 0x69, 0x60, 0xec, 0x3d, 0xba },
  { 0xc4, 0xb8, 0x26, 0x97, 0xdf, 0xc9, 0xed, 0x76, 0xfa, 0x03, 0xcc, 0x79, 0x67, 0xcb, 0x89, 0x8e },
  { 0x47, 0xc2, 0x55, 0xd2, 0xff, 0xda, 0x91, 0x44, 0x37, 0x64, 0xbf, 0x97, 0x47, 0x7b, 0xfa, 0xe3 },
  { 0xc4, 0x4e, 0xfe, 0x30, 0x1d, 0x65, 0x11, 0x77, 0x9b, 0x3c, 0x71, 0x99, 0xc8, 0xf0, 0x2c, 0x61 },
  { 0x75, 0xef, 0x23, 0x73, 0x03, 0x31, 0x94, 0xb4, 0x0e, 0x63, 0x2c, 0x87, 0x7a, 0x60, 0xcf, 0x8c },
  { 0x25, 0xc1, 0x3f, 0x73, 0xd3, 0xf3, 0x25, 0xc6, 0x12, 0x0b, 0x42, 0x85, 0x2a, 0x47, 0x79, 0x49 },
};
static const uint8_t FASTEST_CAP[2][16] = {
  { 0x41, 0x2e, 0xc6, 0x7f, 0xda, 0x59, 0xdf, 0x02, 0x77, 0x78, 0x07, 0xb4, 0xe1, 0x49, 0x54, 0xc1 },
  { 0x39, 0x00, 0xf1, 0x8a, 0x20, 0x41, 0x71, 0x98, 0xeb, 0x0b, 0x51, 0x11, 0x5e, 0x6a, 0x8f, 0x08 },
};
static const uint8_t FASTEST_FIN[8][16] = {
  { 0x13, 0x43, 0x8e, 0x55, 0xfb, 0x83, 0x22, 0x5b, 0xce, 0xb8, 0x10, 0x49, 0xc0, 0xf1, 0xe1, 0x96 },
  { 0x5f, 0x04, 0xf8, 0x92, 0x8e, 0x1a, 0x08, 0xda, 0x7b, 0x02, 0xaf, 0x59, 0x1c, 0x08, 0xaf, 0xa8 },
  { 0x3d, 0xce, 0x76, 0xd0, 0x6e, 0xc4, 0x15, 0x9f, 0x2c, 0xa9, 0xee, 0xa0, 0x2b, 0x3e, 0xb5, 0x25 },
  { 0x26, 0xd7, 0x4d, 0x3c, 0x6a, 0x2c, 0x4d, 0x9f, 0x97, 0x82, 0x56, 0x81, 0x84, 0x35, 0x57, 0x9e },
  { 0x46, 0x88, 0x6f, 0x97, 0xe2, 0xcc, 0x03, 0x64, 0xbd, 0x5a, 0xac, 0xbb, 0x24, 0x4b, 0x12, 0xcc },
  { 0x05, 0xb4, 0xf0, 0xc1, 0x1b, 0xe2, 0x26, 0x09, 0x30, 0xbd, 0x26, 0x2e, 0xd6, 0xdb, 0x48, 0x2e },
  { 0xa8, 0x1e, 0x45, 0xc6, 0x8d, 0x39, 0xe9, 0xb8, 0x1e, 0x80, 0x47, 0x09, 0xf2, 0xf5, 0x30, 0xd1 },
  { 0x79, 0x26, 0x85, 0xba, 0x7d, 0xe7, 0xca, 0x7a, 0x26, 0xea, 0x17, 0xf3, 0xac, 0x48, 0xcc, 0x45 },
};
static const uint8_t FASTEST_STREAM_KEY[16] = { 0x16, 0x31, 0x50, 0xf3, 0x94, 0x80, 0xea, 0x60, 0x20, 0x45, 0xb0, 0xf4, 0xef, 0x67, 0x8b, 0xd5 };
static const uint8_t FASTEST_SBOX[256] = {
  0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5, 0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76,
  0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0, 0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0,
  0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc, 0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15,
  0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a, 0x07, 0x12, 0x80, 0xe2, 0xeb, 0x27, 0xb2, 0x75,
  0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0, 0x52, 0x3b, 0xd6, 0xb3, 0x29, 0xe3, 0x2f, 0x84,
  0x53, 0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b, 0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c, 0x58, 0xcf,
  0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85, 0x45, 0xf9, 0x02, 0x7f, 0x50, 0x3c, 0x9f, 0xa8,
  0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5, 0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2,
  0xcd, 0x0c, 0x13, 0xec, 0x5f, 0x97, 0x44, 0x17, 0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19, 0x73,
  0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88, 0x46, 0xee, 0xb8, 0x14, 0xde, 0x5e, 0x0b, 0xdb,
  0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c, 0xc2, 0xd3, 0xac, 0x62, 0x91, 0x95, 0xe4, 0x79,
  0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9, 0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a, 0xae, 0x08,
  0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6, 0xe8, 0xdd, 0x74, 0x1f, 0x4b, 0xbd, 0x8b, 0x8a,
  0x70, 0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e, 0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1, 0x1d, 0x9e,
  0xe1, 0xf8, 0x98, 0x11, 0x69, 0xd9, 0x8e, 0x94, 0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf,
  0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68, 0x41, 0x99, 0x2d, 0x0f, 0xb0, 0x54, 0xbb, 0x16,
};
static const uint32_t FASTEST_T0[256] = {
  0xa56363c6u, 0x847c7cf8u, 0x997777eeu, 0x8d7b7bf6u, 0x0df2f2ffu, 0xbd6b6bd6u, 0xb16f6fdeu, 0x54c5c591u,
  0x50303060u, 0x03010102u, 0xa96767ceu, 0x7d2b2b56u, 0x19fefee7u, 0x62d7d7b5u, 0xe6abab4du, 0x9a7676ecu,
  0x45caca8fu, 0x9d82821fu, 0x40c9c989u, 0x877d7dfau, 0x15fafaefu, 0xeb5959b2u, 0xc947478eu, 0x0bf0f0fbu,
  0xecadad41u, 0x67d4d4b3u, 0xfda2a25fu, 0xeaafaf45u, 0xbf9c9c23u, 0xf7a4a453u, 0x967272e4u, 0x5bc0c09bu,
  0xc2b7b775u, 0x1cfdfde1u, 0xae93933du, 0x6a26264cu, 0x5a36366cu, 0x413f3f7eu, 0x02f7f7f5u, 0x4fcccc83u,
  0x5c343468u, 0xf4a5a551u, 0x34e5e5d1u, 0x08f1f1f9u, 0x937171e2u, 0x73d8d8abu, 0x53313162u, 0x3f15152au,
  0x0c040408u, 0x52c7c795u, 0x65232346u, 0x5ec3c39du, 0x28181830u, 0xa1969637u, 0x0f05050au, 0xb59a9a2fu,
  0x0907070eu, 0x36121224u, 0x9b80801bu, 0x3de2e2dfu, 0x26ebebcdu, 0x6927274eu, 0xcdb2b27fu, 0x9f7575eau,
  0x1b090912u, 0x9e83831du, 0x742c2c58u, 0x2e1a1a34u, 0x2d1b1b36u, 0xb26e6edcu, 0xee5a5ab4u, 0xfba0a05bu,
  0xf65252a4u, 0x4d3b3b76u, 0x61d6d6b7u, 0xceb3b37du, 0x7b292952u, 0x3ee3e3ddu, 0x712f2f5eu, 0x97848413u,
  0xf55353a6u, 0x68d1d1b9u, 0x00000000u, 0x2cededc1u, 0x60202040u, 0x1ffcfce3u, 0xc8b1b179u, 0xed5b5bb6u,
  0xbe6a6ad4u, 0x46cbcb8du, 0xd9bebe67u, 0x4b393972u, 0xde4a4a94u, 0xd44c4c98u, 0xe85858b0u, 0x4acfcf85u,
  0x6bd0d0bbu, 0x2aefefc5u, 0xe5aaaa4fu, 0x16fbfbedu, 0xc5434386u, 0xd74d4d9au, 0x55333366u, 0x94858511u,
  0xcf45458au, 0x10f9f9e9u, 0x06020204u, 0x817f7ffeu, 0xf05050a0u, 0x443c3c78u, 0xba9f9f25u, 0xe3a8a84bu,
  0xf35151a2u, 0xfea3a35du, 0xc0404080u, 0x8a8f8f05u, 0xad92923fu, 0xbc9d9d21u, 0x48383870u, 0x04f5f5f1u,
  0xdfbcbc63u, 0xc1b6b677u, 0x75dadaafu, 0x63212142u, 0x30101020u, 0x1affffe5u, 0x0ef3f3fdu, 0x6dd2d2bfu,
  0x4ccdcd81u, 0x140c0c18u, 0x35131326u, 0x2fececc3u, 0xe15f5fbeu, 0xa2979735u, 0xcc444488u, 0x3917172eu,
  0x57c4c493u, 0xf2a7a755u, 0x827e7efcu, 0x473d3d7au, 0xac6464c8u, 0xe75d5dbau, 0x2b191932u, 0x957373e6u,
  0xa06060c0u, 0x98818119u, 0xd14f4f9eu, 0x7fdcdca3u, 0x66222244u, 0x7e2a2a54u, 0xab90903bu, 0x8388880bu,
  0xca46468cu, 0x29eeeec7u, 0xd3b8b86bu, 0x3c141428u, 0x79dedea7u, 0xe25e5ebcu, 0x1d0b0b16u, 0x76dbdbadu,
  0x3be0e0dbu, 0x56323264u, 0x4e3a3a74u, 0x1e0a0a14u, 0xdb494992u, 0x0a06060cu, 0x6c242448u, 0xe45c5cb8u,
  0x5dc2c29fu, 0x6ed3d3bdu, 0xefacac43u, 0xa66262c4u, 0xa8919139u, 0xa4959531u, 0x37e4e4d3u, 0x8b7979f2u,
  0x32e7e7d5u, 0x43c8c88bu, 0x5937376eu, 0xb76d6ddau, 0x8c8d8d01u, 0x64d5d5b1u, 0xd24e4e9cu, 0xe0a9a949u,
  0xb46c6cd8u, 0xfa5656acu, 0x07f4f4f3u, 0x25eaeacfu, 0xaf6565cau, 0x8e7a7af4u, 0xe9aeae47u, 0x18080810u,
  0xd5baba6fu, 0x887878f0u, 0x6f25254au, 0x722e2e5cu, 0x241c1c38u, 0xf1a6a657u, 0xc7b4b473u, 0x51c6c697u,
  0x23e8e8cbu, 0x7cdddda1u, 0x9c7474e8u, 0x211f1f3eu, 0xdd4b4b96u, 0xdcbdbd61u, 0x868b8b0du, 0x858a8a0fu,
  0x907070e0u, 0x423e3e7cu, 0xc4b5b571u, 0xaa6666ccu, 0xd8484890u, 0x05030306u, 0x01f6f6f7u, 0x120e0e1cu,
  0xa36161c2u, 0x5f35356au, 0xf95757aeu, 0xd0b9b969u, 0x91868617u, 0x58c1c199u, 0x271d1d3au, 0xb99e9e27u,
  0x38e1e1d9u, 0x13f8f8ebu, 0xb398982bu, 0x33111122u, 0xbb6969d2u, 0x70d9d9a9u, 0x898e8e07u, 0xa7949433u,
  0xb69b9b2du, 0x221e1e3cu, 0x92878715u, 0x20e9e9c9u, 0x49cece87u, 0xff5555aau, 0x78282850u, 0x7adfdfa5u,
  0x8f8c8c03u, 0xf8a1a159u, 0x80898909u, 0x170d0d1au, 0xdabfbf65u, 0x31e6e6d7u, 0xc6424284u, 0xb86868d0u,
  0xc3414182u, 0xb0999929u, 0x772d2d5au, 0x110f0f1eu, 0xcbb0b07bu, 0xfc5454a8u, 0xd6bbbb6du, 0x3a16162cu,
};
static const uint32_t FASTEST_T1[256] = {
  0x6363c6a5u, 0x7c7cf884u, 0x7777ee99u, 0x7b7bf68du, 0xf2f2ff0du, 0x6b6bd6bdu, 0x6f6fdeb1u, 0xc5c59154u,
  0x30306050u, 0x01010203u, 0x6767cea9u, 0x2b2b567du, 0xfefee719u, 0xd7d7b562u, 0xabab4de6u, 0x7676ec9au,
  0xcaca8f45u, 0x82821f9du, 0xc9c98940u, 0x7d7dfa87u, 0xfafaef15u, 0x5959b2ebu, 0x47478ec9u, 0xf0f0fb0bu,
  0xadad41ecu, 0xd4d4b367u, 0xa2a25ffdu, 0xafaf45eau, 0x9c9c23bfu, 0xa4a453f7u, 0x7272e496u, 0xc0c09b5bu,
  0xb7b775c2u, 0xfdfde11cu, 0x93933daeu, 0x26264c6au, 0x36366c5au, 0x3f3f7e41u, 0xf7f7f502u, 0xcccc834fu,
  0x3434685cu, 0xa5a551f4u, 0xe5e5d134u, 0xf1f1f908u, 0x7171e293u, 0xd8d8ab73u, 0x31316253u, 0x15152a3fu,
  0x0404080cu, 0xc7c79552u, 0x23234665u, 0xc3c39d5eu, 0x18183028u, 0x969637a1u, 0x05050a0fu, 0x9a9a2fb5u,
  0x07070e09u, 0x12122436u, 0x80801b9bu, 0xe2e2df3du, 0xebebcd26u, 0x27274e69u, 0xb2b27fcdu, 0x7575ea9fu,
  0x0909121bu, 0x83831d9eu, 0x2c2c5874u, 0x1a1a342eu, 0x1b1b362du, 0x6e6edcb2u, 0x5a5ab4eeu, 0xa0a05bfbu,
  0x5252a4f6u, 0x3b3b764du, 0xd6d6b761u, 0xb3b37dceu, 0x2929527bu, 0xe3e3dd3eu, 0x2f2f5e71u, 0x84841397u,
  0x5353a6f5u, 0xd1d1b968u, 0x00000000u, 0xededc12cu, 0x20204060u, 0xfcfce31fu, 0xb1b179c8u, 0x5b5bb6edu,
  0x6a6ad4beu, 0xcbcb8d46u, 0xbebe67d9u, 0x3939724bu, 0x4a4a94deu, 0x4c4c98d4u, 0x5858b0e8u, 0xcfcf854au,
  0xd0d0bb6bu, 0xefefc52au, 0xaaaa4fe5u, 0xfbfbed16u, 0x434386c5u, 0x4d4d9ad7u, 0x33336655u, 0x85851194u,
  0x45458acfu, 0xf9f9e910u, 0x02020406u, 0x7f7ffe81u, 0x5050a0f0u, 0x3c3c7844u, 0x9f9f25bau, 0xa8a84be3u,
  0x5151a2f3u, 0xa3a35dfeu, 0x404080c0u, 0x8f8f058au, 0x92923fadu, 0x9d9d21bcu, 0x38387048u, 0xf5f5f104u,
  0xbcbc63dfu, 0xb6b677c1u, 0xdadaaf75u, 0x21214263u, 0x10102030u, 0xffffe51au, 0xf3f3fd0eu, 0xd2d2bf6du,
  0xcdcd814cu, 0x0c0c1814u, 0x13132635u, 0xececc32fu, 0x5f5fbee1u, 0x979735a2u, 0x444488ccu, 0x17172e39u,
  0xc4c49357u, 0xa7a755f2u, 0x7e7efc82u, 0x3d3d7a47u, 0x6464c8acu, 0x5d5dbae7u, 0x1919322bu, 0x7373e695u,
  0x6060c0a0u, 0x81811998u, 0x4f4f9ed1u, 0xdcdca37fu, 0x22224466u, 0x2a2a547eu, 0x90903babu, 0x88880b83u,
  0x46468ccau, 0xeeeec729u, 0xb8b86bd3u, 0x1414283cu, 0xdedea779u, 0x5e5ebce2u, 0x0b0b161du, 0xdbdbad76u,
  0xe0e0db3bu, 0x32326456u, 0x3a3a744eu, 0x0a0a141eu, 0x494992dbu, 0x06060c0au, 0x2424486cu, 0x5c5cb8e4u,
  0xc2c29f5du, 0xd3d3bd6eu, 0xacac43efu, 0x6262c4a6u, 0x919139a8u, 0x959531a4u, 0xe4e4d337u, 0x7979f28bu,
  0xe7e7d532u, 0xc8c88b43u, 0x37376e59u, 0x6d6ddab7u, 0x8d8d018cu, 0xd5d5b164u, 0x4e4e9cd2u, 0xa9a949e0u,
  0x6c6cd8b4u, 0x5656acfau, 0xf4f4f307u, 0xeaeacf25u, 0x6565caafu, 0x7a7af48eu, 0xaeae47e9u, 0x08081018u,
  0xbaba6fd5u, 0x7878f088u, 0x25254a6fu, 0x2e2e5c72u, 0x1c1c3824u, 0xa6a657f1u, 0xb4b473c7u, 0xc6c69751u,
  0xe8e8cb23u, 0xdddda17cu, 0x7474e89cu, 0x1f1f3e21u, 0x4b4b96ddu, 0xbdbd61dcu, 0x8b8b0d86u, 0x8a8a0f85u,
  0x7070e090u, 0x3e3e7c42u, 0xb5b571c4u, 0x6666ccaau, 0x484890d8u, 0x03030605u, 0xf6f6f701u, 0x0e0e1c12u,
  0x6161c2a3u, 0x35356a5fu, 0x5757aef9u, 0xb9b969d0u, 0x86861791u, 0xc1c19958u, 0x1d1d3a27u, 0x9e9e27b9u,
  0xe1e1d938u, 0xf8f8eb13u, 0x98982bb3u, 0x11112233u, 0x6969d2bbu, 0xd9d9a970u, 0x8e8e0789u, 0x949433a7u,
  0x9b9b2db6u, 0x1e1e3c22u, 0x87871592u, 0xe9e9c920u, 0xcece8749u, 0x5555aaffu, 0x28285078u, 0xdfdfa57au,
  0x8c8c038fu, 0xa1a159f8u, 0x89890980u, 0x0d0d1a17u, 0xbfbf65dau, 0xe6e6d731u, 0x424284c6u, 0x6868d0b8u,
  0x414182c3u, 0x999929b0u, 0x2d2d5a77u, 0x0f0f1e11u, 0xb0b07bcbu, 0x5454a8fcu, 0xbbbb6dd6u, 0x16162c3au,
};
static const uint32_t FASTEST_T2[256] = {
  0x63c6a563u, 0x7cf8847cu, 0x77ee9977u, 0x7bf68d7bu, 0xf2ff0df2u, 0x6bd6bd6bu, 0x6fdeb16fu, 0xc59154c5u,
  0x30605030u, 0x01020301u, 0x67cea967u, 0x2b567d2bu, 0xfee719feu, 0xd7b562d7u, 0xab4de6abu, 0x76ec9a76u,
  0xca8f45cau, 0x821f9d82u, 0xc98940c9u, 0x7dfa877du, 0xfaef15fau, 0x59b2eb59u, 0x478ec947u, 0xf0fb0bf0u,
  0xad41ecadu, 0xd4b367d4u, 0xa25ffda2u, 0xaf45eaafu, 0x9c23bf9cu, 0xa453f7a4u, 0x72e49672u, 0xc09b5bc0u,
  0xb775c2b7u, 0xfde11cfdu, 0x933dae93u, 0x264c6a26u, 0x366c5a36u, 0x3f7e413fu, 0xf7f502f7u, 0xcc834fccu,
  0x34685c34u, 0xa551f4a5u, 0xe5d134e5u, 0xf1f908f1u, 0x71e29371u, 0xd8ab73d8u, 0x31625331u, 0x152a3f15u,
  0x04080c04u, 0xc79552c7u, 0x23466523u, 0xc39d5ec3u, 0x18302818u, 0x9637a196u, 0x050a0f05u, 0x9a2fb59au,
  0x070e0907u, 0x12243612u, 0x801b9b80u, 0xe2df3de2u, 0xebcd26ebu, 0x274e6927u, 0xb27fcdb2u, 0x75ea9f75u,
  0x09121b09u, 0x831d9e83u, 0x2c58742cu, 0x1a342e1au, 0x1b362d1bu, 0x6edcb26eu, 0x5ab4ee5au, 0xa05bfba0u,
  0x52a4f652u, 0x3b764d3bu, 0xd6b761d6u, 0xb37dceb3u, 0x29527b29u, 0xe3dd3ee3u, 0x2f5e712fu, 0x84139784u,
  0x53a6f553u, 0xd1b968d1u, 0x00000000u, 0xedc12cedu, 0x20406020u, 0xfce31ffcu, 0xb179c8b1u, 0x5bb6ed5bu,
  0x6ad4be6au, 0xcb8d46cbu, 0xbe67d9beu, 0x39724b39u, 0x4a94de4au, 0x4c98d44cu, 0x58b0e858u, 0xcf854acfu,
  0xd0bb6bd0u, 0xefc52aefu, 0xaa4fe5aau, 0xfbed16fbu, 0x4386c543u, 0x4d9ad74du, 0x33665533u, 0x85119485u,
  0x458acf45u, 0xf9e910f9u, 0x02040602u, 0x7ffe817fu, 0x50a0f050u, 0x3c78443cu, 0x9f25ba9fu, 0xa84be3a8u,
  0x51a2f351u, 0xa35dfea3u, 0x4080c040u, 0x8f058a8fu, 0x923fad92u, 0x9d21bc9du, 0x38704838u, 0xf5f104f5u,
  0xbc63dfbcu, 0xb677c1b6u, 0xdaaf75dau, 0x21426321u, 0x10203010u, 0xffe51affu, 0xf3fd0ef3u, 0xd2bf6dd2u,
  0xcd814ccdu, 0x0c18140cu, 0x13263513u, 0xecc32fecu, 0x5fbee15fu, 0x9735a297u, 0x4488cc44u, 0x172e3917u,
  0xc49357c4u, 0xa755f2a7u, 0x7efc827eu, 0x3d7a473du, 0x64c8ac64u, 0x5dbae75du, 0x19322b19u, 0x73e69573u,
  0x60c0a060u, 0x81199881u, 0x4f9ed14fu, 0xdca37fdcu, 0x22446622u, 0x2a547e2au, 0x903bab90u, 0x880b8388u,
  0x468cca46u, 0xeec729eeu, 0xb86bd3b8u, 0x14283c14u, 0xdea779deu, 0x5ebce25eu, 0x0b161d0bu, 0xdbad76dbu,
  0xe0db3be0u, 0x32645632u, 0x3a744e3au, 0x0a141e0au, 0x4992db49u, 0x060c0a06u, 0x24486c24u, 0x5cb8e45cu,
  0xc29f5dc2u, 0xd3bd6ed3u, 0xac43efacu, 0x62c4a662u, 0x9139a891u, 0x9531a495u, 0xe4d337e4u, 0x79f28b79u,
  0xe7d532e7u, 0xc88b43c8u, 0x376e5937u, 0x6ddab76du, 0x8d018c8du, 0xd5b164d5u, 0x4e9cd24eu, 0xa949e0a9u,
  0x6cd8b46cu, 0x56acfa56u, 0xf4f307f4u, 0xeacf25eau, 0x65caaf65u, 0x7af48e7au, 0xae47e9aeu, 0x08101808u,
  0xba6fd5bau, 0x78f08878u, 0x254a6f25u, 0x2e5c722eu, 0x1c38241cu, 0xa657f1a6u, 0xb473c7b4u, 0xc69751c6u,
  0xe8cb23e8u, 0xdda17cddu, 0x74e89c74u, 0x1f3e211fu, 0x4b96dd4bu, 0xbd61dcbdu, 0x8b0d868bu, 0x8a0f858au,
  0x70e09070u, 0x3e7c423eu, 0xb571c4b5u, 0x66ccaa66u, 0x4890d848u, 0x03060503u, 0xf6f701f6u, 0x0e1c120eu,
  0x61c2a361u, 0x356a5f35u, 0x57aef957u, 0xb969d0b9u, 0x86179186u, 0xc19958c1u, 0x1d3a271du, 0x9e27b99eu,
  0xe1d938e1u, 0xf8eb13f8u, 0x982bb398u, 0x11223311u, 0x69d2bb69u, 0xd9a970d9u, 0x8e07898eu, 0x9433a794u,
  0x9b2db69bu, 0x1e3c221eu, 0x87159287u, 0xe9c920e9u, 0xce8749ceu, 0x55aaff55u, 0x28507828u, 0xdfa57adfu,
  0x8c038f8cu, 0xa159f8a1u, 0x89098089u, 0x0d1a170du, 0xbf65dabfu, 0xe6d731e6u, 0x4284c642u, 0x68d0b868u,
  0x4182c341u, 0x9929b099u, 0x2d5a772du, 0x0f1e110fu, 0xb07bcbb0u, 0x54a8fc54u, 0xbb6dd6bbu, 0x162c3a16u,
};
static const uint32_t FASTEST_T3[256] = {
  0xc6a56363u, 0xf8847c7cu, 0xee997777u, 0xf68d7b7bu, 0xff0df2f2u, 0xd6bd6b6bu, 0xdeb16f6fu, 0x9154c5c5u,
  0x60503030u, 0x02030101u, 0xcea96767u, 0x567d2b2bu, 0xe719fefeu, 0xb562d7d7u, 0x4de6ababu, 0xec9a7676u,
  0x8f45cacau, 0x1f9d8282u, 0x8940c9c9u, 0xfa877d7du, 0xef15fafau, 0xb2eb5959u, 0x8ec94747u, 0xfb0bf0f0u,
  0x41ecadadu, 0xb367d4d4u, 0x5ffda2a2u, 0x45eaafafu, 0x23bf9c9cu, 0x53f7a4a4u, 0xe4967272u, 0x9b5bc0c0u,
  0x75c2b7b7u, 0xe11cfdfdu, 0x3dae9393u, 0x4c6a2626u, 0x6c5a3636u, 0x7e413f3fu, 0xf502f7f7u, 0x834fccccu,
  0x685c3434u, 0x51f4a5a5u, 0xd134e5e5u, 0xf908f1f1u, 0xe2937171u, 0xab73d8d8u, 0x62533131u, 0x2a3f1515u,
  0x080c0404u, 0x9552c7c7u, 0x46652323u, 0x9d5ec3c3u, 0x30281818u, 0x37a19696u, 0x0a0f0505u, 0x2fb59a9au,
  0x0e090707u, 0x24361212u, 0x1b9b8080u, 0xdf3de2e2u, 0xcd26ebebu, 0x4e692727u, 0x7fcdb2b2u, 0xea9f7575u,
  0x121b0909u, 0x1d9e8383u, 0x58742c2cu, 0x342e1a1au, 0x362d1b1bu, 0xdcb26e6eu, 0xb4ee5a5au, 0x5bfba0a0u,
  0xa4f65252u, 0x764d3b3bu, 0xb761d6d6u, 0x7dceb3b3u, 0x527b2929u, 0xdd3ee3e3u, 0x5e712f2fu, 0x13978484u,
  0xa6f55353u, 0xb968d1d1u, 0x00000000u, 0xc12cededu, 0x40602020u, 0xe31ffcfcu, 0x79c8b1b1u, 0xb6ed5b5bu,
  0xd4be6a6au, 0x8d46cbcbu, 0x67d9bebeu, 0x724b3939u, 0x94de4a4au, 0x98d44c4cu, 0xb0e85858u, 0x854acfcfu,
  0xbb6bd0d0u, 0xc52aefefu, 0x4fe5aaaau, 0xed16fbfbu, 0x86c54343u, 0x9ad74d4du, 0x66553333u, 0x11948585u,
  0x8acf4545u, 0xe910f9f9u, 0x04060202u, 0xfe817f7fu, 0xa0f05050u, 0x78443c3cu, 0x25ba9f9fu, 0x4be3a8a8u,
  0xa2f35151u, 0x5dfea3a3u, 0x80c04040u, 0x058a8f8fu, 0x3fad9292u, 0x21bc9d9du, 0x70483838u, 0xf104f5f5u,
  0x63dfbcbcu, 0x77c1b6b6u, 0xaf75dadau, 0x42632121u, 0x20301010u, 0xe51affffu, 0xfd0ef3f3u, 0xbf6dd2d2u,
  0x814ccdcdu, 0x18140c0cu, 0x26351313u, 0xc32fececu, 0xbee15f5fu, 0x35a29797u, 0x88cc4444u, 0x2e391717u,
  0x9357c4c4u, 0x55f2a7a7u, 0xfc827e7eu, 0x7a473d3du, 0xc8ac6464u, 0xbae75d5du, 0x322b1919u, 0xe6957373u,
  0xc0a06060u, 0x19988181u, 0x9ed14f4fu, 0xa37fdcdcu, 0x44662222u, 0x547e2a2au, 0x3bab9090u, 0x0b838888u,
  0x8cca4646u, 0xc729eeeeu, 0x6bd3b8b8u, 0x283c1414u, 0xa779dedeu, 0xbce25e5eu, 0x161d0b0bu, 0xad76dbdbu,
  0xdb3be0e0u, 0x64563232u, 0x744e3a3au, 0x141e0a0au, 0x92db4949u, 0x0c0a0606u, 0x486c2424u, 0xb8e45c5cu,
  0x9f5dc2c2u, 0xbd6ed3d3u, 0x43efacacu, 0xc4a66262u, 0x39a89191u, 0x31a49595u, 0xd337e4e4u, 0xf28b7979u,
  0xd532e7e7u, 0x8b43c8c8u, 0x6e593737u, 0xdab76d6du, 0x018c8d8du, 0xb164d5d5u, 0x9cd24e4eu, 0x49e0a9a9u,
  0xd8b46c6cu, 0xacfa5656u, 0xf307f4f4u, 0xcf25eaeau, 0xcaaf6565u, 0xf48e7a7au, 0x47e9aeaeu, 0x10180808u,
  0x6fd5babau, 0xf0887878u, 0x4a6f2525u, 0x5c722e2eu, 0x38241c1cu, 0x57f1a6a6u, 0x73c7b4b4u, 0x9751c6c6u,
  0xcb23e8e8u, 0xa17cddddu, 0xe89c7474u, 0x3e211f1fu, 0x96dd4b4bu, 0x61dcbdbdu, 0x0d868b8bu, 0x0f858a8au,
  0xe0907070u, 0x7c423e3eu, 0x71c4b5b5u, 0xccaa6666u, 0x90d84848u, 0x06050303u, 0xf701f6f6u, 0x1c120e0eu,
  0xc2a36161u, 0x6a5f3535u, 0xaef95757u, 0x69d0b9b9u, 0x17918686u, 0x9958c1c1u, 0x3a271d1du, 0x27b99e9eu,
  0xd938e1e1u, 0xeb13f8f8u, 0x2bb39898u, 0x22331111u, 0xd2bb6969u, 0xa970d9d9u, 0x07898e8eu, 0x33a79494u,
  0x2db69b9bu, 0x3c221e1eu, 0x15928787u, 0xc920e9e9u, 0x8749ceceu, 0xaaff5555u, 0x50782828u, 0xa57adfdfu,
  0x038f8c8cu, 0x59f8a1a1u, 0x09808989u, 0x1a170d0du, 0x65dabfbfu, 0xd731e6e6u, 0x84c64242u, 0xd0b86868u,
  0x82c34141u, 0x29b09999u, 0x5a772d2du, 0x1e110f0fu, 0x7bcbb0b0u, 0xa8fc5454u, 0x6dd6bbbbu, 0x2c3a1616u,
};

static const uint32_t FASTEST_ROT[4] = { 0u, 19u, 38u, 57u };

static inline uint32_t fastest_rd32_le(const uint8_t *p) {
#if !FASTEST_BIG_ENDIAN
  uint32_t v;
  memcpy(&v, p, sizeof(v));
  return v;
#else
  return (uint32_t)p[0] | ((uint32_t)p[1] << 8) | ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
#endif
}

static inline uint64_t fastest_rd64_le(const uint8_t *p) {
#if !FASTEST_BIG_ENDIAN
  uint64_t v;
  memcpy(&v, p, sizeof(v));
  return v;
#else
  return (uint64_t)p[0] | ((uint64_t)p[1] << 8) | ((uint64_t)p[2] << 16) | ((uint64_t)p[3] << 24) |
         ((uint64_t)p[4] << 32) | ((uint64_t)p[5] << 40) | ((uint64_t)p[6] << 48) | ((uint64_t)p[7] << 56);
#endif
}

static inline void fastest_wr64_le(uint8_t *p, uint64_t v) {
#if !FASTEST_BIG_ENDIAN
  memcpy(p, &v, sizeof(v));
#else
  p[0] = (uint8_t)v;
  p[1] = (uint8_t)(v >> 8);
  p[2] = (uint8_t)(v >> 16);
  p[3] = (uint8_t)(v >> 24);
  p[4] = (uint8_t)(v >> 32);
  p[5] = (uint8_t)(v >> 40);
  p[6] = (uint8_t)(v >> 48);
  p[7] = (uint8_t)(v >> 56);
#endif
}

static inline uint64_t fastest_rotl64(uint64_t x, unsigned r) {
  return (r == 0u) ? x : ((x << r) | (x >> (64u - r)));
}

static inline uint64_t fastest_mul128(uint64_t a, uint64_t b, uint64_t *hi) {
#if defined(__SIZEOF_INT128__)
  __extension__ typedef unsigned __int128 fastest_u128;
  fastest_u128 r = (fastest_u128)a * (fastest_u128)b;
  *hi = (uint64_t)(r >> 64);
  return (uint64_t)r;
#elif defined(_MSC_VER) && (defined(_M_X64) || defined(_M_ARM64))
  return _umul128(a, b, hi);
#else
  uint64_t al = (uint32_t)a, ah = a >> 32;
  uint64_t bl = (uint32_t)b, bh = b >> 32;
  uint64_t p0 = al * bl;
  uint64_t p1 = al * bh;
  uint64_t p2 = ah * bl;
  uint64_t p3 = ah * bh;
  uint64_t mid = (p0 >> 32) + (uint32_t)p1 + (uint32_t)p2;
  *hi = p3 + (p1 >> 32) + (p2 >> 32) + (mid >> 32);
  return (p0 & UINT32_MAX) | (mid << 32);
#endif
}

static inline uint64_t fastest_fold(uint64_t a, uint64_t b) {
  uint64_t hi;
  uint64_t lo = fastest_mul128(a, b, &hi);
  return lo ^ hi;
}

static inline void fastest_mum(uint64_t *a, uint64_t *b) {
  uint64_t hi;
  uint64_t lo = fastest_mul128(*a, *b, &hi);
  *a ^= lo;
  *b ^= hi;
}

static inline void fastest_secret_pair(uint64_t seed, uint64_t *ka, uint64_t *kb, uint64_t *ks) {
  uint64_t y0 = seed ^ FASTEST_K_SEED0;
  uint64_t y1 = y0 ^ (y0 >> 33);
  uint64_t m = y1 * FASTEST_K_SEED1;
  uint64_t s = m ^ (m >> 29);
  *ka = s ^ FASTEST_KS[0];
  *kb = fastest_rotl64(s, 23) ^ FASTEST_KS[1];
  *ks = s;
}

static inline void fastest_word_pair(const uint8_t *p, size_t len, uint64_t *w0, uint64_t *w1) {
  if (len >= 8) {
    *w0 = fastest_rd64_le(p);
    *w1 = fastest_rd64_le(p + len - 8u);
  } else if (len >= 4) {
    *w0 = (uint64_t)fastest_rd32_le(p);
    *w1 = (uint64_t)fastest_rd32_le(p + len - 4u);
  } else if (len > 0) {
    *w0 = (uint64_t)p[0] | ((uint64_t)p[len >> 1] << 8) | ((uint64_t)p[len - 1u] << 16);
    *w1 = 0u;
  } else {
    *w0 = 0u;
    *w1 = 0u;
  }
}

static inline void fastest_short_finish(
  int width,
  uint64_t a0,
  uint64_t b0,
  uint64_t a1,
  uint64_t b1,
  uint64_t ln,
  uint64_t out[4]
) {
  out[0] = fastest_fold(a0 ^ (FASTEST_KF[0] ^ ln), b0 ^ FASTEST_KF[1]);
  if (width >= 2) {
    out[1] = fastest_fold(a0 ^ FASTEST_KF[2], b0 ^ (FASTEST_KF[3] ^ ln));
  }
  if (width == 4) {
    out[2] = fastest_fold(a1 ^ FASTEST_KF[4], b1 ^ (FASTEST_KF[5] ^ ln));
    out[3] = fastest_fold(fastest_rotl64(a1, 23) ^ (FASTEST_KF[6] ^ ln), fastest_rotl64(b1, 41) ^ FASTEST_KF[7]);
  }
}

static inline void fastest_short_hash(const uint8_t *p, size_t len, uint64_t seed, int width, uint64_t out[4]) {
  uint64_t ka, kb, ks;
  uint64_t ln = (uint64_t)len;
  fastest_secret_pair(seed, &ka, &kb, &ks);
  (void)ks;
  if (len <= 16) {
    uint64_t a0, b0;
    fastest_word_pair(p, len, &a0, &b0);
    a0 ^= ka;
    b0 ^= kb ^ ln;
    fastest_mum(&a0, &b0);
    fastest_short_finish(width, a0, b0, fastest_rotl64(a0, 17), fastest_rotl64(b0, 29), ln, out);
  } else if (len <= 32) {
    uint64_t x0 = fastest_rd64_le(p) ^ ka;
    uint64_t y0 = fastest_rd64_le(p + 8) ^ kb;
    uint64_t x1 = fastest_rd64_le(p + len - 16) ^ fastest_rotl64(ka, FASTEST_ROT[1]);
    uint64_t y1 = fastest_rd64_le(p + len - 8) ^ (fastest_rotl64(kb, FASTEST_ROT[1]) ^ ln);
    fastest_mum(&x0, &y0);
    fastest_mum(&x1, &y1);
    fastest_short_finish(
      width,
      x0 ^ y1,
      y0 ^ x1,
      x0 + fastest_rotl64(y1, 17),
      y0 + fastest_rotl64(x1, 29),
      ln,
      out
    );
  } else {
    uint64_t x0 = fastest_rd64_le(p) ^ ka;
    uint64_t y0 = fastest_rd64_le(p + 8) ^ kb;
    uint64_t x1 = fastest_rd64_le(p + 16) ^ fastest_rotl64(ka, FASTEST_ROT[1]);
    uint64_t y1 = fastest_rd64_le(p + 24) ^ fastest_rotl64(kb, FASTEST_ROT[1]);
    uint64_t x2 = fastest_rd64_le(p + len - 32) ^ fastest_rotl64(ka, FASTEST_ROT[2]);
    uint64_t y2 = fastest_rd64_le(p + len - 24) ^ fastest_rotl64(kb, FASTEST_ROT[2]);
    uint64_t x3 = fastest_rd64_le(p + len - 16) ^ fastest_rotl64(ka, FASTEST_ROT[3]);
    uint64_t y3 = fastest_rd64_le(p + len - 8) ^ (fastest_rotl64(kb, FASTEST_ROT[3]) ^ ln);
    fastest_mum(&x0, &y0);
    fastest_mum(&x1, &y1);
    fastest_mum(&x2, &y2);
    fastest_mum(&x3, &y3);
    fastest_short_finish(
      width,
      x0 ^ y1 ^ x2 ^ y3,
      y0 ^ x1 ^ y2 ^ x3,
      x0 + fastest_rotl64(y1, 17) + fastest_rotl64(x2, 31) + fastest_rotl64(y3, 47),
      y0 + fastest_rotl64(x1, 29) + fastest_rotl64(y2, 43) + fastest_rotl64(x3, 59),
      ln,
      out
    );
  }
}

static inline void fastest_pair_state(uint64_t lo, uint64_t hi, uint8_t out[16]) {
  fastest_wr64_le(out, lo);
  fastest_wr64_le(out + 8, hi);
}

static inline void fastest_aes_enc_tbl(const uint8_t st[16], const uint8_t rk[16], uint8_t out[16]) {
  size_t col;
  for (col = 0; col < 4; ++col) {
    uint32_t w = FASTEST_T0[st[4u * col]] ^
                 FASTEST_T1[st[(4u * col + 5u) & 15u]] ^
                 FASTEST_T2[st[(4u * col + 10u) & 15u]] ^
                 FASTEST_T3[st[(4u * col + 15u) & 15u]];
    out[4u * col]      = (uint8_t)w ^ rk[4u * col];
    out[4u * col + 1u] = (uint8_t)(w >> 8) ^ rk[4u * col + 1u];
    out[4u * col + 2u] = (uint8_t)(w >> 16) ^ rk[4u * col + 2u];
    out[4u * col + 3u] = (uint8_t)(w >> 24) ^ rk[4u * col + 3u];
  }
}

static inline void fastest_aes_last_tbl(const uint8_t st[16], const uint8_t rk[16], uint8_t out[16]) {
  size_t i;
  for (i = 0; i < 16; ++i) {
    out[i] = FASTEST_SBOX[st[(5u * i) & 15u]] ^ rk[i];
  }
}

#if defined(FASTEST_SIMD_AVX512)

static inline __m128i fastest_ld128(const uint8_t *p) {
  return _mm_loadu_si128((const __m128i *)(const void *)p);
}

static inline __m128i fastest_pair128(uint64_t lo, uint64_t hi) {
  uint8_t buf[16];
  fastest_pair_state(lo, hi, buf);
  return fastest_ld128(buf);
}

static inline __m256i fastest_dup128(__m128i v) {
  return _mm256_inserti128_si256(_mm256_castsi128_si256(v), v, 1);
}

static inline __m256i fastest_sum_zmm_cap(__m512i s_z) {
  __m256i s_y = _mm256_add_epi64(_mm512_castsi512_si256(s_z), _mm512_extracti64x4_epi64(s_z, 1));
  return _mm256_alignr_epi8(s_y, s_y, 14);
}

static void fastest_long_hash(const uint8_t *p, size_t len, uint64_t seed, int hi256, uint8_t root[32]) {
  uint64_t ka, kb, ks;
  size_t z_cnt = (len <= 256) ? 1u : (len <= 1024 ? 2u : 4u);
  size_t stride = z_cnt * 64u;
  uint64_t len_mul = (uint64_t)len * FASTEST_M_LEN;
  __m128i seed_x = fastest_pair128(0, 0), len_x;
  __m256i seed_y, zero_y, cap_init, cap_y;
  __m512i seed_z, len_z, zero_z, sum_z;
  __m512i t[4];
  const uint8_t *last = p + len - stride;
  const uint8_t *cur = p + stride;
  size_t i;

  fastest_secret_pair(seed, &ka, &kb, &ks);
  (void)ka;
  (void)kb;

  seed_x = fastest_pair128(ks, fastest_rotl64(ks, 29) ^ len_mul);
  len_x = fastest_pair128(len_mul, fastest_rotl64(len_mul, 17));
  seed_y = fastest_dup128(seed_x);
  seed_z = _mm512_broadcast_i32x4(seed_x);
  len_z = _mm512_broadcast_i32x4(len_x);
  zero_y = _mm256_setzero_si256();
  zero_z = _mm512_broadcast_i32x4(_mm_setzero_si128());

  sum_z = _mm512_loadu_si512((const void *)p);
  t[0] = _mm512_xor_si512(_mm512_xor_si512(_mm512_loadu_si512((const void *)FASTEST_LANE[0]), seed_z), sum_z);
  for (i = 1; i < z_cnt; ++i) {
    __m512i b = _mm512_loadu_si512((const void *)(p + i * 64u));
    __m512i init = _mm512_loadu_si512((const void *)(FASTEST_LANE[4u * i]));
    sum_z = _mm512_add_epi64(sum_z, b);
    t[i] = _mm512_xor_si512(_mm512_xor_si512(init, seed_z), b);
  }
  cap_init = _mm256_loadu_si256((const __m256i *)(const void *)FASTEST_CAP);
  cap_y = _mm256_xor_si256(_mm256_xor_si256(cap_init, seed_y), fastest_sum_zmm_cap(sum_z));

  while (cur < last) {
    __m512i s_z = _mm512_loadu_si512((const void *)cur);
    t[0] = _mm512_aesenc_epi128(t[0], s_z);
    for (i = 1; i < z_cnt; ++i) {
      __m512i b = _mm512_loadu_si512((const void *)(cur + i * 64u));
      s_z = _mm512_add_epi64(s_z, b);
      t[i] = _mm512_aesenc_epi128(t[i], b);
    }
    cap_y = _mm256_aesenc_epi128(cap_y, fastest_sum_zmm_cap(s_z));
    cur += stride;
  }

  {
    __m512i ls_z = _mm512_xor_si512(_mm512_loadu_si512((const void *)last), len_z);
    t[0] = _mm512_aesenc_epi128(t[0], ls_z);
    for (i = 1; i < z_cnt; ++i) {
      __m512i b = _mm512_xor_si512(_mm512_loadu_si512((const void *)(last + i * 64u)), len_z);
      ls_z = _mm512_add_epi64(ls_z, b);
      t[i] = _mm512_aesenc_epi128(t[i], b);
    }
    cap_y = _mm256_aesenc_epi128(cap_y, fastest_sum_zmm_cap(ls_z));
  }

  for (i = 0; i < z_cnt; ++i) {
    __m512i lane_z = _mm512_loadu_si512((const void *)(FASTEST_LANE[4u * i]));
    t[i] = _mm512_aesenc_epi128(_mm512_aesenc_epi128(t[i], lane_z), zero_z);
  }
  for (i = 1; i < z_cnt; ++i) {
    t[0] = _mm512_add_epi64(t[0], t[i]);
  }
  {
    __m256i cap_done = _mm256_aesenc_epi128(_mm256_aesenc_epi128(cap_y, cap_init), zero_y);
    __m256i y = _mm256_add_epi64(_mm256_add_epi64(_mm512_castsi512_si256(t[0]), _mm512_extracti64x4_epi64(t[0], 1)), cap_done);
    __m128i even = _mm256_castsi256_si128(y);
    __m128i odd = _mm256_extracti128_si256(y, 1);
    __m128i acc0 = _mm_add_epi64(even, odd);
    __m128i h0 = _mm_aesenc_si128(acc0, fastest_ld128(FASTEST_FIN[2]));
    h0 = _mm_aesenc_si128(h0, seed_x);
    h0 = _mm_aesenclast_si128(h0, fastest_ld128(FASTEST_FIN[3]));
    _mm_storeu_si128((__m128i *)(void *)root, h0);
    if (hi256) {
      __m128i acc1 = _mm_xor_si128(even, _mm_alignr_epi8(odd, odd, 14));
      __m128i h1 = _mm_aesenc_si128(acc1, fastest_ld128(FASTEST_FIN[5]));
      h1 = _mm_aesenc_si128(h1, seed_x);
      h1 = _mm_aesenclast_si128(h1, fastest_ld128(FASTEST_FIN[6]));
      _mm_storeu_si128((__m128i *)(void *)(root + 16), h1);
    }
  }
}

#elif defined(FASTEST_SIMD_AVX2)

static inline __m128i fastest_ld128(const uint8_t *p) {
  return _mm_loadu_si128((const __m128i *)(const void *)p);
}

static inline __m128i fastest_pair128(uint64_t lo, uint64_t hi) {
  uint8_t buf[16];
  fastest_pair_state(lo, hi, buf);
  return fastest_ld128(buf);
}

static inline __m256i fastest_dup128(__m128i v) {
  return _mm256_inserti128_si256(_mm256_castsi128_si256(v), v, 1);
}

static void fastest_long_hash(const uint8_t *p, size_t len, uint64_t seed, int hi256, uint8_t root[32]) {
  uint64_t ka, kb, ks;
  size_t y_cnt = (len <= 256) ? 2u : (len <= 1024 ? 4u : 8u);
  size_t stride = y_cnt * 32u;
  uint64_t len_mul = (uint64_t)len * FASTEST_M_LEN;
  __m128i seed_x, len_x;
  __m256i seed_y, len_y, zero_y, cap_init, cap_y, sum_y;
  __m256i t[8];
  const uint8_t *last = p + len - stride;
  const uint8_t *cur = p + stride;
  size_t i;

  fastest_secret_pair(seed, &ka, &kb, &ks);
  (void)ka;
  (void)kb;

  seed_x = fastest_pair128(ks, fastest_rotl64(ks, 29) ^ len_mul);
  len_x = fastest_pair128(len_mul, fastest_rotl64(len_mul, 17));
  seed_y = fastest_dup128(seed_x);
  len_y = fastest_dup128(len_x);
  zero_y = _mm256_setzero_si256();

  sum_y = _mm256_loadu_si256((const __m256i *)(const void *)p);
  t[0] = _mm256_xor_si256(_mm256_xor_si256(_mm256_loadu_si256((const __m256i *)(const void *)FASTEST_LANE[0]), seed_y), sum_y);
  for (i = 1; i < y_cnt; ++i) {
    __m256i b = _mm256_loadu_si256((const __m256i *)(const void *)(p + i * 32u));
    __m256i init = _mm256_loadu_si256((const __m256i *)(const void *)(FASTEST_LANE[2u * i]));
    sum_y = _mm256_add_epi64(sum_y, b);
    t[i] = _mm256_xor_si256(_mm256_xor_si256(init, seed_y), b);
  }
  cap_init = _mm256_loadu_si256((const __m256i *)(const void *)FASTEST_CAP);
  cap_y = _mm256_xor_si256(_mm256_xor_si256(cap_init, seed_y), _mm256_alignr_epi8(sum_y, sum_y, 14));

  while (cur < last) {
    __m256i s_y = _mm256_loadu_si256((const __m256i *)(const void *)cur);
    t[0] = _mm256_aesenc_epi128(t[0], s_y);
    for (i = 1; i < y_cnt; ++i) {
      __m256i b = _mm256_loadu_si256((const __m256i *)(const void *)(cur + i * 32u));
      s_y = _mm256_add_epi64(s_y, b);
      t[i] = _mm256_aesenc_epi128(t[i], b);
    }
    cap_y = _mm256_aesenc_epi128(cap_y, _mm256_alignr_epi8(s_y, s_y, 14));
    cur += stride;
  }

  {
    __m256i ls_y = _mm256_xor_si256(_mm256_loadu_si256((const __m256i *)(const void *)last), len_y);
    t[0] = _mm256_aesenc_epi128(t[0], ls_y);
    for (i = 1; i < y_cnt; ++i) {
      __m256i b = _mm256_xor_si256(_mm256_loadu_si256((const __m256i *)(const void *)(last + i * 32u)), len_y);
      ls_y = _mm256_add_epi64(ls_y, b);
      t[i] = _mm256_aesenc_epi128(t[i], b);
    }
    cap_y = _mm256_aesenc_epi128(cap_y, _mm256_alignr_epi8(ls_y, ls_y, 14));
  }

  for (i = 0; i < y_cnt; ++i) {
    __m256i lane_y = _mm256_loadu_si256((const __m256i *)(const void *)(FASTEST_LANE[2u * i]));
    t[i] = _mm256_aesenc_epi128(_mm256_aesenc_epi128(t[i], lane_y), zero_y);
  }
  {
    __m256i acc_y = _mm256_aesenc_epi128(_mm256_aesenc_epi128(cap_y, cap_init), zero_y);
    for (i = 0; i < y_cnt; ++i) {
      acc_y = _mm256_add_epi64(acc_y, t[i]);
    }
    {
      __m128i even = _mm256_castsi256_si128(acc_y);
      __m128i odd = _mm256_extracti128_si256(acc_y, 1);
      __m128i acc0 = _mm_add_epi64(even, odd);
      __m128i h0 = _mm_aesenc_si128(acc0, fastest_ld128(FASTEST_FIN[2]));
      h0 = _mm_aesenc_si128(h0, seed_x);
      h0 = _mm_aesenclast_si128(h0, fastest_ld128(FASTEST_FIN[3]));
      _mm_storeu_si128((__m128i *)(void *)root, h0);
      if (hi256) {
        __m128i acc1 = _mm_xor_si128(even, _mm_alignr_epi8(odd, odd, 14));
        __m128i h1 = _mm_aesenc_si128(acc1, fastest_ld128(FASTEST_FIN[5]));
        h1 = _mm_aesenc_si128(h1, seed_x);
        h1 = _mm_aesenclast_si128(h1, fastest_ld128(FASTEST_FIN[6]));
        _mm_storeu_si128((__m128i *)(void *)(root + 16), h1);
      }
    }
  }
}

#elif defined(FASTEST_SIMD_AESNI)

static inline __m128i fastest_ld128(const uint8_t *p) {
  return _mm_loadu_si128((const __m128i *)(const void *)p);
}

static inline __m128i fastest_pair128(uint64_t lo, uint64_t hi) {
  uint8_t buf[16];
  fastest_pair_state(lo, hi, buf);
  return fastest_ld128(buf);
}

static inline __m128i fastest_rot_b2_128(__m128i v) {
#ifdef __SSSE3__
  return _mm_alignr_epi8(v, v, 14);
#else
  return _mm_xor_si128(_mm_slli_si128(v, 2), _mm_srli_si128(v, 14));
#endif
}

static void fastest_long_hash(const uint8_t *p, size_t len, uint64_t seed, int hi256, uint8_t root[32]) {
  uint64_t ka, kb, ks;
  size_t lanes = (len <= 256) ? 4u : (len <= 1024 ? 8u : 16u);
  size_t stride = lanes * 16u;
  uint64_t len_mul = (uint64_t)len * FASTEST_M_LEN;
  __m128i seed_x, len_x, zero, c0, c1, even, odd;
  __m128i t[16];
  const uint8_t *last = p + len - stride;
  const uint8_t *cur = p + stride;
  size_t i;

  fastest_secret_pair(seed, &ka, &kb, &ks);
  (void)ka;
  (void)kb;

  zero = _mm_setzero_si128();
  seed_x = fastest_pair128(ks, fastest_rotl64(ks, 29) ^ len_mul);
  len_x = fastest_pair128(len_mul, fastest_rotl64(len_mul, 17));

  {
    __m128i se = fastest_ld128(p);
    __m128i so = fastest_ld128(p + 16);
    t[0] = _mm_xor_si128(_mm_xor_si128(fastest_ld128(FASTEST_LANE[0]), seed_x), se);
    t[1] = _mm_xor_si128(_mm_xor_si128(fastest_ld128(FASTEST_LANE[1]), seed_x), so);
    for (i = 1; i < lanes / 2u; ++i) {
      __m128i b0 = fastest_ld128(p + (2u * i) * 16u);
      __m128i b1 = fastest_ld128(p + (2u * i + 1u) * 16u);
      se = _mm_add_epi64(se, b0);
      so = _mm_add_epi64(so, b1);
      t[2u * i] = _mm_xor_si128(_mm_xor_si128(fastest_ld128(FASTEST_LANE[2u * i]), seed_x), b0);
      t[2u * i + 1u] = _mm_xor_si128(_mm_xor_si128(fastest_ld128(FASTEST_LANE[2u * i + 1u]), seed_x), b1);
    }
    c0 = _mm_xor_si128(_mm_xor_si128(fastest_ld128(FASTEST_CAP[0]), seed_x), fastest_rot_b2_128(se));
    c1 = _mm_xor_si128(_mm_xor_si128(fastest_ld128(FASTEST_CAP[1]), seed_x), fastest_rot_b2_128(so));
  }

  while (cur < last) {
    __m128i se = fastest_ld128(cur);
    __m128i so = fastest_ld128(cur + 16);
    t[0] = _mm_aesenc_si128(t[0], se);
    t[1] = _mm_aesenc_si128(t[1], so);
    for (i = 1; i < lanes / 2u; ++i) {
      __m128i b0 = fastest_ld128(cur + (2u * i) * 16u);
      __m128i b1 = fastest_ld128(cur + (2u * i + 1u) * 16u);
      se = _mm_add_epi64(se, b0);
      so = _mm_add_epi64(so, b1);
      t[2u * i] = _mm_aesenc_si128(t[2u * i], b0);
      t[2u * i + 1u] = _mm_aesenc_si128(t[2u * i + 1u], b1);
    }
    c0 = _mm_aesenc_si128(c0, fastest_rot_b2_128(se));
    c1 = _mm_aesenc_si128(c1, fastest_rot_b2_128(so));
    cur += stride;
  }

  {
    __m128i se = _mm_xor_si128(fastest_ld128(last), len_x);
    __m128i so = _mm_xor_si128(fastest_ld128(last + 16), len_x);
    t[0] = _mm_aesenc_si128(t[0], se);
    t[1] = _mm_aesenc_si128(t[1], so);
    for (i = 1; i < lanes / 2u; ++i) {
      __m128i b0 = _mm_xor_si128(fastest_ld128(last + (2u * i) * 16u), len_x);
      __m128i b1 = _mm_xor_si128(fastest_ld128(last + (2u * i + 1u) * 16u), len_x);
      se = _mm_add_epi64(se, b0);
      so = _mm_add_epi64(so, b1);
      t[2u * i] = _mm_aesenc_si128(t[2u * i], b0);
      t[2u * i + 1u] = _mm_aesenc_si128(t[2u * i + 1u], b1);
    }
    c0 = _mm_aesenc_si128(c0, fastest_rot_b2_128(se));
    c1 = _mm_aesenc_si128(c1, fastest_rot_b2_128(so));
  }

  even = _mm_aesenc_si128(_mm_aesenc_si128(c0, fastest_ld128(FASTEST_CAP[0])), zero);
  odd = _mm_aesenc_si128(_mm_aesenc_si128(c1, fastest_ld128(FASTEST_CAP[1])), zero);
  for (i = 0; i < lanes; ++i) {
    t[i] = _mm_aesenc_si128(_mm_aesenc_si128(t[i], fastest_ld128(FASTEST_LANE[i])), zero);
  }
  for (i = 0; i < lanes / 2u; ++i) {
    even = _mm_add_epi64(even, t[2u * i]);
    odd = _mm_add_epi64(odd, t[2u * i + 1u]);
  }

  {
    __m128i acc0 = _mm_add_epi64(even, odd);
    __m128i h0 = _mm_aesenc_si128(acc0, fastest_ld128(FASTEST_FIN[2]));
    h0 = _mm_aesenc_si128(h0, seed_x);
    h0 = _mm_aesenclast_si128(h0, fastest_ld128(FASTEST_FIN[3]));
    _mm_storeu_si128((__m128i *)(void *)root, h0);
    if (hi256) {
      __m128i acc1 = _mm_xor_si128(even, fastest_rot_b2_128(odd));
      __m128i h1 = _mm_aesenc_si128(acc1, fastest_ld128(FASTEST_FIN[5]));
      h1 = _mm_aesenc_si128(h1, seed_x);
      h1 = _mm_aesenclast_si128(h1, fastest_ld128(FASTEST_FIN[6]));
      _mm_storeu_si128((__m128i *)(void *)(root + 16), h1);
    }
  }
}

#elif defined(FASTEST_SIMD_NEON)

static inline uint8x16_t fastest_pair_v(uint64_t lo, uint64_t hi) {
  return vreinterpretq_u8_u64(vcombine_u64(vcreate_u64(lo), vcreate_u64(hi)));
}

static inline uint8x16_t fastest_add_u64x2(uint8x16_t a, uint8x16_t b) {
  return vreinterpretq_u8_u64(vaddq_u64(vreinterpretq_u64_u8(a), vreinterpretq_u64_u8(b)));
}

static void fastest_long_hash(const uint8_t *p, size_t len, uint64_t seed, int hi256, uint8_t root[32]) {
  uint64_t ka, kb, ks;
  size_t lanes = (len <= 256) ? 4u : (len <= 1024 ? 8u : 16u);
  size_t stride = lanes * 16u;
  uint64_t len_mul = (uint64_t)len * FASTEST_M_LEN;
  uint8x16_t zero = vdupq_n_u8(0);
  uint8x16_t seed_v, len_v, c0, c1, even, odd;
  uint8x16_t t[16];
  const uint8_t *last = p + len - stride;
  const uint8_t *cur = p + stride;
  size_t i;

  fastest_secret_pair(seed, &ka, &kb, &ks);
  (void)ka;
  (void)kb;

  seed_v = fastest_pair_v(ks, fastest_rotl64(ks, 29) ^ len_mul);
  len_v = fastest_pair_v(len_mul, fastest_rotl64(len_mul, 17));

  {
    uint8x16_t se = vld1q_u8(p);
    uint8x16_t so = vld1q_u8(p + 16);
    t[0] = vaesmcq_u8(vaeseq_u8(veorq_u8(vld1q_u8(FASTEST_LANE[0]), seed_v), se));
    t[1] = vaesmcq_u8(vaeseq_u8(veorq_u8(vld1q_u8(FASTEST_LANE[1]), seed_v), so));
    for (i = 1; i < lanes / 2u; ++i) {
      uint8x16_t b0 = vld1q_u8(p + (2u * i) * 16u);
      uint8x16_t b1 = vld1q_u8(p + (2u * i + 1u) * 16u);
      se = fastest_add_u64x2(se, b0);
      so = fastest_add_u64x2(so, b1);
      t[2u * i] = vaesmcq_u8(vaeseq_u8(veorq_u8(vld1q_u8(FASTEST_LANE[2u * i]), seed_v), b0));
      t[2u * i + 1u] = vaesmcq_u8(vaeseq_u8(veorq_u8(vld1q_u8(FASTEST_LANE[2u * i + 1u]), seed_v), b1));
    }
    c0 = vaesmcq_u8(vaeseq_u8(veorq_u8(vld1q_u8(FASTEST_CAP[0]), seed_v), vextq_u8(se, se, 14)));
    c1 = vaesmcq_u8(vaeseq_u8(veorq_u8(vld1q_u8(FASTEST_CAP[1]), seed_v), vextq_u8(so, so, 14)));
  }

  while (cur < last) {
    uint8x16_t se = vld1q_u8(cur);
    uint8x16_t so = vld1q_u8(cur + 16);
    t[0] = vaesmcq_u8(vaeseq_u8(t[0], se));
    t[1] = vaesmcq_u8(vaeseq_u8(t[1], so));
    for (i = 1; i < lanes / 2u; ++i) {
      uint8x16_t b0 = vld1q_u8(cur + (2u * i) * 16u);
      uint8x16_t b1 = vld1q_u8(cur + (2u * i + 1u) * 16u);
      se = fastest_add_u64x2(se, b0);
      so = fastest_add_u64x2(so, b1);
      t[2u * i] = vaesmcq_u8(vaeseq_u8(t[2u * i], b0));
      t[2u * i + 1u] = vaesmcq_u8(vaeseq_u8(t[2u * i + 1u], b1));
    }
    c0 = vaesmcq_u8(vaeseq_u8(c0, vextq_u8(se, se, 14)));
    c1 = vaesmcq_u8(vaeseq_u8(c1, vextq_u8(so, so, 14)));
    cur += stride;
  }

  {
    uint8x16_t se = veorq_u8(vld1q_u8(last), len_v);
    uint8x16_t so = veorq_u8(vld1q_u8(last + 16), len_v);
    t[0] = vaesmcq_u8(vaeseq_u8(t[0], se));
    t[1] = vaesmcq_u8(vaeseq_u8(t[1], so));
    for (i = 1; i < lanes / 2u; ++i) {
      uint8x16_t b0 = veorq_u8(vld1q_u8(last + (2u * i) * 16u), len_v);
      uint8x16_t b1 = veorq_u8(vld1q_u8(last + (2u * i + 1u) * 16u), len_v);
      se = fastest_add_u64x2(se, b0);
      so = fastest_add_u64x2(so, b1);
      t[2u * i] = vaesmcq_u8(vaeseq_u8(t[2u * i], b0));
      t[2u * i + 1u] = vaesmcq_u8(vaeseq_u8(t[2u * i + 1u], b1));
    }
    c0 = vaesmcq_u8(vaeseq_u8(c0, vextq_u8(se, se, 14)));
    c1 = vaesmcq_u8(vaeseq_u8(c1, vextq_u8(so, so, 14)));
  }

  even = vaesmcq_u8(vaeseq_u8(c0, vld1q_u8(FASTEST_CAP[0])));
  odd = vaesmcq_u8(vaeseq_u8(c1, vld1q_u8(FASTEST_CAP[1])));
  for (i = 0; i < lanes; ++i) {
    t[i] = vaesmcq_u8(vaeseq_u8(t[i], vld1q_u8(FASTEST_LANE[i])));
  }
  for (i = 0; i < lanes / 2u; ++i) {
    even = fastest_add_u64x2(even, t[2u * i]);
    odd = fastest_add_u64x2(odd, t[2u * i + 1u]);
  }

  {
    uint8x16_t acc0 = fastest_add_u64x2(even, odd);
    uint8x16_t m0 = vaesmcq_u8(vaeseq_u8(acc0, zero));
    uint8x16_t m1 = vaesmcq_u8(vaeseq_u8(m0, vld1q_u8(FASTEST_FIN[2])));
    uint8x16_t r0 = veorq_u8(vaeseq_u8(m1, seed_v), vld1q_u8(FASTEST_FIN[3]));
    vst1q_u8(root, r0);
    if (hi256) {
      uint8x16_t acc1 = veorq_u8(even, vextq_u8(odd, odd, 14));
      uint8x16_t n0 = vaesmcq_u8(vaeseq_u8(acc1, zero));
      uint8x16_t n1 = vaesmcq_u8(vaeseq_u8(n0, vld1q_u8(FASTEST_FIN[5])));
      uint8x16_t r1 = veorq_u8(vaeseq_u8(n1, seed_v), vld1q_u8(FASTEST_FIN[6]));
      vst1q_u8(root + 16, r1);
    }
  }
}

#else /* FASTEST_PORTABLE fallback */

static inline void fastest_xor16(const uint8_t a[16], const uint8_t b[16], uint8_t out[16]) {
  size_t i;
  for (i = 0; i < 16; ++i) {
    out[i] = a[i] ^ b[i];
  }
}

static inline void fastest_add16_u64x2(const uint8_t a[16], const uint8_t b[16], uint8_t out[16]) {
  uint64_t al = fastest_rd64_le(a), ah = fastest_rd64_le(a + 8);
  uint64_t bl = fastest_rd64_le(b), bh = fastest_rd64_le(b + 8);
  fastest_pair_state(al + bl, ah + bh, out);
}

static inline void fastest_rot_b2_state(const uint8_t in[16], uint8_t out[16]) {
  size_t i;
  for (i = 0; i < 16; ++i) {
    out[i] = in[(i + 14u) & 15u];
  }
}

static void fastest_long_hash(const uint8_t *p, size_t len, uint64_t seed, int hi256, uint8_t root[32]) {
  uint64_t ka, kb, ks;
  size_t lanes = (len <= 256) ? 4u : (len <= 1024 ? 8u : 16u);
  size_t stride = lanes * 16u;
  uint64_t len_mul = (uint64_t)len * FASTEST_M_LEN;
  uint8_t zero[16] = { 0 };
  uint8_t seed_vec[16], len_vec[16];
  uint8_t t[16][16], c0[16], c1[16];
  uint8_t even[16], odd[16];
  const uint8_t *last = p + len - stride;
  const uint8_t *cur = p + stride;
  size_t i;

  fastest_secret_pair(seed, &ka, &kb, &ks);
  (void)ka;
  (void)kb;

  fastest_pair_state(ks, fastest_rotl64(ks, 29) ^ len_mul, seed_vec);
  fastest_pair_state(len_mul, fastest_rotl64(len_mul, 17), len_vec);

  {
    uint8_t se[16] = { 0 }, so[16] = { 0 }, cb0[16], cb1[16];
    for (i = 0; i < lanes / 2u; ++i) {
      uint8_t tmp0[16], tmp1[16];
      fastest_add16_u64x2(se, p + (2u * i) * 16u, se);
      fastest_add16_u64x2(so, p + (2u * i + 1u) * 16u, so);
      fastest_xor16(FASTEST_LANE[2u * i], seed_vec, tmp0);
      fastest_xor16(tmp0, p + (2u * i) * 16u, t[2u * i]);
      fastest_xor16(FASTEST_LANE[2u * i + 1u], seed_vec, tmp1);
      fastest_xor16(tmp1, p + (2u * i + 1u) * 16u, t[2u * i + 1u]);
    }
    fastest_rot_b2_state(se, cb0);
    fastest_rot_b2_state(so, cb1);
    fastest_xor16(FASTEST_CAP[0], seed_vec, c0);
    fastest_xor16(c0, cb0, c0);
    fastest_xor16(FASTEST_CAP[1], seed_vec, c1);
    fastest_xor16(c1, cb1, c1);
  }

  while (cur < last) {
    uint8_t se[16] = { 0 }, so[16] = { 0 }, cb0[16], cb1[16];
    for (i = 0; i < lanes / 2u; ++i) {
      uint8_t tmp0[16], tmp1[16];
      fastest_add16_u64x2(se, cur + (2u * i) * 16u, se);
      fastest_add16_u64x2(so, cur + (2u * i + 1u) * 16u, so);
      fastest_aes_enc_tbl(t[2u * i], cur + (2u * i) * 16u, tmp0);
      fastest_aes_enc_tbl(t[2u * i + 1u], cur + (2u * i + 1u) * 16u, tmp1);
      memcpy(t[2u * i], tmp0, 16);
      memcpy(t[2u * i + 1u], tmp1, 16);
    }
    fastest_rot_b2_state(se, cb0);
    fastest_rot_b2_state(so, cb1);
    {
      uint8_t tc0[16], tc1[16];
      fastest_aes_enc_tbl(c0, cb0, tc0);
      fastest_aes_enc_tbl(c1, cb1, tc1);
      memcpy(c0, tc0, 16);
      memcpy(c1, tc1, 16);
    }
    cur += stride;
  }

  {
    uint8_t se[16] = { 0 }, so[16] = { 0 }, cb0[16], cb1[16];
    for (i = 0; i < lanes / 2u; ++i) {
      uint8_t lb0[16], lb1[16], tmp0[16], tmp1[16], clo0[16], clo1[16];
      fastest_xor16(last + (2u * i) * 16u, len_vec, lb0);
      fastest_xor16(last + (2u * i + 1u) * 16u, len_vec, lb1);
      fastest_add16_u64x2(se, lb0, se);
      fastest_add16_u64x2(so, lb1, so);
      fastest_aes_enc_tbl(t[2u * i], lb0, tmp0);
      fastest_aes_enc_tbl(t[2u * i + 1u], lb1, tmp1);
      fastest_aes_enc_tbl(tmp0, FASTEST_LANE[2u * i], clo0);
      fastest_aes_enc_tbl(tmp1, FASTEST_LANE[2u * i + 1u], clo1);
      fastest_aes_enc_tbl(clo0, zero, t[2u * i]);
      fastest_aes_enc_tbl(clo1, zero, t[2u * i + 1u]);
    }
    fastest_rot_b2_state(se, cb0);
    fastest_rot_b2_state(so, cb1);
    {
      uint8_t tc0[16], tc1[16], mid0[16], mid1[16];
      fastest_aes_enc_tbl(c0, cb0, tc0);
      fastest_aes_enc_tbl(c1, cb1, tc1);
      fastest_aes_enc_tbl(tc0, FASTEST_CAP[0], mid0);
      fastest_aes_enc_tbl(tc1, FASTEST_CAP[1], mid1);
      fastest_aes_enc_tbl(mid0, zero, even);
      fastest_aes_enc_tbl(mid1, zero, odd);
    }
  }

  for (i = 0; i < lanes / 2u; ++i) {
    fastest_add16_u64x2(even, t[2u * i], even);
    fastest_add16_u64x2(odd, t[2u * i + 1u], odd);
  }

  {
    uint8_t acc0[16], tmp[16];
    fastest_add16_u64x2(even, odd, acc0);
    fastest_aes_enc_tbl(acc0, FASTEST_FIN[2], tmp);
    fastest_aes_enc_tbl(tmp, seed_vec, acc0);
    fastest_aes_last_tbl(acc0, FASTEST_FIN[3], root);
    if (hi256) {
      uint8_t rot_odd[16], acc1[16];
      fastest_rot_b2_state(odd, rot_odd);
      fastest_xor16(even, rot_odd, acc1);
      fastest_aes_enc_tbl(acc1, FASTEST_FIN[5], tmp);
      fastest_aes_enc_tbl(tmp, seed_vec, acc1);
      fastest_aes_last_tbl(acc1, FASTEST_FIN[6], root + 16);
    }
  }
}
#endif

static inline void fastest_ctr_block(const uint8_t root[32], uint64_t counter, uint8_t out[16]) {
  const uint8_t *r0 = root;
  const uint8_t *k1 = root + 16;
  uint8_t k2[16], ctr[16], y[16], enc[16];
  uint64_t base_lo = fastest_rd64_le(r0);
  uint64_t base_hi = fastest_rd64_le(r0 + 8);
  size_t i;

  for (i = 0; i < 16; ++i) {
    k2[i] = r0[i] ^ FASTEST_STREAM_KEY[i];
  }
  fastest_pair_state(base_lo + counter, base_hi + counter * FASTEST_M_CTR, ctr);
  fastest_aes_enc_tbl(ctr, k1, y);
  fastest_aes_last_tbl(y, k2, enc);
  for (i = 0; i < 16; ++i) {
    out[i] = enc[i] ^ ctr[i];
  }
}

uint32_t fastest32(const void *data, size_t len, uint64_t seed) {
  return (uint32_t)fastest64(data, len, seed);
}

uint64_t fastest64(const void *data, size_t len, uint64_t seed) {
  const uint8_t *p = (const uint8_t *)data;
  if (len <= 64u) {
    uint64_t out[4] = { 0, 0, 0, 0 };
    fastest_short_hash(p, len, seed, 1, out);
    return out[0];
  }
  {
    uint8_t root[32];
    fastest_long_hash(p, len, seed, 0, root);
    return fastest_rd64_le(root);
  }
}

void fastest128(const void *data, size_t len, uint64_t seed, void *out) {
  const uint8_t *p = (const uint8_t *)data;
  uint8_t *dst = (uint8_t *)out;
  if (len <= 64u) {
    uint64_t w[4] = { 0, 0, 0, 0 };
    fastest_short_hash(p, len, seed, 2, w);
    fastest_wr64_le(dst, w[0]);
    fastest_wr64_le(dst + 8, w[1]);
    return;
  }
  {
    uint8_t root[32];
    fastest_long_hash(p, len, seed, 0, root);
    memcpy(dst, root, 16);
  }
}

void fastest256(const void *data, size_t len, uint64_t seed, void *out) {
  const uint8_t *p = (const uint8_t *)data;
  uint8_t *dst = (uint8_t *)out;
  if (len <= 64u) {
    uint64_t w[4] = { 0, 0, 0, 0 };
    fastest_short_hash(p, len, seed, 4, w);
    fastest_wr64_le(dst, w[0]);
    fastest_wr64_le(dst + 8, w[1]);
    fastest_wr64_le(dst + 16, w[2]);
    fastest_wr64_le(dst + 24, w[3]);
    return;
  }
  fastest_long_hash(p, len, seed, 1, dst);
}

static void fastest_stream_apply(
  const uint8_t root[32],
  uint64_t offset,
  uint8_t *buf,
  size_t len,
  int do_xor
) {
  size_t pos = 0;
  uint64_t cur = offset;
  if (len == 0u) {
    return;
  }
  if (cur < 32u) {
    size_t head = (size_t)cur;
    size_t take = 32u - head;
    size_t i;
    if (take > len) {
      take = len;
    }
    if (do_xor) {
      for (i = 0; i < take; ++i) {
        buf[i] ^= root[head + i];
      }
    } else {
      memcpy(buf, root + head, take);
    }
    pos += take;
    cur += take;
  }
  while (pos < len) {
    uint64_t rel = cur - 32u;
    uint64_t counter = rel >> 4;
    size_t blk_off = (size_t)(rel & 15u);
    size_t take = 16u - blk_off;
    uint8_t blk[16];
    size_t i;
    if (take > len - pos) {
      take = len - pos;
    }
    fastest_ctr_block(root, counter, blk);
    if (do_xor) {
      for (i = 0; i < take; ++i) {
        buf[pos + i] ^= blk[blk_off + i];
      }
    } else {
      memcpy(buf + pos, blk + blk_off, take);
    }
    pos += take;
    cur += take;
  }
}

void fastest_xof(const void *data, size_t len, uint64_t seed, void *out, size_t out_len) {
  uint8_t root[32];
  uint8_t *dst = (uint8_t *)out;
  if (out_len == 0u) {
    return;
  }
  fastest256(data, len, seed, root);
  if (out_len <= 32u) {
    memcpy(dst, root, out_len);
    return;
  }
  memcpy(dst, root, 32);
  fastest_stream_apply(root, 32u, dst + 32, out_len - 32u, 0);
}

void fastest_xor_stream(
  const void *key,
  size_t key_len,
  uint64_t seed,
  uint64_t offset,
  void *buf,
  size_t buf_len
) {
  uint8_t root[32];
  if (buf_len == 0u) {
    return;
  }
  fastest256(key, key_len, seed, root);
  fastest_stream_apply(root, offset, (uint8_t *)buf, buf_len, 1);
}
