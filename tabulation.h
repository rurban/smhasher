// Based on Thorup's "high speed hashing for integers and strings"
// https://arxiv.org/pdf/1504.06804.pdf
// rurban 2021-03-30: Added checks for bad seeds

#ifndef tabulation_included
#define tabulation_included

#include <stdint.h>
#include <assert.h>

static inline uint8_t  take08(const uint8_t *p){ uint8_t  v; memcpy(&v, p, 1); return v; }
static inline uint16_t take16(const uint8_t *p){ uint16_t v; memcpy(&v, p, 2); return v; }
static inline uint32_t take32(const uint8_t *p){ uint32_t v; memcpy(&v, p, 4); return v; }
static inline uint64_t take64(const uint8_t *p){ uint64_t v; memcpy(&v, p, 8); return v; }


////////////////////////////////////////////////////////////////////////////////
// 32 Bit Version
////////////////////////////////////////////////////////////////////////////////

const static uint64_t MERSENNE_31 = (1ull << 31) - 1;
const static int CHAR_SIZE = 8;
const static int BLOCK_SIZE_32 = 1<<8;

static uint64_t multiply_shift_random_64[BLOCK_SIZE_32];
static uint32_t multiply_shift_a_64;
static uint64_t multiply_shift_b_64;
static int32_t tabulation_32[32/CHAR_SIZE][1<<CHAR_SIZE];
static int have_broken_rand = 0;

static uint32_t combine31(uint32_t h, uint32_t x, uint32_t a) {
   uint64_t temp = (uint64_t)h * x + a;
   return ((uint32_t)temp & MERSENNE_31) + (uint32_t)(temp >> 31);
}

static uint32_t finalize_tabulation_32(uint32_t h) {
   uint32_t tab = 0;
   for (int i = 0; i < 32/CHAR_SIZE; i++, h >>= CHAR_SIZE)
      tab ^= tabulation_32[i][h & ((1<<CHAR_SIZE)-1)];
   return tab;
}

static uint32_t tabulation_32_hash(const void * key, int len_bytes, uint32_t seed) {
   const uint8_t* buf = (const uint8_t*) key;
   int len_words_32 = len_bytes/4;
   int len_blocks_32 = len_words_32/BLOCK_SIZE_32;

   uint32_t h = len_bytes ^ seed;

   for (int b = 0; b < len_blocks_32; b++) {
      uint32_t block_hash = 0;
      for (int i = 0; i < BLOCK_SIZE_32; i++, buf += 4)
         block_hash ^= multiply_shift_random_64[i] * take32(buf) >> 32;
      h = combine31(h, multiply_shift_a_64, block_hash >> 2);
   }

   int remaining_words = len_words_32 % BLOCK_SIZE_32;
   for (int i = 0; i < remaining_words; i++, buf += 4)
      h ^= multiply_shift_random_64[i] * take32(buf) >> 32;

   int remaining_bytes = len_bytes % 4;
   if (remaining_bytes) {
      uint32_t last = 0;
      if (remaining_bytes & 2) {last = take16(buf); buf += 2;}
      if (remaining_bytes & 1) {last = (last << 8) | take08(buf);}
      h ^= multiply_shift_b_64 * last >> 32;
   }

   return finalize_tabulation_32(h);
}


static uint64_t tab_rand64() {
   // we don't know how many bits we get from rand(),
   // but it is at least 16, so we concatenate a couple.
   uint64_t r = 0;
   for (int i = 0; i < 4; i++) {
      r <<= 16;
      r ^= rand();
   }
   return r;
}

static void tabulation_32_seed_init(size_t &seed) {
   srand(seed);
   // the lazy mersenne combination requires 30 bits values in the polynomial.
   multiply_shift_a_64 = tab_rand64() & ((1ull<<30)-1);
   if (!multiply_shift_a_64) {
      multiply_shift_a_64 = tab_rand64() & ((1ull<<30)-1);
   }
   if (!multiply_shift_a_64) {
      have_broken_rand = 1;
      multiply_shift_a_64 = 0xababababbeafcafeULL & ((1ull<<30)-1);
   }
   multiply_shift_b_64 = tab_rand64();
   if (!multiply_shift_b_64) {
      multiply_shift_b_64 = have_broken_rand ? 0xdeadbeef : tab_rand64();
   }
   for (int i = 0; i < BLOCK_SIZE_32; i++) {
      multiply_shift_random_64[i] = tab_rand64();
      if (!multiply_shift_random_64[i]) {
         multiply_shift_random_64[i] = have_broken_rand ? 0xdeadbeef : tab_rand64();
      }
   }
   for (int i = 0; i < 32/CHAR_SIZE; i++)
      for (int j = 0; j < 1<<CHAR_SIZE; j++)
         tabulation_32[i][j] = tab_rand64();
}



////////////////////////////////////////////////////////////////////////////////
// 64 Bit Version
////////////////////////////////////////////////////////////////////////////////

#ifdef __SIZEOF_INT128__

const static uint64_t TAB_MERSENNE_61 = (1ull << 61) - 1;
// multiply shift works on fixed length strings, so we operate in blocks.
// this size can be tuned depending on the system.
const static int TAB_BLOCK_SIZE = 1<<8;

static __uint128_t tab_multiply_shift_random[TAB_BLOCK_SIZE];
static __uint128_t tab_multiply_shift_a;
static __uint128_t tab_multiply_shift_b;

static int64_t tabulation[64/CHAR_SIZE][1<<CHAR_SIZE];

