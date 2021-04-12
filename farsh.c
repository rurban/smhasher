#include "farsh.h"

//#include <stddef.h>   /* for size_t */
//#include <stdint.h>   /* for uint32_t & uint64_t */
#include <stdlib.h>   /* for abort() */
#include <memory.h>   /* for memcpy() */

#if __GNUC__
#if defined(__aarch64__)
       #include "sse2neon.h"
#else
       #include <x86intrin.h>
#endif
#define ALIGN(n)      __attribute__ ((aligned(n)))
#elif _MSC_VER
#include <intrin.h>
#define ALIGN(n)      __declspec(align(n))
#else
#define ALIGN(n)
#endif

#define STRIPE          FARSH_BASE_KEY_SIZE
#define STRIPE_ELEMENTS (STRIPE/sizeof(uint32_t))  /* should be power of 2 due to use of 'x % STRIPE_ELEMENTS' below */
#define EXTRA_ELEMENTS  (((FARSH_MAX_HASHES-1) * FARSH_EXTRA_KEY_SIZE) / sizeof(uint32_t))

ALIGN(64) static const uint32_t FARSH_KEYS [STRIPE_ELEMENTS + EXTRA_ELEMENTS] = {  /* STRIPE bytes of key material plus extra keys for hashes up to 1024 bits long */
    0xb8fe6c39,0x23a44bbe,0x7c01812c,0xf721ad1c,0xded46de9,0x839097db,0x7240a4a4,0xb7b3671f,0xcb79e64e,0xccc0e578,0x825ad07d,0xccff7221,0xb8084674,0xf743248e,0xe03590e6,0x813a264c,0x3c2852bb,0x91c300cb,0x88d0658b,0x1b532ea3,0x71644897,0xa20df94e,0x3819ef46,0xa9deacd8,0xa8fa763f,0xe39c343f,0xf9dcbbc7,0xc70b4f1d,0x8a51e04b,0xcdb45931,0xc89f7ec9,0xd9787364,0x4f6a0752,0xa79b079c,0x8fc49499,0x8ec9b7a9,0x33c92249,0x4eb6404f,0xfb2afb4e,0xa4814255,0x2f0e1b98,0xace93b24,0x188850cd,0x6c5c74a7,0x66fa4404,0xeac5ac83,0x34d3ebc3,0xc581a0ff,0xfa1363eb,0x170ddd51,0xb7f0da49,0xd3165526,0x29d4689e,0x2b16be58,0x7d47a1fc,0x8ff8b8d1,0x7ad031ce,0x45cb3a8f,0x95160428,0xafd7fbca,0xbb4b407e,0x995274a4,0xeb9a2d93,0x3be78908,0xed475f6c,0x919cd8f2,0xd3861e5a,0x6e31390c,0xfe6a3a49,0xdcad0914,0x06508beb,0xa88399f3,0xb058112f,0xe8b0fa79,0x29b4da06,0xedc253fb,0xc3e96dad,0x6e372b83,0x4f78b153,0xfffa6e86,0x21beeeec,0x01caea02,0x1267e50d,0x11e6092f,0xe819d298,0x832f80dd,0x0c4e2477,0xbc7886eb,0x01506637,0x8ba89668,0x6d11e7a0,0xfc12fd15,0x86a54c19,0x593ce3dd,0xd2b13fe5,0x8e772b53,0xae4a60cc,0x647a3b1b,0x547786e0,0x3ec4378e,0x8d7acf89,0xca36f947,0x0e89d5ef,0xaada6a3c,0x6da4a109,0x9ac6e11c,0x686691ef,0xa357bd2b,0xd16f1b9a,0x38c70303,0x7d4622b3,0x2968fa8f,0x8ca5bcb9,0xfcd61005,0x228b5e96,0x2c9dcc19,0x57cf243c,0x3c53f9c1,0x0cc7952c,0x686de4f0,0x93a747b5,0x4e87a510,0x975e91ae,0x4c10b98e,0x8a7f068c,0x346b19ab,0x353ca625,0xf20a50e0,0xce9921f6,0xdf66e014,0x0a11ef4b,0x8bc84ddf,0x84d25d22,0xc823936d,0x94741ec3,0x88278a60,0xb8649331,0x7a707a10,0x7292cad6,0xa7c644c2,0xbd156bfa,0x646c9578,0xb7f4dfd5,0x9f8277a7,0x7013924e,0xad674cc3,0x2cae9d05,0x912a9a22,0xf67c53fa,0x8d7e22a9,0x59ae372b,0x850199f3,0x63a2102c,0xd6ff1261,0x56738ee1,0xaa95145b,0xfdd12832,0x5b684deb,0x0784de94,0xaa62390e,0xbb7ccf19,0x0fefd572,0x565b41ca,0x2206d202,0x2d608479,0x4c0fcd3d,0xd36d3be3,0x155a9a65,0x10f9e732,0xac9b0f1e,0x1f72a03b,0xea9440ae,0x5b674b4f,0x31a827d1,0xecca954f,0x3d2cd61e,0x768d3da4,0x93745ac1,0x1d5d58cb,0x4b86f3b6,0x2aba923a,0x0e65814c,0x8ae063d9,0xcd6969b0,0x36641585,0x742af59d,0x613a1316,0x338ea471,0x47861af3,0x30479dc3,0x1270a481,0x08771069,0xe3c4f0d2,0x0229874c,0x5a8a3bc1,0xe30d9733,0xd05be5a2,0xe2af31ba,0x222049f9,0x9f923b6a,0x033f64ec,0xe528b62b,0x8201efbd,0x2107d877,0xd8312ef1,0xa5679f99,0x1730b51b,0x752616d2,0x05305909,0x0dca440b,0x2093cdd9,0x6409ab50,0xba5c8ecc,0x8d4708ea,0x429f0917,0xb762fab0,0x5161ea75,0x45eba0eb,0xb6f34b41,0x52047123,0xe4181523,0x8d74e90a,0x54fa401c,0xddda0cc7,0x63df182a,0xc6403ef6,0x348ec6e8,0xb9ff57f5,0xf652b8bd,0x0f86b0f3,0xfb3a088a,0x4dc71533,0x7b3617d2,0xa34e87eb,0xba2a9bdd,0xe3381306,0x14bad6bb,0xc96dc7c2,0x333b54b6,0x9be47cfa,0x1dcf9299,0xe7ea5f99,0xb38feacd,0xc3cfe2f7,0x5b87e822,0x39c5ab56,0x18f4a18f,0x2d484d9c,0x4163d519,0x79769e98,0xf58a67f0,0x40590c02,0x319671c0,0x266b133a,0xaf81b287,0x6a31f737,
    0xe3bc0197,0x55079913,0x9f72c696,0x363e00c8,0x53153947,0xebfd127f,0x00f60519,0x46a6b62a,0x93b83380,0x3fe29324,0xdfc67091,0x0f62386d,0xdc375e79,0x8fea3f3e,0xdf8463d0,0x3702fa7b,0x3954435e,0x87caa648,0xa9158bee,0x08f30c25,0x66b82936,0xe7fc3feb,0x183c5450,0xd7ef4345,0x798c7963,0xc02cf557,0x098553d1,0xfa4312aa,0xe29ef883,0x7caf128d,0x74b3a07d,0xc8efdf5b,0x8db23782,0x2c409f4a,0xdae469da,0x4d3e1b3f,0x2e7b9a58,0xc83e3753,0xcefd96a6,0x44ddb068,0x5faed141,0xdee7d0f1,0xc223dbb4,0x7bfbe104,0x114d6e1d,0x52039cd5,0x307c0a9c,0xa6289c12,0x20ee8b3e,0x03724b0b,0xba68ae4a,0x93c5f2a1,0x9af27bb2,0x480f0eba,0xc14c6bbe,0xe7331f87,0xf0104df4,0x22c05363,0xb7e6d08a,0x6f15c449,0x4b9ee2cd,0x6b2c78ae,0x25ed2673,0xb6256596,0x99ad4803,0x654f8f10,0xe89eca64,0xd9a506df,0x530dc5fa,0xfe75be5c,0xa543833d,0xf739fd45,0x1605b488,0xe50f614a,0xe930df83,0x4540195d,0xf2da0f32,0x6b04f79c,0xe3c73c99,0xb3a5265c,0x5a1be07d,0xbda13d2a,0xeddc281c,0xe9d9a39a,0xde9beff1,0x573c1747,0x40be5b3e,0x3756e968,0x968077b6,0x6525a28f,0x747d0735,0x8a0ec11d,0x49c03af5,0xf3def45b,0xc3c9214d,0x9ea2e76d,0xfad3a715,0xcaa7ad89,0xde828e4c,0xa5769bd5,0x467cdb5a,0xd5f2cacb,0x68ebd182,0x8d40341a,0x21556887,0x000a5f6f,0x5ad8a473,0xafe7e886,0x98997d39,0x945ad218,0x46be0c93,0x93a5bd3a,0x3ffa4a8c,0xd834d936,0x2f022a2a,0x20791c6b,0x5db51516,0x8defeed2,0x9dee28a5,0x5188eba7,0xab4f8c67,0x48ceac96,0x2a11e16f,0xc1593b6d
    };

