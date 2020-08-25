#ifndef UMASH_H
#define UMASH_H
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/**
 * # UMASH: a non-cryptographic hash function with collision bounds
 *
 * SPDX-License-Identifier: MIT
 * Copyright 2020 Backtrace I/O, Inc.
 *
 * UMASH is a fast (9-22 ns latency for inputs of 1-64 bytes and 22
 * GB/s peak throughput, on a 2.5 GHz Intel 8175M) 64-bit hash
 * function with mathematically proven collision bounds: it is
 * [ceil(l / 2048) * 2^{-56}]-almost-universal for inputs of l or
 * fewer bytes.
 *
 * When that's not enough, UMASH can also generate two independent
 * 64-bit hashes in a single traversal.  The resulting fingerprint
 * squares the collision probability to less than
 * ceil(l / 2048)^2 * 2^{-112}; the probability that two distinct
 * inputs receive the same fingerprint is less than 2^{-70} as long
 * as they are shorter than 7.5 GB each.  This expectation is taken
 * over the randomly generated `umash_params`; if an attacker can
 * infer the contents of these parameters, the bounds do not apply.
 *
 * ## Initialisation
 *
 * In order to use `UMASH`, one must first generate a `struct
 * umash_params`; each such param defines a distinct `UMASH` function
 * (a pair of such functions, in fact).  Ideally, one would fill
 * a struct with random bytes and call`umash_params_prepare`.
 *
 * - `umash_params_prepare`: attempts to convert the contents of
 *   randomly filled `struct umash_params` into a valid UMASH
 *   parameter struct (key).  When the input consists of uniformly
 *   generated random bytes, the probability of failure is
 *   astronomically small.
 *
 * - `umash_params_derive`: deterministically constructs a `struct
 *   umash_params` from a 64-bit seed and an optional 32-byte secret.
 *   The seed and secret are expanded into random bytes with Salsa20;
 *   the resulting `umash_params` should be practically random, as
 *   long the seed or secret are unknown.
 *
 * ## Batch hashing and fingerprinting
 *
 * Once we have a `struct umash_params`, we can use `umash_full` or
 * `umash_fprint` like regular hash functions.
 *
 * - `umash_full` can compute either of the two UMASH functions
 *   described by a `struct umash_params`.  Its `seed` argument will
 *   change the output, but is not associated with any collision
 *   bound.
 *
 * - `umash_fprint` computes both `UMASH` functions described by a
 *   `struct umash_params`.  `umash_fp::hash[0]` corresnponds to
 *   calling `umash_full` with the same arguments and `which = 0`;
 *   `umash_fp::hash[1]` corresponds to `which = 1`.
 *
 * ## Incremental hashing and fingerprinting
 *
 * We can also compute UMASH values by feeding bytes incrementally.
 * The result is guaranteed to the same as if we had buffered all the
 * bytes and called `umash_full` or `umash_fprint`.
 *
 * - `umash_init` initialises a `struct umash_state` with the same
 *   parameters one would pass to `umash_full`.
 *
 * - `umash_digest` computes the value `umash_full` would return
 *   were it passed the arguments that were given to `umash_init`,
 *   and the bytes "fed" into the `umash_state`.
 *
 * - `umash_fp_init` initialises a `struct umash_fp_state` with the
 *   same parameters one would pass to `umash_fprint`.
 *
 * - `umash_fp_digest` computes the value `umash_fprint` would return
 *   for the bytes "fed" into the `umash_fp_state`.
 *
 * In both cases, one passes a pointer to `struct umash_state::sink`
 * or `struct umash_fp_state::sink` to callees that wish to feed bytes
 * into the `umash_state` or `umash_fp_state`.
 *
 * - `umash_sink_update` feeds a byte range to the `umash_sink`
 *   initialised by calling `umash_init` or `umash_fp_init`.  The sink
 *   does not take ownership of anything and the input bytes may be
 *   overwritten or freed as soon as `umash_sink_update` returns.
 */

