/*
 * CLHash is a very fast hashing function that uses the
 * carry-less multiplication and SSE instructions.
 *
 * Daniel Lemire, Owen Kaser, Faster 64-bit universal hashing
 * using carry-less multiplications, Journal of Cryptographic Engineering (to appear)
 *
 * Best used on recent x64 processors (Haswell or better).
 *
 * Compile option: if you define BITMIX during compilation, extra work is done to
 * pass smhasher's avalanche test succesfully. Disabled by default.
 **/

#ifndef INCLUDE_CLHASH_H_
#define INCLUDE_CLHASH_H_


#include <stdlib.h>
#include <stdint.h> // life is short, please use a C99-compliant compiler
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// BITMIX necessary to pass the Avalanche Tests
#define BITMIX
  
enum {RANDOM_64BITWORDS_NEEDED_FOR_CLHASH=133,RANDOM_BYTES_NEEDED_FOR_CLHASH=133*8};

/**
 *  random : the random data source (should contain at least
 *  RANDOM_BYTES_NEEDED_FOR_CLHASH random bytes), it should
 *  also be aligned on 16-byte boundaries so that (((uintptr_t) random & 15) == 0)
 *  for performance reasons. This is usually generated once and reused with many
 *  inputs.
 *  It should avoid 0 in its first 64 bits. (bad seed)
 *
 * stringbyte : the input data source, could be anything you want to has
 *
 *
 * length : number of bytes in the string
 */
uint64_t clhash(const void* random, const char * stringbyte,
                const size_t lengthbyte);



/**
 * Convenience method. Will generate a random key from two 64-bit seeds.
 * Caller is responsible to call "free" on the result.
 */
void * get_random_key_for_clhash(uint64_t seed1, uint64_t seed2);
void free_random_key_for_clhash(void *);

#ifdef __cplusplus
} // extern "C"
#endif

#ifdef __cplusplus
#include <vector>
#include <string>
#include <cstring> // For std::strlen

struct clhasher {
    const void *random_data_;
    clhasher(uint64_t seed1=137, uint64_t seed2=777): random_data_(get_random_key_for_clhash(seed1, seed2)) {}
    template<typename T>
    uint64_t operator()(const T *data, const size_t len) const {
        return clhash(random_data_, (const char *)data, len * sizeof(T));
    }
    uint64_t operator()(const char *str) const {return operator()(str, std::strlen(str));}
    template<typename T>
    uint64_t operator()(const T &input) const {
        return operator()((const char *)&input, sizeof(T));
    }
    template<typename T>
    uint64_t operator()(const std::vector<T> &input) const {
        return operator()((const char *)input.data(), sizeof(T) * input.size());
    }
    uint64_t operator()(const std::string &str) const {
        return operator()(str.data(), str.size());
    }
    ~clhasher() {
        free((void *)random_data_);
    }
};
#endif // #ifdef __cplusplus

#endif /* INCLUDE_CLHASH_H_ */
