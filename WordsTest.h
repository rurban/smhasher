#include "Stats.h"
#include <iostream>
#include <fstream>
template < typename hashtype >
bool WordsTest( hashfunc<hashtype> hash, double confidence) {
    std::vector<hashtype> hashes(100000);
    std::string line;
    std::string filename= "/usr/share/dict/words";
    std::ifstream wordfile (filename.c_str());
    int count;
    if (wordfile.is_open())
    {
        while ( getline (wordfile,line) )
        {
            if (count >= hashes.size()) hashes.resize(count+100);
            hash(line.c_str(),line.length(),&hashes[count++]);
        }
        wordfile.close();
        hashes.resize(count);
        printf("# Read %d lines from %s\n", count, filename.c_str());
    } else {
        std::cout << "Unable to open word file"; 
        exit(1);
    }
    return TestHashList<hashtype>(hashes,true,confidence,false,"Keyset 'Words'");
}