#ifdef __cplusplus
extern "C" {
#endif

enum { UMASH_PH_PARAM_COUNT = 32, UMASH_PH_TOEPLITZ_SHIFT = 4 };

/**
 * A single UMASH params struct stores the parameters for a pair of
 * independent `UMASH` functions.
 */
struct umash_params {
	/*
	 * Each uint64_t[2] array consists of {f^2, f}, where f is a
	 * random multiplier in mod 2**61 - 1.
	 */
	uint64_t poly[2][2];
	/*
	 * The second PH function starts reading parameters from the
	 * 4th element.
	 */
	uint64_t ph[UMASH_PH_PARAM_COUNT + UMASH_PH_TOEPLITZ_SHIFT];
};

/**
 * A fingerprint consists of two independent `UMASH` hash values.
 */
struct umash_fp {
	uint64_t hash[2];
};

/**
 * This struct holds the state for incremental UMASH hashing or
 * fingerprinting.
 *
 * A sink owns no allocation, and simply borrows a pointer to its
 * `umash_params`.  It can be byte-copied to snapshot its state.
 *
 * The layout works best with alignment to 64 bytes, but does not
 * require it.
 */
struct umash_sink {
	/*
	 * We incrementally maintain two states when fingerprinting.
	 * When hashing, only the first `poly_state` and `ph_acc`
	 * entries are active.
	 */
	struct {
		uint64_t mul[2]; /* Multiplier, and multiplier^2. */
		uint64_t acc; /* Current Horner accumulator. */
	} poly_state[2];

	/*
	 * We write new bytes to the second half, and keep the previous
	 * 16 byte chunk in the first half.
	 */
	char buf[2 * 16];

	/* The next 64 bytes are accessed in the `PH` inner loop. */

	/* key->ph, shifted when evaluating only the second UMASH function. */
	const uint64_t *ph;

	/* ph_iter tracks where we are in the inner loop, times 2. */
	uint32_t ph_iter;
	uint8_t bufsz; /* Write pointer in `buf + 16`. */
	uint8_t block_size; /* Current PH block size, excluding `bufsz`. */
	bool large_umash; /* True once we definitely have >= 16 bytes. */
	/*
	 * If true, we're computing a fingerprint.  Otherwise, we only
	 * want to update the first PH/poly state.
	 */
	bool fingerprinting;

	/* Accumulators for the current PH value. */
	struct umash_ph {
		uint64_t bits[2];
	} ph_acc[2];

	uint64_t seed;
};

/**
 * The `umash_state` struct wraps a sink in a type-safe interface: we
 * don't want to try and extract a fingerprint from a sink configured
 * for hashing.
 */
struct umash_state {
	struct umash_sink sink;
};

/**
 * Similarly, the `umash_fp_state` struct wraps a sink from which we
 * should extract a fingerprint.
 */
struct umash_fp_state {
	struct umash_sink sink;
};

/**
 * Converts a `umash_params` struct filled with random values into
 * something usable by the UMASH functions below.
 *
 * When it succeeds, this function is idempotent.  Failure happens
 * with probability < 2**-110 is `params` is filled with uniformly
 * distributed random bits.  That's an astronomically unlikely event,
 * and most likely signals an issue with the caller's (pseudo-)random
 * number generator.
 *
 * @return false on failure, probably because the input was not random.
 */
bool umash_params_prepare(struct umash_params *params);

/**
 * Deterministically derives a `umash_params` struct from `bits` and
 * `extension`.  These values do not have to be particularly well
 * distributed, and can be generated sequentially; in fact, it is safe
 * to always let `extension` be 0.
 *
 * @param key a pointer to exactly 32 secret bytes.  NULL will be
 *   replaced with "Do not use UMASH VS adversaries.", the default
 *   UMASH secret.
 */
void umash_params_derive(struct umash_params *, uint64_t bits, const void *key);

/**
 * Updates a `umash_sink` to take into account `data[0 ... n_bytes)`.
 */
void umash_sink_update(struct umash_sink *, const void *data, size_t n_bytes);

/**
 * Computes the UMASH hash of `data[0 ... n_bytes)`.
 *
 * Randomly generated `param` lead to independent UMASH values and
 * associated worst-case collision bounds; changing the `seed` comes
 * with no guarantee.
 *
 * @param which 0 to compute the first UMASH defined by `params`, 1
 *   for the second.
 */
uint64_t umash_full(const struct umash_params *params, uint64_t seed, int which,
    const void *data, size_t n_bytes);

/**
 * Computes the UMASH fingerprint of `data[0 ... n_bytes)`.
 *
 * Randomly generated `param` lead to independent UMASH values and
 * associated worst-case collision bounds; changing the `seed` comes
 * with no guarantee.
 */
struct umash_fp umash_fprint(
    const struct umash_params *params, uint64_t seed, const void *data, size_t n_bytes);

/**
 * Prepares a `umash_state` for computing the `which`th UMASH function in
 * `params`.
 */
void umash_init(
    struct umash_state *, const struct umash_params *params, uint64_t seed, int which);

/**
 * Returns the UMASH value for the bytes that have been
 * `umash_sink_update`d into the state.
 */
uint64_t umash_digest(const struct umash_state *);

/**
 * Prepares a `umash_fp_state` for computing the UMASH fingerprint in
 * `params`.
 */
void umash_fp_init(
    struct umash_fp_state *, const struct umash_params *params, uint64_t seed);

/**
 * Returns the UMASH fingerprint for the bytes that have been
 * `umash_sink_update`d into the state.
 */
struct umash_fp umash_fp_digest(const struct umash_fp_state *);

#ifdef __cplusplus
}
#endif
#endif /* !UMASH_H */
