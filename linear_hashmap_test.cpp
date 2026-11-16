#include "LinearHashMap.h"
#include <string>

static size_t collide(const std::string &) { return 7; }

int main()
{
  LinearHashMap map(1500, collide);
  const std::string binary1("a\0b", 3), binary2("a\0c", 3);
  int *first = map.get_or_insert(binary1);
  if (!first || *first != 0) return 1;
  *first = 11;
  int *second = map.get_or_insert(binary2);
  if (!second || *second != 0) return 1;
  *second = 22;
  if (!map.erase(binary1) || *map.get_or_insert(binary2) != 22) return 1;
  int *again = map.get_or_insert(binary1);
  if (!again || *again != 0) return 1;
  *again = 33;
  if (*map.get_or_insert(binary1) != 33 || *map.get_or_insert(binary2) != 22) return 1;
  if (map.erase("missing") || map.limit_reached() || map.collisions() == 0) return 1;

  LinearHashMap pathological(1500, collide);
  for (int i = 0; i < LinearHashMap::kProbeLimit; ++i) {
    if (!pathological.get_or_insert(std::to_string(i))) return 1;
  }
  if (pathological.get_or_insert("overflow") || !pathological.limit_reached()) return 1;
  return 0;
}
