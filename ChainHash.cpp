/*
 * MIT License
 *
 * Copyright (c) 2026 Thomas Dybdahl Ahle
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#include "chainhash/chainhash.h"
#include "chainhash/chainhash128.h"
#include "ChainHash.h"
#include <atomic>

// The classic pfHash API passes a 32-bit seed with every call, and the speed tests
// change it on every call. Key expansion therefore happens once per Hash_Seed_init:
// its seed is expanded by the headers' own seed constructors (SplitMix64 outputs
// become the key bytes s, y, c0..c4, tau), the expansion the SMHasher3 registration
// uses. The 32-bit seed of each call enters as tau XOR seed. tau is the key word added
// to the Horner value before the finalizer, so every (init seed, call seed) pair is an
// ordinary ChainHash key; a call costs one store for its seed instead of a key
// expansion. PolymurHash takes its per-call tweak at the same point and CLHASH writes
// its per-call seed into its key. A 64-bit seed carries 64 bits of entropy; the
// collision bound assumes 64 (resp. 128) uniformly random key bytes and is not a claim
// about this adapter.
static chainhash_key g_chainhash_key;
static chainhash128_key g_chainhash128_key;
static std::atomic<unsigned> g_chainhash_generation{0};

void chainhash_seed_init(size_t seed) {
    g_chainhash_key = chainhash_key_from_seed((uint64_t)seed);
    g_chainhash128_key = chainhash128_key_from_seed((uint64_t)seed);
    g_chainhash_generation.fetch_add(1, std::memory_order_release);
}

namespace {
struct ChainHashDefaultKey { ChainHashDefaultKey() { chainhash_seed_init(0); } } chainhash_default_key;
}

void chainhash_test(const void * in, int len, uint32_t seed, void * out) {
    static thread_local chainhash_key key;
    static thread_local unsigned generation = 0;
    const unsigned g = g_chainhash_generation.load(std::memory_order_acquire);
    if (generation != g) { key = g_chainhash_key; generation = g; }
    key.tau = g_chainhash_key.tau ^ seed;
    const uint64_t h = chainhash(&key, in, (size_t)len);
    for (unsigned i = 0; i < 8; ++i) ((uint8_t *)out)[i] = (uint8_t)(h >> (8 * i));
}

// The 128-bit digest is written as its 16 canonical little-endian bytes.
void chainhash128_test(const void * in, int len, uint32_t seed, void * out) {
    static thread_local chainhash128_key key;
    static thread_local unsigned generation = 0;
    const unsigned g = g_chainhash_generation.load(std::memory_order_acquire);
    if (generation != g) { key = g_chainhash128_key; generation = g; }
    key.tau.lo = g_chainhash128_key.tau.lo ^ seed;
    chainhash128_store(out, chainhash128(&key, in, (size_t)len));
}
