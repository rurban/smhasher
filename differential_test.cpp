#include "DifferentialTest.h"

int main()
{
  std::vector<uint32_t> diffs;
  if (!ProcessDifferentials(diffs, 10, false)) return 1;
  diffs.push_back(1);
  diffs.push_back(2);
  if (!ProcessDifferentials(diffs, 10, false)) return 1;
  diffs.push_back(2);
  if (ProcessDifferentials(diffs, 10, false)) return 1;
  diffs.push_back(1);
  if (ProcessDifferentials(diffs, 10, false)) return 1;
  return 0;
}
