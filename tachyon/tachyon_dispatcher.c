// Tachyon
// Copyright (c) byt3forg3
// Licensed under the MIT or Apache 2.0 License
// -------------------------------------------------------------------------

#include "tachyon.h"
#include "tachyon_impl.h"
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#if defined(__x86_64__) || defined(__i386__) || defined(_M_X64) || defined(_M_IX86)
#if defined(_MSC_VER)
#include <intrin.h>
#else
#include <cpuid.h>
#endif
#endif

// =============================================================================
// CONSTANTS
// =============================================================================

#define CHUNK_SIZE (256 * 1024)
#define MAX_TREE_LEVELS 64

#define XCR0_SSE_AVX_MASK 0x06
#define XCR0_AVX512_MASK  0xE0

// CPUID feature bit positions
#define CPUID_AES_BIT       25
#define CPUID_AVX512F_BIT   16
#define CPUID_AVX512BW_BIT  30
#define CPUID_VAES_BIT       9
#define CPUID_VPCLMUL_BIT   10

typedef enum {
    CPU_UNKNOWN = 0,
    CPU_PORTABLE = 1,
    CPU_AESNI = 2,
    CPU_AVX512 = 3
} cpu_feature_t;

// =============================================================================
// CPU FEATURE DETECTION
// =============================================================================

static cpu_feature_t g_cpu_feature = CPU_UNKNOWN;

static void detect_cpu() {
#ifdef FORCE_PORTABLE
    g_cpu_feature = CPU_PORTABLE;
#elif defined(FORCE_AESNI) && (defined(__x86_64__) || defined(__i386__) || defined(_M_X64) || defined(_M_IX86))
    g_cpu_feature = CPU_AESNI;
#elif defined(__x86_64__) || defined(__i386__) || defined(_M_X64) || defined(_M_IX86)
    unsigned int eax = 0, ebx = 0, ecx = 0, edx = 0;

#if defined(_MSC_VER)
    int cpuInfo[4];
    __cpuid(cpuInfo, 1);
    eax = cpuInfo[0]; ebx = cpuInfo[1]; ecx = cpuInfo[2]; edx = cpuInfo[3];
    int cpuid_ok = 1;
#else
    int cpuid_ok = __get_cpuid(1, &eax, &ebx, &ecx, &edx);
#endif

    if (!cpuid_ok || !(ecx & (1 << CPUID_AES_BIT))) {
        g_cpu_feature = CPU_PORTABLE;
    } else {
        g_cpu_feature = CPU_AESNI;

#if defined(_MSC_VER)
        __cpuidex(cpuInfo, 7, 0);
        eax = cpuInfo[0]; ebx = cpuInfo[1]; ecx = cpuInfo[2]; edx = cpuInfo[3];
        int cpuid7_ok = 1;
#else
        int cpuid7_ok = __get_cpuid_count(7, 0, &eax, &ebx, &ecx, &edx);
#endif

        if (cpuid7_ok) {
            int has_avx512_cpuid =
                (ebx & (1 << CPUID_AVX512F_BIT))  &&  /* AVX-512 Foundation  */
                (ebx & (1 << CPUID_AVX512BW_BIT)) &&  /* AVX-512 Byte/Word   */
                (ecx & (1 << CPUID_VAES_BIT))     &&  /* Vector AES          */
                (ecx & (1 << CPUID_VPCLMUL_BIT));     /* Vector CLMUL        */

            if (has_avx512_cpuid) {
                /* Verify OS support: check XCR0 for ZMM state saving.
                 * Prevents crashes in VMs that mask AVX-512 at the hypervisor level.
                 * Uses inline asm to avoid requiring -mxsave compiler flag. */
                uint32_t xcr0_lo, xcr0_hi;
#if defined(_MSC_VER)
                uint64_t xcr0 = _xgetbv(0);
                xcr0_lo = (uint32_t)xcr0;
                xcr0_hi = (uint32_t)(xcr0 >> 32);
#else
                __asm__ volatile ("xgetbv" : "=a"(xcr0_lo), "=d"(xcr0_hi) : "c"(0));
#endif

                if ((xcr0_lo & XCR0_SSE_AVX_MASK) == XCR0_SSE_AVX_MASK &&
                    (xcr0_lo & XCR0_AVX512_MASK)  == XCR0_AVX512_MASK) {
                    g_cpu_feature = CPU_AVX512;
                }
            }
        }
    }
#else
    g_cpu_feature = CPU_PORTABLE;
#endif
}

const char* tachyon_get_backend_name(void) {
    if (g_cpu_feature == CPU_UNKNOWN) detect_cpu();
    switch (g_cpu_feature) {
#if !defined(FORCE_PORTABLE) && (defined(__x86_64__) || defined(__i386__) || defined(_M_X64) || defined(_M_IX86))
        case CPU_AVX512: return "AVX-512 (Truck)";
        case CPU_AESNI:  return "AES-NI (Scooter)";
#endif
        default:         return "Portable";
    }
}