/* Internal: hash exactly STRIPE bytes */
static uint64_t farsh_full_block (const uint32_t *data, const uint32_t *key)
{
#ifdef FARSH_AVX2
    __m256i sum = _mm256_setzero_si256();  __m128i sum128;  int i;
    const __m256i *xdata = (const __m256i *) data;
    const __m256i *xkey  = (const __m256i *) key;

    for (i=0; i < STRIPE/sizeof(__m256i); i++)
    {
        __m256i d = _mm256_loadu_si256 (xdata+i);
        __m256i k = _mm256_loadu_si256 (xkey+i);
        __m256i dk = _mm256_add_epi32(d,k);                                     // uint32 dk[8]  = {d0+k0, d1+k1 .. d7+k7}
        __m256i res = _mm256_mul_epu32 (dk, _mm256_shuffle_epi32 (dk,0x31));    // uint64 res[4] = {dk0*dk1, dk2*dk3, dk4*dk5, dk6*dk7}
        sum = _mm256_add_epi64(sum,res);
    }
    sum = _mm256_add_epi64 (sum, _mm256_shuffle_epi32(sum,3*4+2));              // return sum of four 64-bit values in the sum
    sum128 = _mm_add_epi64 (_mm256_castsi256_si128(sum), _mm256_extracti128_si256(sum,1));
    return *(uint64_t*) &sum128;
#elif defined(FARSH_SSE2)
    __m128i sum = _mm_setzero_si128();  int i;
    const __m128i *xdata = (const __m128i *) data;
    const __m128i *xkey  = (const __m128i *) key;

    for (i=0; i < STRIPE/sizeof(__m128i); i++)
    {
#ifdef FARSH_ALIGNED_INPUT
        __m128i d = _mm_load_si128 (xdata+i);
#else
        __m128i d = _mm_loadu_si128 (xdata+i);
#endif
        __m128i k = _mm_load_si128 (xkey+i);
        __m128i dk = _mm_add_epi32(d,k);                                        // uint32 dk[4]  = {d0+k0, d1+k1, d2+k2, d3+k3}
        __m128i res = _mm_mul_epu32 (dk, _mm_shuffle_epi32 (dk,0x31));          // uint64 res[2] = {dk0*dk1,dk2*dk3}
        sum = _mm_add_epi64(sum,res);
    }
    sum = _mm_add_epi64 (sum, _mm_shuffle_epi32(sum,3*4+2));                    // return sum of two 64-bit values in the sum
    return *(uint64_t*) &sum;
#else
    uint64_t sum = 0;  int i;
    for (i=0; i < STRIPE_ELEMENTS; i+=2)
        sum += (data[i] + key[i]) * (uint64_t)(data[i+1] + key[i+1]);
    return sum;
#endif
}

