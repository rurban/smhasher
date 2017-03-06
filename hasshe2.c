/* http://cessu.blogspot.com/2008/11/hashing-with-sse2-revisited-or-my-hash.html */
/* Compile with gcc -O3 -msse2 ... */

#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <string.h>

#ifdef __SSE2__
#include <xmmintrin.h>

static uint32_t coeffs[12] __attribute__((aligned(16))) = {
  /* Four carefully selected coefficients and interleaving zeros. */
  2561893793UL, 0, 1388747947UL, 0,
  3077216833UL, 0, 3427609723UL, 0,
  /* 128 bits of random data. */
  0x564A4447, 0xC7265595, 0xE20C241D, 0x128FA608,
};

#define COMBINE_AND_MIX(c_1, c_2, s_1, s_2, in)                              \
  /* Phase 1: Perform four 32x32->64 bit multiplication with the             \
     input block and words 1 and 3 coeffs, respectively.  This               \
     effectively propagates a bit change in input to 32 more                 \
     significant bit positions.  Combine into internal state by              \
     subtracting the result of multiplications from the internal             \
     state. */                                                               \
  s_1 = _mm_sub_epi64(s_1, _mm_mul_epu32(c_1, _mm_unpackhi_epi32(in, in)));  \
  s_2 = _mm_sub_epi64(s_2, _mm_mul_epu32(c_2, _mm_unpacklo_epi32(in, in)));  \
                                                                             \
  /* Phase 2: Perform shifts and xors to propagate the 32-bit                \
     changes produced above into 64-bit (and even a little larger)           \
     changes in the internal state. */                                       \
  /* state ^= state >64> 29; */                                              \
  s_1 = _mm_xor_si128(s_1, _mm_srli_epi64(s_1, 29));                         \
  s_2 = _mm_xor_si128(s_2, _mm_srli_epi64(s_2, 29));                         \
  /* state +64= state <64< 16; */                                            \
  s_1 = _mm_add_epi64(s_1, _mm_slli_epi64(s_1, 16));                         \
  s_2 = _mm_add_epi64(s_2, _mm_slli_epi64(s_2, 16));                         \
  /* state ^= state >64> 21; */                                              \
  s_1 = _mm_xor_si128(s_1, _mm_srli_epi64(s_1, 21));                         \
  s_2 = _mm_xor_si128(s_2, _mm_srli_epi64(s_2, 21));                         \
  /* state +64= state <128< 32; */                                           \
  s_1 = _mm_add_epi64(s_1, _mm_slli_si128(s_1, 4));                          \
  s_2 = _mm_add_epi64(s_2, _mm_slli_si128(s_2, 4));                          \
                                                                             \
  /* Phase 3: Propagate the changes among the four 64-bit words by           \
     performing 64-bit subtractions and 32-bit word shuffling. */            \
  s_1 = _mm_sub_epi64(s_1, s_2);                                             \
  s_2 = _mm_sub_epi64(_mm_shuffle_epi32(s_2, _MM_SHUFFLE(0, 3, 2, 1)), s_1); \
  s_1 = _mm_sub_epi64(_mm_shuffle_epi32(s_1, _MM_SHUFFLE(0, 1, 3, 2)), s_2); \
  s_2 = _mm_sub_epi64(_mm_shuffle_epi32(s_2, _MM_SHUFFLE(2, 1, 0, 3)), s_1); \
  s_1 = _mm_sub_epi64(_mm_shuffle_epi32(s_1, _MM_SHUFFLE(2, 1, 0, 3)), s_2); \
                                                                             \
  /* With good coefficients any one-bit flip in the input has now            \
     changed all bits in the internal state with a probability               \
     between 45% to 55%. */

void hasshe2(const void *input_buf, int n_bytes, const void *state, void *output_state)
{
  __m128i coeffs_1, coeffs_2, rnd_data, input, state_1, state_2;


  coeffs_1 = _mm_load_si128((void *) coeffs);
  coeffs_2 = _mm_load_si128((void *) (coeffs + 4));
  rnd_data = _mm_load_si128((void *) state);

  /* Initialize internal state to something random.  (Alternatively,
     if hashing a chain of data, read in the previous hash result from
     somewhere.) */
  state_1 = state_2 = rnd_data;

  while (n_bytes >= 16) {
    /* Read in 16 bytes, or 128 bits, from buf.  Advance buf and
       decrement n_bytes accordingly. */
    input = _mm_loadu_si128((void *) input_buf);
    input_buf += 16;
    n_bytes -= 16;

    COMBINE_AND_MIX(coeffs_1, coeffs_2, state_1, state_2, input);
  }
  if (n_bytes) {
    uint8_t tmp_buf[16];
    memset(tmp_buf,n_bytes,16);
    memcpy(tmp_buf,input_buf,n_bytes);
    input = _mm_loadu_si128((void *) tmp_buf);
    COMBINE_AND_MIX(coeffs_1, coeffs_2, state_1, state_2, input);
  }

  /* Postprocessing.  Copy half of the internal state into fake input,
     replace it with the constant rnd_data, and do one combine and mix
     phase more. */ 
  input = state_1;
  state_1 = rnd_data;
  COMBINE_AND_MIX(coeffs_1, coeffs_2, state_1, state_2, input);

  _mm_storeu_si128((void *) output_state, state_1);
  _mm_storeu_si128((void *) (output_state + 16), state_2);
}

#endif
