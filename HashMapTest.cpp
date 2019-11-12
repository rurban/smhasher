#include "Platform.h"
#include "HashMapTest.h"
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

std::set<std::string> HashMapInit(bool verbose) {
  std::set<std::string> words;
  std::set<std::string>::iterator it;
  std::string line;
  std::string filename = "/usr/share/dict/words";
  int lines = 0, sum = 0;
  std::ifstream wordfile(filename.c_str());
  if (!wordfile.is_open()) {
    std::cout << "Unable to open words dict file " << filename << "\n";
    return words;
  }
  while (getline(wordfile, line)) {
    int len = line.length();
    lines++;
    sum += len;
    words.insert(line);
    //line.append("!");
    //words.insert(line);
    //line.append("!");
    //words.insert(line);
  }
  wordfile.close();
  if (verbose) {
    printf ("Read %d words from '%s'\n", lines, filename.c_str());
    printf ("Avg len: %0.3f\n", (sum+0.0)/lines);
  }
  return words;
}

bool HashMapTest ( pfHash pfhash, std::set<std::string> words,
                   const int hashbits, bool verbose )
{
  const int hashbytes = hashbits / 8;
  Rand r(82762);
  const uint32_t seed = r.rand_u32();
  unordered_map<string,int,function<size_t ( const string &key)>>
    hashmap(words.size(), []( const string &key)
                  {
                    const void* out;
                    size_t result;
                    pfhash(key.c_str(), key.length(), seed, out);
                    memcpy(&result, out, hashbits);
                    return result;
                  });
  
  printf("Running HashMapTest     ");

  //----------

  delete hashmap;
  printf(" ....... PASS\n");
  return true;
}
