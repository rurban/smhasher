#pragma once

#include "Types.h"

constexpr int TIMEHASH_SMALL_LEN_MAX = 255;

void BulkSpeedTest ( pfHash hash, uint32_t seed );
double TinySpeedTest ( pfHash hash, int hashsize, int keysize, uint32_t seed, bool verbose );
double HashMapSpeedTest ( pfHash pfhash, int hashbits, std::vector<std::string> words,
                          const uint32_t seed, const int trials, bool verbose );
void ReportTinySpeedTest ( const std::vector<double>& cycles_per_hash, int minkey, int maxkey );