/* Internal: hash less than STRIPE bytes, with careful handling of partial uint32_t pair at the end of buffer */
static uint64_t farsh_partial_block (const uint32_t *data, size_t bytes, const uint32_t *key)
{
    uint64_t sum = 0;  int i;
    size_t elements = (bytes/sizeof(uint32_t)) & (~1);

    uint32_t extra_data[2] = {0};
    size_t extra_bytes = bytes - elements*sizeof(uint32_t);
    memcpy (extra_data, data+elements, extra_bytes);

    for (i=0; i < elements; i+=2)
        sum += (data[i] + key[i]) * (uint64_t)(data[i+1] + key[i+1]);
    if (extra_bytes)
        sum += (extra_data[0] + key[i]) * (uint64_t)(extra_data[1] + key[i+1]);
    return sum;
}

/* ////////////////////////////////////////////////////////////////////////// */
/* Hash mixing code, including all constants, was kidnapped from the xxHash64 */

/* Internal: combine hash of the current block with overall hashsum */
static uint64_t farsh_combine (uint64_t sum, uint64_t h)
{
    uint64_t PRIME64_1 = 11400714785074694791ULL;
    uint64_t PRIME64_2 = 14029467366897019727ULL;
    uint64_t PRIME64_4 =  9650029242287828579ULL;
    h *= PRIME64_2;
    h += h >> 31;
    h *= PRIME64_1;
    sum ^= h;
    sum = (sum+(sum>>27)) * PRIME64_1 + PRIME64_4;
    return sum;
}

