#include "Stats.h"
#include <iostream>
#include <fstream>
template < typename hashtype >
bool WordsTest( hashfunc<hashtype> hash, double confidence) {
    std::vector<hashtype> hashes(200000);
    std::string line;
    std::string filename= "/usr/share/dict/words";
    int count= 0;
    int lines= 0;
    Rand r(82762);
    hash.seed_state_rand(r);
    std::ifstream wordfile (filename.c_str());
    if (!wordfile.is_open())
    {
        std::cout << "Unable to open word file " << filename << "\n";
        exit(1);
    }
    while ( getline (wordfile,line) )
    {
        lines++;
        if (count+3 >= hashes.size())
            hashes.resize(count+100);

        hash(line.c_str(),line.length(),&hashes[count++]);

        line.append("!");
        hash(line.c_str(),line.length(),&hashes[count++]);

        line.append("!");
        hash(line.c_str(),line.length(),&hashes[count++]);
    }
    wordfile.close();
    hashes.resize(count);
    printf("# Hashed %d keys from %d words from file '%s'\n",
            lines, count, filename.c_str());
    return TestHashList<hashtype>(hashes,true,confidence,false,"Keyset 'Words'");
}