typedef void (*tachyon_oneshot_fn)(const uint8_t *input, size_t len, uint64_t domain, uint64_t seed, const uint8_t *key, uint8_t *out);

static tachyon_oneshot_fn get_kernel() {
    if (g_cpu_feature == CPU_UNKNOWN) detect_cpu();
#if !defined(FORCE_PORTABLE) && (defined(__x86_64__) || defined(__i386__) || defined(_M_X64) || defined(_M_IX86))
    if (g_cpu_feature == CPU_AVX512) return tachyon_avx512_oneshot;
    if (g_cpu_feature == CPU_AESNI)  return tachyon_aesni_oneshot;
#endif
    return tachyon_portable_oneshot;
}

static void compute_kernel(const uint8_t *data, size_t len, uint64_t domain,
                           uint64_t seed, const uint8_t *key, uint8_t *out) {
    get_kernel()(data, len, domain, seed, key, out);
}

// =============================================================================
// MERKLE TREE ENGINE
// =============================================================================

typedef struct {
    uint8_t buffer[CHUNK_SIZE];
    size_t buffer_len;
    uint64_t total_len;
    uint64_t domain;
    uint64_t seed;
    uint8_t key[HASH_SIZE];
    int has_key;
    uint8_t stack[MAX_TREE_LEVELS][HASH_SIZE];
    uint64_t stack_usage;
} tachyon_internal_state_t;

/* Merkle tree stack push using bitfield-based sparse representation. */
static void stack_push(tachyon_internal_state_t *s, const uint8_t *hash) {
    uint8_t current_hash[HASH_SIZE];
    memcpy(current_hash, hash, HASH_SIZE);

    for (int level = 0; level < MAX_TREE_LEVELS; level++) {
        if (s->stack_usage & (1ULL << level)) {
            uint8_t buffer[HASH_SIZE * 2];
            memcpy(buffer,             &s->stack[level], HASH_SIZE);
            memcpy(buffer + HASH_SIZE, current_hash,     HASH_SIZE);
            compute_kernel(buffer, HASH_SIZE * 2, DOMAIN_NODE, s->seed,
                           s->has_key ? s->key : NULL, current_hash);
            s->stack_usage &= ~(1ULL << level);
        } else {
            memcpy(&s->stack[level], current_hash, HASH_SIZE);
            s->stack_usage |= (1ULL << level);
            return;
        }
    }
}

static tachyon_internal_state_t* new_state(uint64_t domain, uint64_t seed, const uint8_t *key) {
    tachyon_internal_state_t *s = (tachyon_internal_state_t*)malloc(sizeof(tachyon_internal_state_t));
    if (!s) return NULL;
    memset(s, 0, sizeof(*s));
    s->domain = domain;
    s->seed   = seed;
    if (key) {
        memcpy(s->key, key, HASH_SIZE);
        s->has_key = 1;
    }
    return s;
}

static void update_state(tachyon_internal_state_t *state, const uint8_t *data, size_t len) {
    size_t processed = 0;

    while (processed < len) {
        size_t copy_len = CHUNK_SIZE - state->buffer_len;
        if (len - processed < copy_len) {
            copy_len = len - processed;
        }

        memcpy(state->buffer + state->buffer_len, data + processed, copy_len);
        state->buffer_len += copy_len;
        state->total_len  += copy_len;
        processed         += copy_len;

        if (state->buffer_len == CHUNK_SIZE) {
            uint8_t chunk_hash[HASH_SIZE];
            compute_kernel(state->buffer, CHUNK_SIZE, DOMAIN_LEAF, state->seed,
                           state->has_key ? state->key : NULL, chunk_hash);
            stack_push(state, chunk_hash);
            state->buffer_len = 0;
        }
    }
}

/* Fast path: inputs < CHUNK_SIZE bypass tree. Tree path: collapse stack and commit length. */
static void finalize_state(tachyon_internal_state_t *state, uint8_t *out) {
    if (state->stack_usage == 0 && state->buffer_len < CHUNK_SIZE) {
        compute_kernel(state->buffer, state->buffer_len, state->domain, state->seed,
                       state->has_key ? state->key : NULL, out);
        free(state);
        return;
    }

    if (state->buffer_len > 0) {
        uint8_t chunk_hash[HASH_SIZE];
        compute_kernel(state->buffer, state->buffer_len, DOMAIN_LEAF, state->seed,
                       state->has_key ? state->key : NULL, chunk_hash);
        stack_push(state, chunk_hash);
    }

    uint8_t root[HASH_SIZE];
    int first = 1;

    for (int i = 0; i < MAX_TREE_LEVELS; i++) {
        if (state->stack_usage & (1ULL << i)) {
            if (first) {
                memcpy(root, &state->stack[i], HASH_SIZE);
                first = 0;
            } else {
                uint8_t buffer[HASH_SIZE * 2];
                memcpy(buffer,             &state->stack[i], HASH_SIZE);
                memcpy(buffer + HASH_SIZE, root,             HASH_SIZE);
                compute_kernel(buffer, HASH_SIZE * 2, DOMAIN_NODE, state->seed,
                               state->has_key ? state->key : NULL, root);
            }
        }
    }

    /* Length commitment: prevents length extension attacks. */
    uint8_t final_buf[HASH_SIZE + sizeof(uint64_t) * 2];
    memcpy(final_buf,                   root,              HASH_SIZE);
    memcpy(final_buf + HASH_SIZE,       &state->domain,    sizeof(uint64_t));
    memcpy(final_buf + HASH_SIZE + 8,   &state->total_len, sizeof(uint64_t));
    compute_kernel(final_buf, sizeof(final_buf), 0, state->seed,
                   state->has_key ? state->key : NULL, out);
    free(state);
}

