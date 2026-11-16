#include <stdint.h>
#include <stddef.h>

void FNV128(uint64_t out[2], const char *key, int len, uint64_t seed);

int main()
{
  uint8_t key[255];
  for (size_t i = 0; i < sizeof(key); ++i) key[i] = (uint8_t)i;
  const uint32_t seeds[] = {0, 1, 0xffffffffU};
  const int sizes[] = {0, 1, 3, 16, 255};
  const uint64_t expected[][5][2] = {
    {{0x6c62272e07bb0142ULL,0x62b821756295c58dULL},{0xd228cb69101a8cafULL,0x78912b704e4a147fULL},{0xa68bb3934b8b5822ULL,0x836dbc78c7423baeULL},{0x5b6a0f8f6515372cULL,0xdea9313bc0fa66cdULL},{0xfafca378fa74a5e2ULL,0xf8709b67a8441d36ULL}},
    {{0x6c62272e07bb0143ULL,0x62b821756295c58dULL},{0xd228cb69101a8deaULL,0x78912b704e4a147fULL},{0xa68bb3934d684565ULL,0x836dbc78c7423baeULL},{0x1f16201babb022edULL,0xdea9313bc0fa66cdULL},{0x0631862429c2c9d5ULL,0xf8709b67a8441d36ULL}},
    {{0x6c62272ef844febdULL,0x62b821756295c58dULL},{0xd228cc9109e57308ULL,0x78912b704e4a147fULL},{0xa84bd31550078e53ULL,0x836dbc78c7423baeULL},{0xc8b3aaa986cb39e7ULL,0xdea9313bc0fa66cdULL},{0x6b5e4bfdfb9212a3ULL,0xf8709b67a8441d36ULL}}
  };
  for (size_t s = 0; s < 3; ++s) {
    for (size_t i = 0; i < 5; ++i) {
      uint64_t out[2];
      FNV128(out, (const char *)key, sizes[i], seeds[s]);
      if (out[0] != expected[s][i][0] || out[1] != expected[s][i][1]) return 1;
    }
  }
  return 0;
}
