
#include <stdint.h>


/* mask types */
#if defined(__i386__) || defined(__X86_64__)
typedef unsigned long long v2uq __attribute__ ((vector_size (16)));
uint64_t gf2_matrix_times_vec2(uint64_t *mat, uint64_t vec);
#else
uint64_t gf2_matrix_times_switch(uint64_t *mat, uint64_t vec);
#endif
void init_combine_cache(uint64_t poly, uint8_t dim);
uint64_t crc64_combine(uint64_t crc1, uint64_t crc2, uintmax_t len2, uint64_t poly, uint8_t dim);
