#include "hashtron.h"

int main()
{
  const uint8_t ascii[] = {'H','e','l','l','o'};
  const uint8_t unicode[] = {0xc3,0xa9,0xe4,0xb8,0xad,0xf0,0x9f,0x98,0x80};
  const uint8_t invalid[] = {0xff,0xc0,0x80,0xed,0xa0,0x80,0xf4,0x90,0x80,0x80};
  const uint32_t seeds[] = {0, 1, UINT32_MAX};
  const uint32_t expected[][3] = {
    {0x784f0deeU, 0xe62f54deU, 0x5bbf0147U},
    {0x719094adU, 0x59090206U, 0xe5eb0153U},
    {0xae509a73U, 0xf9cab1b8U, 0xb18c7c06U}
  };
  for (int i = 0; i < 3; ++i) {
    if (hashtron_string(NULL, 0, seeds[i]) != seeds[i] ||
        hashtron_string(ascii, sizeof(ascii), seeds[i]) != expected[i][0] ||
        hashtron_string(unicode, sizeof(unicode), seeds[i]) != expected[i][1] ||
        hashtron_string(invalid, sizeof(invalid), seeds[i]) != expected[i][2])
      return 1;
  }
  return 0;
}
