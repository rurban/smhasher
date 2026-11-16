#include <stdint.h>
#include <string.h>

uint32_t SuperFastHash(const char *, int, uint32_t);

int main()
{
  const uint8_t bytes[] = {0x80,0xff,0x00,0x7f,0xa5,0x33,0x42,0xe1,
                           0x01,0x80,0xfe,0x7f,0x00,0xff,0x05,0x44};
  const uint32_t expected[] = {
    0x00000000,0xcb6a7c6e,0x31970cd0,0xeb79d1c4,0xc629d6a1,0xddfcbc76,
    0x89f5fb47,0xf2e971bb,0x3033f5fc,0xc8e50c66,0x25a8cd31,0x38ff2940,
    0x75752a2a,0xd4919488,0x93a5794a,0x568fe325,0xc444f192
  };
  alignas(16) uint8_t unaligned[sizeof(bytes) + 1];
  memcpy(unaligned + 1, bytes, sizeof(bytes));
  for (int len = 0; len <= (int)sizeof(bytes); ++len)
    if (SuperFastHash((const char *)unaligned + 1, len, 0x12345678U) != expected[len]) return 1;
  return SuperFastHash(NULL, 0, 0x12345678U) == 0 ? 0 : 1;
}
