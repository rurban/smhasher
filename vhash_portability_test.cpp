// Standalone regression: link with vmac.cpp and rijndael-alg-fst.c.
// Mathematical reference: Krovetz and Dai, draft-krovetz-vmac-01,
// sections 3 and 5 and Appendix, https://www.ietf.org/archive/id/draft-krovetz-vmac-01.txt
// The seed extension is specific to the SMHasher adapters: XOR the seed
// into the low 32 bits of the initial polynomial high word.
#include "vmac.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <vector>

struct U128 { uint64_t hi, lo; };

static U128 add128(U128 a, U128 b)
{
    U128 r = {a.hi + b.hi, a.lo + b.lo};
    r.hi += r.lo < a.lo;
    return r;
}

static U128 shl128(U128 a)
{
    U128 r = {(a.hi << 1) | (a.lo >> 63), a.lo << 1};
    return r;
}

static U128 mul64(uint64_t a, uint64_t b)
{
    U128 sum = {0, 0}, term = {0, a};
    for (unsigned i = 0; i < 64; ++i) {
        if (b & 1) sum = add128(sum, term);
        term = shl128(term);
        b >>= 1;
    }
    return sum;
}

// Addition and multiplication modulo 2^127-1 using bit-at-a-time arithmetic.
static U128 add127(U128 a, U128 b)
{
    U128 r = add128(a, b);
    uint64_t top = r.hi >> 63;
    r.hi &= UINT64_C(0x7fffffffffffffff);
    r = add128(r, U128{0, top});
    if (r.hi == UINT64_C(0x7fffffffffffffff) && r.lo == UINT64_MAX)
        r = U128{0, 0};
    return r;
}

static U128 mul127(U128 a, U128 b)
{
    U128 r = {0, 0};
    for (unsigned i = 0; i < 127; ++i) {
        if (b.lo & 1) r = add127(r, a);
        a = add127(a, a);
        b.lo = (b.lo >> 1) | (b.hi << 63);
        b.hi >>= 1;
    }
    return r;
}

static uint64_t load_be(const unsigned char *p)
{
    uint64_t r = 0;
    for (unsigned i = 0; i < 8; ++i) r = (r << 8) | p[i];
    return r;
}

static uint64_t load_le(const unsigned char *p)
{
    uint64_t r = 0;
    for (unsigned i = 0; i < 8; ++i) r |= (uint64_t)p[i] << (8 * i);
    return r;
}

static const uint64_t prime64 = UINT64_C(0xfffffffffffffeff);

static uint64_t add64mod(uint64_t a, uint64_t b)
{
    return a >= prime64 - b ? a - (prime64 - b) : a + b;
}

static uint64_t mul64mod(uint64_t a, uint64_t b)
{
    uint64_t r = 0;
    while (b) {
        if (b & 1) r = add64mod(r, a);
        a = add64mod(a, a);
        b >>= 1;
    }
    return r;
}

// Long division of a 127-bit number by 2^64 - 2^32.
static void divide_l3(U128 n, uint64_t &quotient, uint64_t &remainder)
{
    const uint64_t divisor = UINT64_C(0xffffffff00000000);
    quotient = remainder = 0;
    for (int i = 126; i >= 0; --i) {
        unsigned bit = i >= 64 ? (unsigned)((n.hi >> (i - 64)) & 1)
                               : (unsigned)((n.lo >> i) & 1);
        bool overflow = (remainder >> 63) != 0;
        remainder = (remainder << 1) | bit;
        bool subtract = overflow || remainder >= divisor;
        if (subtract) remainder -= divisor;
        quotient = (quotient << 1) | (unsigned)subtract;
    }
}

struct Reference {
    aes_int_key aes;
    uint64_t nh[16], kh, kl, l3a, l3b;

    Reference()
    {
        unsigned char key[16] = {'a','b','c','d','e','f','g','h',
                                 'i','j','k','l','m','n','o','p'};
        aes_key_setup(key, &aes);
        unsigned char in[16] = {}, out[16];
        in[0] = 0x80;
        for (unsigned i = 0; i < 16; i += 2) {
            aes_encryption(in, out, &aes);
            nh[i] = load_be(out);
            nh[i + 1] = load_be(out + 8);
            ++in[15];
        }
        memset(in, 0, sizeof(in)); in[0] = 0xc0;
        aes_encryption(in, out, &aes);
        kh = load_be(out) & UINT64_C(0x1fffffff1fffffff);
        kl = load_be(out + 8) & UINT64_C(0x1fffffff1fffffff);
        memset(in, 0, sizeof(in)); in[0] = 0xe0;
        do {
            aes_encryption(in, out, &aes);
            l3a = load_be(out); l3b = load_be(out + 8);
            ++in[15];
        } while (l3a >= prime64 || l3b >= prime64);
    }

    uint64_t hash(const unsigned char *msg, size_t len, uint32_t seed) const
    {
        U128 y = {kh ^ seed, kl};
        U128 multiplier = {kh, kl};
        size_t offset = 0;
        bool first = true;
        while (offset < len) {
            size_t count = len - offset < 128 ? len - offset : 128;
            unsigned char block[128] = {};
            memcpy(block, msg + offset, count);
            U128 n = {0, 0};
            for (size_t i = 0; i < (count + 15) / 16; ++i) {
                uint64_t a = load_le(block + 16 * i) + nh[2 * i];
                uint64_t b = load_le(block + 16 * i + 8) + nh[2 * i + 1];
                n = add128(n, mul64(a, b));
            }
            n.hi &= UINT64_C(0x3fffffffffffffff);
            if (!first) y = mul127(y, multiplier);
            y = add127(y, n);
            first = false;
            offset += count;
        }
        y = add127(y, U128{(uint64_t)((len % 128) * 8), 0});
        uint64_t a, b;
        divide_l3(y, a, b);
        return mul64mod(add64mod(a, l3a), add64mod(b, l3b));
    }

