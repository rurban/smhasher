// FxHash byte hashing, adapted from fxhash 0.2.1 on crates.io.
// This adaptation uses the SMHasher seed as the initial state; seed zero
// matches FxHasher32/FxHasher64's default state.
//
// Copyright (c) The Rust Project Contributors
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#pragma once
#include <stdint.h>
#include <stddef.h>
#include <string.h>

static inline uint32_t fxhash32(const void *key, size_t len, uint32_t hash)
{
  const uint8_t *p = (const uint8_t *)key;
  while (len >= 4) {
    uint32_t word;
    memcpy(&word, p, 4);
    hash = ((hash << 5) | (hash >> 27)) ^ word;
    hash *= UINT32_C(0x27220a95);
    p += 4; len -= 4;
  }
  while (len--) {
    hash = (((hash << 5) | (hash >> 27)) ^ *p++) * UINT32_C(0x27220a95);
  }
  return hash;
}

static inline uint64_t fxhash64(const void *key, size_t len, uint64_t hash)
{
  const uint8_t *p = (const uint8_t *)key;
  while (len >= 8) {
    uint64_t word;
    memcpy(&word, p, 8);
    hash = (((hash << 5) | (hash >> 59)) ^ word) * UINT64_C(0x517cc1b727220a95);
    p += 8; len -= 8;
  }
  if (len >= 4) {
    uint32_t word;
    memcpy(&word, p, 4);
    hash = (((hash << 5) | (hash >> 59)) ^ word) * UINT64_C(0x517cc1b727220a95);
    p += 4; len -= 4;
  }
  while (len--) {
    hash = (((hash << 5) | (hash >> 59)) ^ *p++) * UINT64_C(0x517cc1b727220a95);
  }
  return hash;
}