// =============================================================================
// PUBLIC ONE-SHOT API
// =============================================================================

int tachyon_hash_full(const uint8_t *input, size_t len, uint64_t domain,
                      uint64_t seed, const uint8_t *key, uint8_t *out) {
    if (!input || !out) {
        return TACHYON_ERROR_NULL_PTR;
    }

    if (len < CHUNK_SIZE) {
        compute_kernel(input, len, domain, seed, key, out);
    } else {
        tachyon_internal_state_t *s = new_state(domain, seed, key);
        if (!s) {
            return TACHYON_ERROR_NULL_PTR;
        }
        update_state(s, input, len);
        finalize_state(s, out);
    }
    return 0;
}

int tachyon_hash(const uint8_t *input, size_t len, uint8_t *out) {
    return tachyon_hash_full(input, len, 0, 0, NULL, out);
}

int tachyon_hash_seeded(const uint8_t *input, size_t len, uint64_t seed, uint8_t *out) {
    return tachyon_hash_full(input, len, 0, seed, NULL, out);
}

int tachyon_hash_with_domain(const uint8_t *input, size_t len, uint64_t domain, uint8_t *out) {
    return tachyon_hash_full(input, len, domain, 0, NULL, out);
}

int tachyon_hash_keyed(const uint8_t *input, size_t len, const uint8_t *key, uint8_t *out) {
    return tachyon_hash_full(input, len, TACHYON_DOMAIN_MESSAGE_AUTH, 0, key, out);
}

// =============================================================================
// STREAMING API
// =============================================================================

tachyon_state_t* tachyon_hasher_new(void) {
    return (tachyon_state_t*)new_state(0, 0, NULL);
}

tachyon_state_t* tachyon_hasher_new_with_domain(uint64_t domain) {
    return (tachyon_state_t*)new_state(domain, 0, NULL);
}

tachyon_state_t* tachyon_hasher_new_seeded(uint64_t seed) {
    return (tachyon_state_t*)new_state(0, seed, NULL);
}

tachyon_state_t* tachyon_hasher_new_full(uint64_t domain, uint64_t seed, const uint8_t *key) {
    return (tachyon_state_t*)new_state(domain, seed, key);
}

void tachyon_hasher_update(tachyon_state_t *state, const uint8_t *data, size_t len) {
    update_state((tachyon_internal_state_t*)state, data, len);
}

void tachyon_hasher_finalize(tachyon_state_t *state, uint8_t *out) {
    finalize_state((tachyon_internal_state_t*)state, out);
}

void tachyon_hasher_free(tachyon_state_t *state) {
    if (state) {
        free(state);
    }
}

// =============================================================================
// VERIFICATION HELPERS
// =============================================================================

/* Constant-time comparison. Branchless XOR fold prevents timing side-channels. */
static int ct_eq32(const uint8_t *a, const uint8_t *b) {
    uint8_t diff = 0;

    for (int i = 0; i < HASH_SIZE; i++) {
        diff |= a[i] ^ b[i];
    }

    return 1 - (int)((unsigned int)(diff | -diff) >> 31);
}

int tachyon_verify(const uint8_t *input, size_t len, const uint8_t *hash) {
    if (!input || !hash) {
        return TACHYON_ERROR_NULL_PTR;
    }

    uint8_t buf[HASH_SIZE];
    int rc = tachyon_hash(input, len, buf);
    if (rc != 0) {
        return rc;
    }

    return ct_eq32(buf, hash);
}

int tachyon_verify_mac(const uint8_t *input, size_t len, const uint8_t *key, const uint8_t *mac) {
    if (!input || !key || !mac) {
        return TACHYON_ERROR_NULL_PTR;
    }

    uint8_t buf[HASH_SIZE];
    int rc = tachyon_hash_keyed(input, len, key, buf);
    if (rc != 0) {
        return rc;
    }

    return ct_eq32(buf, mac);
}

/* Key derivation with domain separation. */
int tachyon_derive_key(const char *context, size_t context_len,
                       const uint8_t *material, uint8_t *out) {
    if (!context || !material || !out) {
        return TACHYON_ERROR_NULL_PTR;
    }
    return tachyon_hash_full((const uint8_t*)context, context_len,
                             TACHYON_DOMAIN_KEY_DERIVATION, 0, material, out);
}
