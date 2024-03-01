/*
* Quick file to test the bindings to C without building the whole project
* Run with `cargo build && gcc test-bindings.c target/debug/librust_hashes.a && ./a.out`
* Make sure to generate hashes if bindings have changes
* (`RUST_GENERATE_HASHES=1 cargo +nightly build`)
*/

#include <stdio.h>
#include <string.h>
#include "rust_hashes.h"

int print_clear(char* name, char* out, int bits) {
  printf("%s: ", name);
  
  for(int i=0; i < (bits >> 3); ++i) {
    printf("%02x", out[i] & 0xff);
    out[i] = 0;
  }

  printf("\n");
}
  
int main() {
  char *x = "Hello, world!";
  char out[1000] = {0};

  printf("hashing '%s' (len %ld)\n", x, strlen(x));

  ahash_rstest(x, strlen(x), 0, out);
  print_clear("ahash", out, 64);
  
  sha256_rstest(x, strlen(x), 0, &out);
  print_clear("sha256", out, 256);
  
  blake2s256_rstest(x, strlen(x), 0, &out);
  print_clear("blake2s256", out, 256);
  
  blake3_rstest(x, strlen(x), 0, &out);
  print_clear("blake3", out, 256);

  keccak256full_rstest(x, strlen(x), 0, &out);
  print_clear("keccak256_full", out, 1600);
}
