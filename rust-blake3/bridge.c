#include <stddef.h>
#include <stdint.h>

extern void smhasher_rust_blake3_digest(const uint8_t *, size_t, uint8_t *);

#if defined(_WIN32)
#define BLAKE3_BRIDGE_EXPORT __declspec(dllexport)
#elif defined(__GNUC__) || defined(__clang__)
#define BLAKE3_BRIDGE_EXPORT __attribute__((visibility("default")))
#else
#define BLAKE3_BRIDGE_EXPORT
#endif

BLAKE3_BRIDGE_EXPORT void smhasher_blake3_rust(const void *data, size_t len, void *out)
{
  smhasher_rust_blake3_digest((const uint8_t *)data, len, (uint8_t *)out);
}
