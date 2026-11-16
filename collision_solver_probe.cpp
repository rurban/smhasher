// Reuse the actual registry and initialization without duplicating hash lists.
#define main smhasher_program_main
#include "main.cpp"
#undef main

#if defined(__GNUC__) || defined(__clang__)
#define SOLVER_NOINLINE __attribute__((noinline, used))
#elif defined(_MSC_VER)
#define SOLVER_NOINLINE __declspec(noinline)
#else
#define SOLVER_NOINLINE
#endif

extern "C" SOLVER_NOINLINE void collision_solver_marker(pfHash hash, const uint8_t *key, uint8_t *out) {
  hash(key, 64, 0, out);
}

int main(int argc, char **argv) {
  if (argc == 2 && strcmp(argv[1], "--listnames") == 0) {
    for (size_t i = 0; i < sizeof(g_hashes) / sizeof(g_hashes[0]); ++i)
      printf("%s\n", g_hashes[i].name);
    return 0;
  }
  if (argc != 3 || strlen(argv[2]) != 8) return 2;
  HashInfo *info = findHash(argv[1]);
  if (!info) { fprintf(stderr, "Unknown hash: %s\n", argv[1]); return 2; }
  alignas(16) uint8_t key[64] = {}, output[64] = {};
  for (unsigned i = 0; i < 4; ++i) {
    char word[3] = {argv[2][2*i], argv[2][2*i+1], 0};
    char *end;
    unsigned long value = strtoul(word, &end, 16);
    if (*end || end != word + 2 || word[0] == '-' || word[0] == '+') return 2;
    key[i] = (uint8_t)value;
  }
  Hash_init(info);
  Seed_init(info, 0);
  collision_solver_marker(info->hash, key, output);
  printf("SOLVER_HASH=");
  for (int i = 0; i < info->hashbits / 8 && i < 64; ++i) printf("%02x", output[i]);
  printf("\n");
  return 0;
}
