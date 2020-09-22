#include "Platform.h"
#include "HashMapTest.h"
#include "SpeedTest.h"
#include "Random.h"

#include <string>
#include <unordered_map>
#include <functional>
#include <iostream>
#include <fstream>

using namespace std;

//-----------------------------------------------------------------------------
// This should be a realistic I-Cache test, when our hash is used inlined
// in a hash table. There the size matters more than the bulk speed.

std::vector<std::string> HashMapInit(bool verbose) {
  std::vector<std::string> words;
  std::string line;
  std::string filename = "/usr/share/dict/words";
  int lines = 0, sum = 0;
  std::ifstream wordfile(filename.c_str());
  if (!wordfile.is_open()) {
#ifdef __aarch64__ /* should be __ANDROID_API__, but not yet */
    filename = "dict.words";
    std::ifstream wordfile1(filename.c_str());
    if (wordfile1.is_open()) {
      wordfile = move(wordfile1);
      goto ok_file;
    }
#endif
    std::cout << "Unable to open words dict file " << filename << "\n";
    return words;
  }
 ok_file:
  while (getline(wordfile, line)) {
    int len = line.length();
    lines++;
    sum += len;
    words.push_back(line);
  }
  wordfile.close();
  if (verbose) {
    printf ("Read %d words from '%s'\n", lines, filename.c_str());
    printf ("Avg len: %0.3f\n", (sum+0.0)/lines);
  }
  return words;
}

bool HashMapTest ( pfHash pfhash, 
                   const int hashbits, std::vector<std::string> words,
                   const uint32_t seed, const int trials, bool verbose )
{
  double mean = 0.0;
  try {
    mean = HashMapSpeedTest( pfhash, hashbits, words, seed, trials, verbose);
  }
  catch (...) {
    printf(" aborted !!!!\n");
  }
  // if faster than ~sha1
  if (mean > 5. && mean < 1500.)
    printf(" ....... PASS\n");
  else
    printf(" ....... FAIL\n");
  return true;
}
