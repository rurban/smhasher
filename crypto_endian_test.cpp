#include <stdint.h>
#include <string.h>

#include "sha1.h"
extern "C" {
#include "blake3/blake3.h"
int crypto_hash(unsigned char *, const unsigned char *, unsigned long long);
}

static bool matches_hex(const uint8_t *bytes, size_t n, const char *hex) {
  const char digits[] = "0123456789abcdef";
  for (size_t i = 0; i < n; ++i)
    if (digits[bytes[i] >> 4] != hex[2 * i] ||
        digits[bytes[i] & 15] != hex[2 * i + 1]) return false;
  return true;
}

int main() {
  uint8_t input[3074];
  for (size_t i = 0; i < sizeof(input) - 1; ++i)
    input[i + 1] = (uint8_t)(i % 251);
  uint8_t digest[65];

  SHA1_CTX sha;
  SHA1_Init(&sha);
  const char *message = "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq";
  SHA1_Update(&sha, (const uint8_t *)message, 13);
  SHA1_Update(&sha, (const uint8_t *)message + 13, strlen(message) - 13);
  SHA1_Final(&sha, 20, digest);
  if (!matches_hex(digest, 20, "84983e441c3bd26ebaae4aa1f95129e5e54670f1")) return 1;
  SHA1_Init(&sha);
  sha.state[0] ^= UINT32_C(0x12345678);
  SHA1_Update(&sha, input + 1, 17);
  SHA1_Final(&sha, 20, digest);
  if (!matches_hex(digest, 20, "aacb8c8c4f81b5cd09a531e744798e18b3abc444")) return 6;

  // Ascon v1.2 KATs, with an unaligned input and output, spanning the rate.
  static const char *ascon[] = {
    "7346bc14f036e87ae03d0997913088f5f68411434b3cf8b54fa796a80d251f91",
    "8dd446ada58a7740ecf56eb638ef775f7d5c0fd5f0c2bbbdfdec29609d3c43a2",
    "f4c6a44b29915d3d57cf928a18ec6226bb8dd6c1136acd24965f7e7780cd69cf",
    "1e1e710d08a78263773331782621088ca9fe2ee4f596f06c8f7884ca564acec1"
  };
  const unsigned ascon_lengths[] = {0, 1, 8, 9};
  for (unsigned i = 0; i < 4; ++i) {
    memset(digest, 0xa5, sizeof(digest));
    crypto_hash(digest + 1, input + 1, ascon_lengths[i]);
    if (digest[0] != 0xa5 || digest[33] != 0xa5 ||
        !matches_hex(digest + 1, 32, ascon[i])) return 2;
  }

  // BLAKE3 official test vectors use input[i] = i % 251.
  static const struct { size_t len; const char *hash; } blake[] = {
    {0, "af1349b9f5f9a1a6a0404dea36dcc9499bcb25c9adc112b7cc9a93cae41f3262"},
    {1024, "42214739f095a406f3fc83deb889744ac00df831c10daa55189b5d121c855af7"},
    {1025, "d00278ae47eb27b34faecf67b4fe263f82d5412916c1ffd97c8cb7fb814b8444"},
    {2049, "5f4d72f40d7a5f82b15ca2b2e44b1de3c2ef86c426c95c1af0b6879522563030"},
    {3072, "b98cb0ff3623be03326b373de6b9095218513e64f1ee2edd2525c7ad1e5cffd2"}
  };
  for (unsigned i = 0; i < sizeof(blake) / sizeof(blake[0]); ++i) {
    blake3_hasher hasher;
    blake3_hasher_init(&hasher);
    blake3_hasher_update(&hasher, input + 1, blake[i].len);
    blake3_hasher_finalize(&hasher, digest, 32);
    if (!matches_hex(digest, 32, blake[i].hash)) return 4;
    // A first buffered chunk exercises output_chaining_value independently.
    blake3_hasher_init(&hasher);
    size_t first = blake[i].len < 1000 ? blake[i].len : 1000;
    blake3_hasher_update(&hasher, input + 1, first);
    blake3_hasher_update(&hasher, input + 1 + first, blake[i].len - first);
    blake3_hasher_finalize(&hasher, digest, 32);
    if (!matches_hex(digest, 32, blake[i].hash)) return 5;
  }
  return 0;
}
