// Tachyon
// Copyright (c) byt3forg3
// Licensed under the MIT or Apache 2.0 License
// -------------------------------------------------------------------------

#ifndef TACHYON_H
#define TACHYON_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// DOMAIN CONSTANTS
// =============================================================================

#define TACHYON_DOMAIN_GENERIC           0
#define TACHYON_DOMAIN_FILE_CHECKSUM     1
#define TACHYON_DOMAIN_KEY_DERIVATION    2
#define TACHYON_DOMAIN_MESSAGE_AUTH      3
#define TACHYON_DOMAIN_DATABASE_INDEX    4
#define TACHYON_DOMAIN_CONTENT_ADDRESSED 5

#define TACHYON_SUCCESS       0
#define TACHYON_ERROR_NULL_PTR (-1)  /* null required pointer */
#define TACHYON_ERROR_STATE    (-2)  /* panic / unsupported CPU */

// =============================================================================
// TYPES
// =============================================================================

typedef struct tachyon_state_t tachyon_state_t;

// =============================================================================
// ONE-SHOT API
// =============================================================================

int tachyon_hash(const uint8_t *input, size_t len, uint8_t *out /*[32]*/);
int tachyon_hash_seeded(const uint8_t *input, size_t len, uint64_t seed, uint8_t *out /*[32]*/);
int tachyon_hash_full(const uint8_t *input, size_t len, uint64_t domain, uint64_t seed,
                      const uint8_t *key /*[32] or NULL*/, uint8_t *out /*[32]*/);
int tachyon_hash_with_domain(const uint8_t *input, size_t len, uint64_t domain, uint8_t *out /*[32]*/);
int tachyon_verify(const uint8_t *input, size_t len, const uint8_t *hash /*[32]*/);
int tachyon_hash_keyed(const uint8_t *input, size_t len, const uint8_t *key /*[32]*/, uint8_t *out /*[32]*/);
int tachyon_verify_mac(const uint8_t *input, size_t len, const uint8_t *key /*[32]*/, const uint8_t *mac /*[32]*/);
int tachyon_derive_key(const char *context, size_t context_len,
                       const uint8_t *material /*[32]*/, uint8_t *out /*[32]*/);
/* Returns a static, null-terminated string — do NOT free. */
const char* tachyon_get_backend_name(void);

// =============================================================================
// STREAMING API
// =============================================================================

/* Returns NULL on unsupported CPU. Caller must free with tachyon_hasher_free (or tachyon_hasher_finalize). */
tachyon_state_t* tachyon_hasher_new(void);
tachyon_state_t* tachyon_hasher_new_with_domain(uint64_t domain);
tachyon_state_t* tachyon_hasher_new_seeded(uint64_t seed);
tachyon_state_t* tachyon_hasher_new_full(uint64_t domain, uint64_t seed, const uint8_t *key /*[32] or NULL*/);
void tachyon_hasher_update(tachyon_state_t *state, const uint8_t *data, size_t len);
void tachyon_hasher_finalize(tachyon_state_t *state, uint8_t *out /*[32]*/); /* frees state */
void tachyon_hasher_free(tachyon_state_t *state);                             /* frees without output */

#ifdef __cplusplus
}
#endif

#endif 
