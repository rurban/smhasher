#include "CompressibilityTest.h"
#include <algorithm>
#include <stdio.h>
#include <string.h>
#include <vector>
#include <zlib.h>

bool CompressibilityTest(pfHash hash, int hashbits)
{
  const size_t stream_size = 1024 * 1024;
  const size_t output_size = (size_t)hashbits / 8;
  const size_t state_size = std::max(output_size, size_t(8));
  alignas(16) uint8_t state[64] = {};
  alignas(16) uint8_t input[80] = {};
  alignas(16) uint8_t output[64] = {};
  std::vector<uint8_t> stream(stream_size);

  for (size_t offset = 0; offset < stream_size;) {
    if (output_size < 8) {
      memcpy(input, state, state_size);
      input[state_size] = 0;
      hash(input, (int)state_size + 1, 0, output);
      memcpy(state, output, output_size);
      input[state_size] = 1;
      hash(input, (int)state_size + 1, 0, output);
      memcpy(state + output_size, output, output_size);
    } else {
      memcpy(input, state, state_size);
      hash(input, (int)state_size, 0, output);
      memcpy(state, output, state_size);
    }
    size_t count = std::min(state_size, stream_size - offset);
    memcpy(stream.data() + offset, state, count);
    offset += count;
  }

  uLongf compressed_size = compressBound(stream_size);
  std::vector<uint8_t> compressed(compressed_size);
  int status = compress2(compressed.data(), &compressed_size, stream.data(),
                         stream_size, Z_BEST_COMPRESSION);
  if (status != Z_OK) {
    printf("Compression failed with zlib error %d\n", status);
    return false;
  }
  printf("Compressed %zu bytes to %lu bytes (%.2f%%)\n", stream_size,
         (unsigned long)compressed_size, 100.0 * compressed_size / stream_size);
  return compressed_size >= stream_size * 995 / 1000;
}