/* Internal: compute the final hashsum value */
static uint32_t farsh_final (uint64_t sum)
{
    uint64_t PRIME64_2 = 14029467366897019727ULL;
    uint64_t PRIME64_3 =  1609587929392839161ULL;
    sum ^= sum >> 33;
    sum *= PRIME64_2;
    sum ^= sum >> 29;
    sum *= PRIME64_3;
    return (uint32_t)sum ^ (uint32_t)(sum >> 32);
}
/* End of hash mixing code kidnapped from the xxHash64 */
/* ////////////////////////////////////////////////////////////////////////// */


/* Public API functions documented in farsh.h */

uint32_t farsh_keyed (const void *data, size_t bytes, const void *key, uint64_t seed)
{
    uint64_t sum = seed;
    const char *ptr     = (const char*) data;
    const uint32_t *key_ptr = (const uint32_t*) key;
    while (bytes >= STRIPE)
    {
        size_t chunk = STRIPE;
        uint64_t h = farsh_full_block ((const uint32_t*)ptr, key_ptr);
        sum = farsh_combine (sum, h);
        ptr += chunk;  bytes -= chunk;
    }
    if (bytes)
    {
        size_t chunk = bytes;
        uint64_t h = farsh_partial_block ((const uint32_t*)ptr, chunk, key_ptr);
        sum = farsh_combine (sum, h);
        ptr += chunk;  bytes -= chunk;
    }
    return farsh_final(sum) ^ key_ptr[bytes%STRIPE_ELEMENTS];   /* ensure that zeroes at the end of data will affect the hash value */
}

void farsh_keyed_n (const void *data, size_t bytes, const void *key, int n, uint64_t seed, void *hash)
{
    int i;  uint32_t *hash_ptr = (uint32_t*)hash;
    for (i=0; i < n; i++)
        hash_ptr[i] = farsh_keyed (data, bytes, (const char*)key + i*FARSH_EXTRA_KEY_SIZE, seed);
}

void farsh_n (const void *data, size_t bytes, int k, int n, uint64_t seed, void *hash)
{
    if (k+n > FARSH_MAX_HASHES)  abort();  /* FARSH_KEYS contains only material for the hashes 0..FARSH_MAX_HASHES-1 */
    farsh_keyed_n (data, bytes, (const char*)FARSH_KEYS + k*FARSH_EXTRA_KEY_SIZE, n, seed, hash);
}

uint32_t farsh (const void *data, size_t bytes, uint64_t seed)
{
    return farsh_keyed (data, bytes, FARSH_KEYS, seed);
}

#undef EXTRA_ELEMENTS
#undef STRIPE
#undef STRIPE_ELEMENTS
#undef ALIGN