static uint64_t combine61(uint64_t h, uint64_t x, uint64_t a) {
   // we assume 2^b-1 >= 2u-1. in other words
   // x <= u-1 <= 2^(b-1)-1 (at most 60 bits)
   // a <= p-1  = 2^b-2     (60 bits suffices)
      // actually, checking the proof, it's fine if a is 61 bits.
   // h <= 2p-1 = 2^62-3. this will also be guaranteed of the output.
   __uint128_t temp = (__uint128_t)h * x + a;
   return ((uint64_t)temp & TAB_MERSENNE_61) + (uint64_t)(temp >> 61);
}


static uint64_t finalize_tabulation(uint64_t h) {
   uint64_t tab = 0;
   for (int i = 0; i < 64/CHAR_SIZE; i++, h >>= CHAR_SIZE)
      tab ^= tabulation[i][h % (1<<CHAR_SIZE)];
   return tab;
}


static uint64_t tabulation_hash(const void * key, int len_bytes, uint32_t seed) {
   const uint8_t* buf = (const uint8_t*) key;

   // the idea is to compute a fast "signature" of the string before doing
   // tabulation hashing. this signature only has to be collision resistant,
   // so we can use the variabe-length-hashing polynomial mod-mersenne scheme
   // from thorup.
   // because of the birthday paradox, the signature needs to be around twice
   // as many bits as in the number of keys tested. since smhasher tests
   // collisions in keys in the order of millions, we need the signatures to
   // be at least 40 bits. we settle on 64.

   // we mix in len_bytes in the basis, since smhasher considers two keys
   // of different length to be different, even if all the extra bits are 0.
   // this is needed for the appendzero test.

   uint64_t h = len_bytes ^ seed ^ (seed << 8);

   if (len_bytes >= 8) {
      const int len_words = len_bytes/8;
      if (len_words >= TAB_BLOCK_SIZE) {
         const int len_blocks = len_words/TAB_BLOCK_SIZE;

         // to save time, we partition the string in blocks of ~ 256 words.
         // each word is hashed using a fast strongly-universal multiply-shift,
         // and since the xor of independent strongly-universal hash functions
         // is also universal, we get a unique value for each block.
         for (int b = 0; b < len_blocks; b++) {
            uint64_t block_hash = 0;
            for (int i = 0; i < TAB_BLOCK_SIZE; i++, buf += 8) {
               // we don't have to shift yet, but shifting by 64 allows the
               // compiler to produce a single "high bits only" multiplication instruction.
               block_hash ^= (tab_multiply_shift_random[i] * take64(buf)) >> 64;

               // the following is very fast, basically using mum, but theoretically wrong.
               // __uint128_t mum = (__uint128_t)tab_multiply_shift_random_64[i] * take64(buf);
               // block_hash ^= mum ^ (mum >> 64);
            }

            // finally we combine the block hash using variable length hashing.
            // values have to be less than mersenne for the combination to work.
            // we can shift down, since any shift of multiply-shift outputs is
            // strongly-universal.
            h = combine61(h, tab_multiply_shift_a, block_hash >> 4);
         }

         // in principle we should finish the mersenne modular reduction.
         // however, this isn't be needed, since it can never reduce collisions.
         // if (h >= TAB_MERSENNE_61) h -= TAB_MERSENNE_61;
      }

      // then read the remaining words
      const int remaining_words = len_words % TAB_BLOCK_SIZE;
      for (int i = 0; i < remaining_words; i++, buf += 8)
         h ^= tab_multiply_shift_random[i] * take64(buf) >> 64;
   }

   // now get the remaining bytes
   const int remaining_bytes = len_bytes % 8;
   if (remaining_bytes) {
      uint64_t last = 0;
      if (remaining_bytes & 4) {last = take32(buf); buf += 4;}
      if (remaining_bytes & 2) {last = (last << 16) | take16(buf); buf += 2;}
      if (remaining_bytes & 1) {last = (last << 8) | take08(buf);}
      h ^= tab_multiply_shift_b * last >> 64;
   }

   return finalize_tabulation(h);
}

static __uint128_t tab_rand128() {
   return (__uint128_t)tab_rand64() << 64 | tab_rand64();
}

static void tabulation_seed_init(size_t &seed) {
   srand(seed);
   // the lazy mersenne combination requires 60 bits values in the polynomial.
   // rurban: added checks for bad seeds
   tab_multiply_shift_a = tab_rand128() & ((1ull<<60)-1);
   tab_multiply_shift_b = tab_rand128();
   if (!tab_multiply_shift_a) tab_multiply_shift_a = tab_rand128() & ((1ull<<60)-1);
   if (!tab_multiply_shift_a) {
      have_broken_rand = 1;
      tab_multiply_shift_a = 0xababababbeafcafeULL & ((1ull<<60)-1);
   }
   if (!tab_multiply_shift_b) tab_multiply_shift_b = tab_rand128();
   if (!tab_multiply_shift_b) {
      have_broken_rand = 1;
      tab_multiply_shift_b++;
   }
   for (int i = 0; i < TAB_BLOCK_SIZE; i++) {
      tab_multiply_shift_random[i] = tab_rand128();
      if (!tab_multiply_shift_random[i])
         tab_multiply_shift_random[i] = 0x12345678;
   }
   if (have_broken_rand)
      assert(TAB_BLOCK_SIZE >= 64/CHAR_SIZE);
   for (int i = 0; i < 64/CHAR_SIZE; i++)
      for (int j = 0; j < 1<<CHAR_SIZE; j++)
         tabulation[i][j] = have_broken_rand ? tab_multiply_shift_random[i] : tab_rand128();
}

#endif
#endif
