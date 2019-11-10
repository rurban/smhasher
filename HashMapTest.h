#include "Platform.h"
#include "Types.h"

#include <unordered_map>

class HashMap {
  std::unordered_map<const char *, int, pfHash> htmap;
};

bool HashMapTest ( pfHash hash, const int hashbits, bool verbose );
