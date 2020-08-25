#pragma once

#include "Types.h"

void BulkSpeedTest ( pfHash hash, uint32_t seed );
double TinySpeedTest ( pfHash hash, int hashsize, int keysize, uint32_t seed, bool verbose );
double HashMapSpeedTest ( pfHash pfhash, int hashbits, std::vector<std::string> words,
                          const uint32_t seed, const int trials, bool verbose );
//-----------------------------------------------------------------------------