    uint64_t published_pad()
    {
        // Appendix nonce "bcdefghi": pad to 128 bits and clear low bit.
        unsigned char in[16] = {0,0,0,0,0,0,0,0,'b','c','d','e','f','g','h','h'};
        unsigned char out[16];
        aes_encryption(in, out, &aes);
        return load_be(out + 8); // Original nonce low bit selects second half.
    }
};

static int failures;

static void check(bool ok, const char *what, size_t len, uint32_t seed, unsigned align)
{
    if (!ok) {
        fprintf(stderr, "%s: len=%zu seed=%08x alignment=%u\n", what, len,
                (unsigned)seed, align);
        ++failures;
    }
}

static void published_vectors(Reference &ref)
{
    // These are VMAC-64 tags, not VHASH tags. Subtract the independently
    // derived AES nonce pad modulo 2^64 to recover the seed-zero VHASH value.
    const size_t repeats[] = {0, 1, 16, 100};
    const uint64_t tags[] = {UINT64_C(0x2576be1c56d8b81b),
        UINT64_C(0x2d376cf5b1813ce5), UINT64_C(0xe8421f61d573d298),
        UINT64_C(0x4492df6c5cac1bbe)};
    uint64_t pad = ref.published_pad();
    for (unsigned i = 0; i < sizeof(repeats) / sizeof(repeats[0]); ++i) {
        std::vector<unsigned char> msg(repeats[i] * 3);
        for (size_t j = 0; j < msg.size(); ++j) msg[j] = "abc"[j % 3];
        const unsigned char *p = msg.empty() ? NULL : &msg[0];
        uint64_t expected = tags[i] - pad;
        unsigned char out[8];
        VHASH_64(p, (int)msg.size(), 0, out);
        check(ref.hash(p, msg.size(), 0) == expected, "published/reference", msg.size(), 0, 0);
        check(load_le(out) == expected, "published/adapter", msg.size(), 0, 0);
    }
}

int main()
{
    VHASH_init();
    Reference ref;
    published_vectors(ref);
    const size_t lengths[] = {0,1,2,7,8,9,15,16,17,31,32,33,63,64,65,
                              111,112,113,127,128,129,143,144,145,255,256,
                              257,383,384,385,511,512,513,1023,1024,1025};
    const uint32_t seeds[] = {0, 1, UINT32_C(0x12345678), UINT32_MAX};
    for (unsigned l = 0; l < sizeof(lengths) / sizeof(lengths[0]); ++l) {
        size_t len = lengths[l];
        std::vector<unsigned char> msg(len);
        for (size_t i = 0; i < len; ++i)
            msg[i] = (unsigned char)((i * 131 + (i >> 3) + 19) & 255);
        const unsigned char *msgp = len ? &msg[0] : NULL;
        const uint64_t unseeded = ref.hash(msgp, len, 0);
        for (unsigned s = 0; s < sizeof(seeds) / sizeof(seeds[0]); ++s) {
            uint32_t seed = seeds[s];
            uint64_t expected = ref.hash(msgp, len, seed);
            if (seed)
                check(expected != unseeded, "seed influence", len, seed, 0);
            for (unsigned align = 0; align < 16; ++align) {
                // Dirty suffix detects reading bytes beyond the logical length.
                std::vector<unsigned char> storage(len + 32, 0xa5);
                unsigned char *p = &storage[align];
                if (len) memcpy(p, msgp, len);
                unsigned char out32[12], out64[16];
                memset(out32, 0x5a, sizeof(out32));
                memset(out64, 0x5a, sizeof(out64));
                VHASH_32(p, (int)len, seed, out32 + 3);
                VHASH_64(p, (int)len, seed, out64 + 3);
                check(load_le(out64 + 3) == expected, "reference/64", len, seed, align);
                uint32_t actual32 = (uint32_t)out32[3] | ((uint32_t)out32[4] << 8) |
                                    ((uint32_t)out32[5] << 16) | ((uint32_t)out32[6] << 24);
                check(actual32 == (uint32_t)expected, "reference/32", len, seed, align);
                for (unsigned j = 0; j < sizeof(out32); ++j)
                    if (j < 3 || j >= 7)
                        check(out32[j] == 0x5a, "canary/32", len, seed, align);
                for (unsigned j = 0; j < sizeof(out64); ++j)
                    if (j < 3 || j >= 11)
                        check(out64[j] == 0x5a, "canary/64", len, seed, align);
                memset(p + len, 0x3c, 16);
                unsigned char again[8];
                VHASH_64(p, (int)len, seed, again);
                check(!memcmp(again, out64 + 3, 8), "suffix/repeat", len, seed, align);
            }
        }
    }
    if (failures) fprintf(stderr, "%d VHASH regression failures\n", failures);
    else puts("VHASH portability regression passed");
    return failures ? 1 : 0;
}
