// Native oracle for collision_prover.py: hashes arbitrary keys with the real,
// compiled hash functions of the SMHasher registry, so that every finding of the
// prover is checked against the C implementation, not against a Python model.
//
//   CollisionProverVerify --listnames
//   CollisionProverVerify <hash> <seed> <keyhex|->      one hash, prints hex digest
//   CollisionProverVerify --serve                       line protocol on stdin/stdout:
//        H <hash> <seed> <keyhex|->        -> <hex digest>
//        R <hash> <len> <seconds>          -> <hashes per second> (throughput, 1 thread)
//   The digest is printed as the bytes the hash wrote into its output buffer,
//   in memory order (info->hashbits / 8 bytes).
//
// Reuse the actual registry and initialization without duplicating hash lists.
#define main smhasher_program_main
#include "main.cpp"
#undef main

#include <chrono>
#include <ctype.h>
#include <errno.h>
#include <string>

static bool parse_hex(const char *s, std::vector<uint8_t> &out) {
  out.clear();
  if (!strcmp(s, "-")) return true;
  size_t n = strlen(s);
  if (n % 2) return false;
  for (size_t i = 0; i < n; i += 2) {
    char w[3] = {s[i], s[i + 1], 0};
    char *end;
    if (!isxdigit((unsigned char)w[0]) || !isxdigit((unsigned char)w[1])) return false;
    out.push_back((uint8_t)strtoul(w, &end, 16));
  }
  return true;
}

static bool parse_u32(const char *s, uint32_t &seed) {
  char *end;
  errno = 0;
  unsigned long long v = strtoull(s, &end, 0);
  if (errno || end == s || *end || v > 0xffffffffULL) return false;
  seed = (uint32_t)v;
  return true;
}

static std::vector<HashInfo *> g_initialized;

static HashInfo *lookup(const char *name) {
  HashInfo *info = findHash(name);
  if (!info) return NULL;
  bool done = false;
  for (size_t i = 0; i < g_initialized.size(); ++i) done |= g_initialized[i] == info;
  if (!done) { Hash_init(info); g_initialized.push_back(info); }
  return info;
}

static int hash_one(const char *name, uint32_t seed, const std::vector<uint8_t> &key, std::string &digest) {
  HashInfo *info = lookup(name);
  if (!info) return 2;
  // Seed_init may reseed global state; the hash itself gets the seed as well.
  Seed_init(info, seed);
  // padded, aligned copy: some hashes read whole words past the end
  std::vector<uint8_t> buf(key.size() + 128 + 64, 0);
  uint8_t *p = buf.data();
  p += (16 - ((uintptr_t)p & 15)) & 15;
  if (!key.empty()) memcpy(p, key.data(), key.size());
  alignas(16) uint8_t out[64 + 64] = {};
  info->hash(p, (int)key.size(), seed, out);
  static const char hexd[] = "0123456789abcdef";
  digest.clear();
  for (int i = 0; i < info->hashbits / 8 && i < 64; ++i) {
    digest += hexd[out[i] >> 4];
    digest += hexd[out[i] & 15];
  }
  return 0;
}

static double rate(const char *name, int len, double seconds) {
  HashInfo *info = lookup(name);
  if (!info || len < 0 || len > 4096) return -1;
  Seed_init(info, 0);
  std::vector<uint8_t> buf(len + 128 + 64, 0);
  uint8_t *p = buf.data();
  p += (16 - ((uintptr_t)p & 15)) & 15;
  alignas(16) uint8_t out[64 + 64] = {};
  typedef std::chrono::steady_clock Clock;
  const Clock::time_point start = Clock::now();
  uint64_t n = 0, sink = 0;
  double elapsed = 0;
  do {
    for (int i = 0; i < 4096; ++i, ++n) {
      for (int j = 0; j < len && j < 8; ++j) p[j] = (uint8_t)(n >> (8 * j));
      info->hash(p, len, 0, out);
      sink += out[0];
    }
    elapsed = std::chrono::duration<double>(Clock::now() - start).count();
  } while (elapsed < seconds);
  if (sink == 0x123456789abcdefULL) fputs("", stderr);  // keep the loop
  return n / elapsed;
}

int main(int argc, char **argv) {
  if (argc == 2 && !strcmp(argv[1], "--listnames")) {
    for (size_t i = 0; i < sizeof(g_hashes) / sizeof(g_hashes[0]); ++i) {
      HashInfo &h = g_hashes[i];
      printf("%s %d\n", h.name, h.hashbits);
    }
    return 0;
  }
  if (argc == 2 && !strcmp(argv[1], "--serve")) {
    char line[1 << 16];
    while (fgets(line, sizeof line, stdin)) {
      char cmd[8], name[128], a[64], b[1 << 15];
      int n = sscanf(line, "%7s %127s %63s %32767s", cmd, name, a, b);
      if (n == 4 && !strcmp(cmd, "H")) {
        uint32_t seed;
        std::vector<uint8_t> key;
        std::string digest;
        if (!parse_u32(a, seed) || !parse_hex(b, key) || hash_one(name, seed, key, digest)) puts("ERR");
        else puts(digest.c_str());
      } else if (n == 4 && !strcmp(cmd, "R")) {
        double r = rate(name, atoi(a), atof(b));
        if (r < 0) puts("ERR"); else printf("%.0f\n", r);
      } else puts("ERR");
      fflush(stdout);
    }
    return 0;
  }
  if (argc != 4) return 2;
  uint32_t seed;
  std::vector<uint8_t> key;
  std::string digest;
  if (!parse_u32(argv[2], seed) || !parse_hex(argv[3], key)) return 2;
  if (hash_one(argv[1], seed, key, digest)) { fprintf(stderr, "Unknown hash: %s\n", argv[1]); return 2; }
  puts(digest.c_str());
  return 0;
}
