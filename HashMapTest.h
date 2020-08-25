#pragma once

#include "Platform.h"
#include "Types.h"

std::vector<std::string> HashMapInit(bool verbose);
bool HashMapTest ( pfHash pfhash, 
                   const int hashbits, std::vector<std::string> words,
                   const uint32_t seed, const int trials, bool verbose );
