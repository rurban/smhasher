make
rm -f doc/*.tmp
export BITS=

#BeagleHash_32_112	32 bit	(Yves Orton's hash for 64-bit reduced to 32 (112 bit seed).)

if [ "x$BITS" == "x" -o "x$BITS" == "x32" ]; then
    touch doc/BeagleHash_32_112_32.out &&
    ./SMHasher BeagleHash_32_112 2>&1 | tee doc/BeagleHash_32_112_32.tmp &&
    mv doc/BeagleHash_32_112_32.out doc/BeagleHash_32_112_32.bak &&
    mv doc/BeagleHash_32_112_32.tmp doc/BeagleHash_32_112_32.out
fi


#BeagleHash_32_127	32 bit	(Yves Orton's hash for 64-bit reduced to 32 (127 bit seed).)

if [ "x$BITS" == "x" -o "x$BITS" == "x32" ]; then
    touch doc/BeagleHash_32_127_32.out &&
    ./SMHasher BeagleHash_32_127 2>&1 | tee doc/BeagleHash_32_127_32.tmp &&
    mv doc/BeagleHash_32_127_32.out doc/BeagleHash_32_127_32.bak &&
    mv doc/BeagleHash_32_127_32.tmp doc/BeagleHash_32_127_32.out
fi


#BeagleHash_32_32	32 bit	(Yves Orton's hash for 64-bit reduced to 32 (32 bit seed).)

if [ "x$BITS" == "x" -o "x$BITS" == "x32" ]; then
    touch doc/BeagleHash_32_32_32.out &&
    ./SMHasher BeagleHash_32_32 2>&1 | tee doc/BeagleHash_32_32_32.tmp &&
    mv doc/BeagleHash_32_32_32.out doc/BeagleHash_32_32_32.bak &&
    mv doc/BeagleHash_32_32_32.tmp doc/BeagleHash_32_32_32.out
fi


#BeagleHash_32_64	32 bit	(Yves Orton's hash for 64-bit reduced to 32 (64 bit seed).)

if [ "x$BITS" == "x" -o "x$BITS" == "x32" ]; then
    touch doc/BeagleHash_32_64_32.out &&
    ./SMHasher BeagleHash_32_64 2>&1 | tee doc/BeagleHash_32_64_32.tmp &&
    mv doc/BeagleHash_32_64_32.out doc/BeagleHash_32_64_32.bak &&
    mv doc/BeagleHash_32_64_32.tmp doc/BeagleHash_32_64_32.out
fi


#BeagleHash_32_96	32 bit	(Yves Orton's hash for 64-bit reduced to 32 (96 bit seed).)

if [ "x$BITS" == "x" -o "x$BITS" == "x32" ]; then
    touch doc/BeagleHash_32_96_32.out &&
    ./SMHasher BeagleHash_32_96 2>&1 | tee doc/BeagleHash_32_96_32.tmp &&
    mv doc/BeagleHash_32_96_32.out doc/BeagleHash_32_96_32.bak &&
    mv doc/BeagleHash_32_96_32.tmp doc/BeagleHash_32_96_32.out
fi


#bernstein       	32 bit	(Bernstein, 32-bit)

if [ "x$BITS" == "x" -o "x$BITS" == "x32" ]; then
    touch doc/bernstein_32.out &&
    ./SMHasher bernstein 2>&1 | tee doc/bernstein_32.tmp &&
    mv doc/bernstein_32.out doc/bernstein_32.bak &&
    mv doc/bernstein_32.tmp doc/bernstein_32.out
fi


#City32          	32 bit	(Google CityHash32WithSeed (old))

if [ "x$BITS" == "x" -o "x$BITS" == "x32" ]; then
    touch doc/City32_32.out &&
    ./SMHasher City32 2>&1 | tee doc/City32_32.tmp &&
    mv doc/City32_32.out doc/City32_32.bak &&
    mv doc/City32_32.tmp doc/City32_32.out
fi


#fasthash32      	32 bit	(fast-hash 32bit)

if [ "x$BITS" == "x" -o "x$BITS" == "x32" ]; then
    touch doc/fasthash32_32.out &&
    ./SMHasher fasthash32 2>&1 | tee doc/fasthash32_32.tmp &&
    mv doc/fasthash32_32.out doc/fasthash32_32.bak &&
    mv doc/fasthash32_32.tmp doc/fasthash32_32.out
fi


#FNV1a           	32 bit	(Fowler-Noll-Vo hash, 32-bit)

if [ "x$BITS" == "x" -o "x$BITS" == "x32" ]; then
    touch doc/FNV1a_32.out &&
    ./SMHasher FNV1a 2>&1 | tee doc/FNV1a_32.tmp &&
    mv doc/FNV1a_32.out doc/FNV1a_32.bak &&
    mv doc/FNV1a_32.tmp doc/FNV1a_32.out
fi


#FNV1a_YT        	32 bit	(FNV1a-YoshimitsuTRIAD 32-bit sanmayce)

if [ "x$BITS" == "x" -o "x$BITS" == "x32" ]; then
    touch doc/FNV1a_YT_32.out &&
    ./SMHasher FNV1a_YT 2>&1 | tee doc/FNV1a_YT_32.tmp &&
    mv doc/FNV1a_YT_32.out doc/FNV1a_YT_32.bak &&
    mv doc/FNV1a_YT_32.tmp doc/FNV1a_YT_32.out
fi


#GoodOAAT        	32 bit	(Small non-multiplicative OAAT)

if [ "x$BITS" == "x" -o "x$BITS" == "x32" ]; then
    touch doc/GoodOAAT_32.out &&
    ./SMHasher GoodOAAT 2>&1 | tee doc/GoodOAAT_32.tmp &&
    mv doc/GoodOAAT_32.out doc/GoodOAAT_32.bak &&
    mv doc/GoodOAAT_32.tmp doc/GoodOAAT_32.out
fi


#HalfSipHash     	32 bit	(HalfSipHash 2-4, 32bit)

if [ "x$BITS" == "x" -o "x$BITS" == "x32" ]; then
    touch doc/HalfSipHash_32.out &&
    ./SMHasher HalfSipHash 2>&1 | tee doc/HalfSipHash_32.tmp &&
    mv doc/HalfSipHash_32.out doc/HalfSipHash_32.bak &&
    mv doc/HalfSipHash_32.tmp doc/HalfSipHash_32.out
fi


#JenkinsOOAT     	32 bit	(Bob Jenkins' OOAT as in perl 5.18)

if [ "x$BITS" == "x" -o "x$BITS" == "x32" ]; then
    touch doc/JenkinsOOAT_32.out &&
    ./SMHasher JenkinsOOAT 2>&1 | tee doc/JenkinsOOAT_32.tmp &&
    mv doc/JenkinsOOAT_32.out doc/JenkinsOOAT_32.bak &&
    mv doc/JenkinsOOAT_32.tmp doc/JenkinsOOAT_32.out
fi


#JenkinsOOAT_perl	32 bit	(Bob Jenkins' OOAT as in old perl5)

if [ "x$BITS" == "x" -o "x$BITS" == "x32" ]; then
    touch doc/JenkinsOOAT_perl_32.out &&
    ./SMHasher JenkinsOOAT_perl 2>&1 | tee doc/JenkinsOOAT_perl_32.tmp &&
    mv doc/JenkinsOOAT_perl_32.out doc/JenkinsOOAT_perl_32.bak &&
    mv doc/JenkinsOOAT_perl_32.tmp doc/JenkinsOOAT_perl_32.out
fi


#lookup3         	32 bit	(Bob Jenkins' lookup3)

if [ "x$BITS" == "x" -o "x$BITS" == "x32" ]; then
    touch doc/lookup3_32.out &&
    ./SMHasher lookup3 2>&1 | tee doc/lookup3_32.tmp &&
    mv doc/lookup3_32.out doc/lookup3_32.bak &&
    mv doc/lookup3_32.tmp doc/lookup3_32.out
fi


#Marvin32        	32 bit	(Marvin32 from MicroSoft)

if [ "x$BITS" == "x" -o "x$BITS" == "x32" ]; then
    touch doc/Marvin32_32.out &&
    ./SMHasher Marvin32 2>&1 | tee doc/Marvin32_32.tmp &&
    mv doc/Marvin32_32.out doc/Marvin32_32.bak &&
    mv doc/Marvin32_32.tmp doc/Marvin32_32.out
fi


#MicroOAAT       	32 bit	(Small non-multiplicative OAAT that passes all collision checks (by funny-falcon))

if [ "x$BITS" == "x" -o "x$BITS" == "x32" ]; then
    touch doc/MicroOAAT_32.out &&
    ./SMHasher MicroOAAT 2>&1 | tee doc/MicroOAAT_32.tmp &&
    mv doc/MicroOAAT_32.out doc/MicroOAAT_32.bak &&
    mv doc/MicroOAAT_32.tmp doc/MicroOAAT_32.out
fi


#Murmur2         	32 bit	(MurmurHash2 for x86, 32-bit)

if [ "x$BITS" == "x" -o "x$BITS" == "x32" ]; then
    touch doc/Murmur2_32.out &&
    ./SMHasher Murmur2 2>&1 | tee doc/Murmur2_32.tmp &&
    mv doc/Murmur2_32.out doc/Murmur2_32.bak &&
    mv doc/Murmur2_32.tmp doc/Murmur2_32.out
fi


#Murmur2A        	32 bit	(MurmurHash2A for x86, 32-bit)

if [ "x$BITS" == "x" -o "x$BITS" == "x32" ]; then
    touch doc/Murmur2A_32.out &&
    ./SMHasher Murmur2A 2>&1 | tee doc/Murmur2A_32.tmp &&
    mv doc/Murmur2A_32.out doc/Murmur2A_32.bak &&
    mv doc/Murmur2A_32.tmp doc/Murmur2A_32.out
fi


#Murmur3A        	32 bit	(MurmurHash3 for x86, 32-bit)

if [ "x$BITS" == "x" -o "x$BITS" == "x32" ]; then
    touch doc/Murmur3A_32.out &&
    ./SMHasher Murmur3A 2>&1 | tee doc/Murmur3A_32.tmp &&
    mv doc/Murmur3A_32.out doc/Murmur3A_32.bak &&
    mv doc/Murmur3A_32.tmp doc/Murmur3A_32.out
fi


#MurmurOAAT      	32 bit	(Murmur one-at-a-time)

if [ "x$BITS" == "x" -o "x$BITS" == "x32" ]; then
    touch doc/MurmurOAAT_32.out &&
    ./SMHasher MurmurOAAT 2>&1 | tee doc/MurmurOAAT_32.tmp &&
    mv doc/MurmurOAAT_32.out doc/MurmurOAAT_32.bak &&
    mv doc/MurmurOAAT_32.tmp doc/MurmurOAAT_32.out
fi


#Phat            	32 bit	(Phat Hash)

if [ "x$BITS" == "x" -o "x$BITS" == "x32" ]; then
    touch doc/Phat_32.out &&
    ./SMHasher Phat 2>&1 | tee doc/Phat_32.tmp &&
    mv doc/Phat_32.out doc/Phat_32.bak &&
    mv doc/Phat_32.tmp doc/Phat_32.out
fi


#Phat4           	32 bit	(Phat4 Hash)

if [ "x$BITS" == "x" -o "x$BITS" == "x32" ]; then
    touch doc/Phat4_32.out &&
    ./SMHasher Phat4 2>&1 | tee doc/Phat4_32.tmp &&
    mv doc/Phat4_32.out doc/Phat4_32.bak &&
    mv doc/Phat4_32.tmp doc/Phat4_32.out
fi


#PMurHash32      	32 bit	(Shane Day's portable-ized MurmurHash3 for x86, 32-bit.)

if [ "x$BITS" == "x" -o "x$BITS" == "x32" ]; then
    touch doc/PMurHash32_32.out &&
    ./SMHasher PMurHash32 2>&1 | tee doc/PMurHash32_32.tmp &&
    mv doc/PMurHash32_32.out doc/PMurHash32_32.bak &&
    mv doc/PMurHash32_32.tmp doc/PMurHash32_32.out
fi


#sdbm            	32 bit	(sdbm as in perl5)

if [ "x$BITS" == "x" -o "x$BITS" == "x32" ]; then
    touch doc/sdbm_32.out &&
    ./SMHasher sdbm 2>&1 | tee doc/sdbm_32.tmp &&
    mv doc/sdbm_32.out doc/sdbm_32.bak &&
    mv doc/sdbm_32.tmp doc/sdbm_32.out
fi


#Spooky32        	32 bit	(Bob Jenkins' SpookyHash, 32-bit result)

if [ "x$BITS" == "x" -o "x$BITS" == "x32" ]; then
    touch doc/Spooky32_32.out &&
    ./SMHasher Spooky32 2>&1 | tee doc/Spooky32_32.tmp &&
    mv doc/Spooky32_32.out doc/Spooky32_32.bak &&
    mv doc/Spooky32_32.tmp doc/Spooky32_32.out
fi


#superfast       	32 bit	(Paul Hsieh's SuperFastHash)

if [ "x$BITS" == "x" -o "x$BITS" == "x32" ]; then
    touch doc/superfast_32.out &&
    ./SMHasher superfast 2>&1 | tee doc/superfast_32.tmp &&
    mv doc/superfast_32.out doc/superfast_32.bak &&
    mv doc/superfast_32.tmp doc/superfast_32.out
fi


#x17             	32 bit	(x17)

if [ "x$BITS" == "x" -o "x$BITS" == "x32" ]; then
    touch doc/x17_32.out &&
    ./SMHasher x17 2>&1 | tee doc/x17_32.tmp &&
    mv doc/x17_32.out doc/x17_32.bak &&
    mv doc/x17_32.tmp doc/x17_32.out
fi


#xxHash32        	32 bit	(xxHash, 32-bit for x64)

if [ "x$BITS" == "x" -o "x$BITS" == "x32" ]; then
    touch doc/xxHash32_32.out &&
    ./SMHasher xxHash32 2>&1 | tee doc/xxHash32_32.tmp &&
    mv doc/xxHash32_32.out doc/xxHash32_32.bak &&
    mv doc/xxHash32_32.tmp doc/xxHash32_32.out
fi


#Zaphod32        	32 bit	(Marvin32 like hash)

if [ "x$BITS" == "x" -o "x$BITS" == "x32" ]; then
    touch doc/Zaphod32_32.out &&
    ./SMHasher Zaphod32 2>&1 | tee doc/Zaphod32_32.tmp &&
    mv doc/Zaphod32_32.out doc/Zaphod32_32.bak &&
    mv doc/Zaphod32_32.tmp doc/Zaphod32_32.out
fi


#BeagleHash_64_112	64 bit	(Yves Orton's hash for 64-bit (112 bit seed).)

if [ "x$BITS" == "x" -o "x$BITS" == "x64" ]; then
    touch doc/BeagleHash_64_112_64.out &&
    ./SMHasher BeagleHash_64_112 2>&1 | tee doc/BeagleHash_64_112_64.tmp &&
    mv doc/BeagleHash_64_112_64.out doc/BeagleHash_64_112_64.bak &&
    mv doc/BeagleHash_64_112_64.tmp doc/BeagleHash_64_112_64.out
fi


#BeagleHash_64_127	64 bit	(Yves Orton's hash for 64-bit (127 bit seed).)

if [ "x$BITS" == "x" -o "x$BITS" == "x64" ]; then
    touch doc/BeagleHash_64_127_64.out &&
    ./SMHasher BeagleHash_64_127 2>&1 | tee doc/BeagleHash_64_127_64.tmp &&
    mv doc/BeagleHash_64_127_64.out doc/BeagleHash_64_127_64.bak &&
    mv doc/BeagleHash_64_127_64.tmp doc/BeagleHash_64_127_64.out
fi


#BeagleHash_64_32	64 bit	(Yves Orton's hash for 64-bit. (32 bit seed))

if [ "x$BITS" == "x" -o "x$BITS" == "x64" ]; then
    touch doc/BeagleHash_64_32_64.out &&
    ./SMHasher BeagleHash_64_32 2>&1 | tee doc/BeagleHash_64_32_64.tmp &&
    mv doc/BeagleHash_64_32_64.out doc/BeagleHash_64_32_64.bak &&
    mv doc/BeagleHash_64_32_64.tmp doc/BeagleHash_64_32_64.out
fi


#BeagleHash_64_64	64 bit	(Yves Orton's hash for 64-bit. (64 bit seed))

if [ "x$BITS" == "x" -o "x$BITS" == "x64" ]; then
    touch doc/BeagleHash_64_64_64.out &&
    ./SMHasher BeagleHash_64_64 2>&1 | tee doc/BeagleHash_64_64_64.tmp &&
    mv doc/BeagleHash_64_64_64.out doc/BeagleHash_64_64_64.bak &&
    mv doc/BeagleHash_64_64_64.tmp doc/BeagleHash_64_64_64.out
fi


#BeagleHash_64_96	64 bit	(Yves Orton's hash for 64-bit (96 bit seed).)

if [ "x$BITS" == "x" -o "x$BITS" == "x64" ]; then
    touch doc/BeagleHash_64_96_64.out &&
    ./SMHasher BeagleHash_64_96 2>&1 | tee doc/BeagleHash_64_96_64.tmp &&
    mv doc/BeagleHash_64_96_64.out doc/BeagleHash_64_96_64.bak &&
    mv doc/BeagleHash_64_96_64.tmp doc/BeagleHash_64_96_64.out
fi


#City64          	64 bit	(Google CityHash64WithSeed)

if [ "x$BITS" == "x" -o "x$BITS" == "x64" ]; then
    touch doc/City64_64.out &&
    ./SMHasher City64 2>&1 | tee doc/City64_64.tmp &&
    mv doc/City64_64.out doc/City64_64.bak &&
    mv doc/City64_64.tmp doc/City64_64.out
fi


#City64          	64 bit	(Google CityHash64WithSeed (old))

if [ "x$BITS" == "x" -o "x$BITS" == "x64" ]; then
    touch doc/City64_64.out &&
    ./SMHasher City64 2>&1 | tee doc/City64_64.tmp &&
    mv doc/City64_64.out doc/City64_64.bak &&
    mv doc/City64_64.tmp doc/City64_64.out
fi


#cmetrohash64_1  	64 bit	(cmetrohash64_1, 64-bit for x64)

if [ "x$BITS" == "x" -o "x$BITS" == "x64" ]; then
    touch doc/cmetrohash64_1_64.out &&
    ./SMHasher cmetrohash64_1 2>&1 | tee doc/cmetrohash64_1_64.tmp &&
    mv doc/cmetrohash64_1_64.out doc/cmetrohash64_1_64.bak &&
    mv doc/cmetrohash64_1_64.tmp doc/cmetrohash64_1_64.out
fi


#cmetrohash64_1o 	64 bit	(cmetrohash64_1 (shorter key optimized) , 64-bit for x64)

if [ "x$BITS" == "x" -o "x$BITS" == "x64" ]; then
    touch doc/cmetrohash64_1o_64.out &&
    ./SMHasher cmetrohash64_1o 2>&1 | tee doc/cmetrohash64_1o_64.tmp &&
    mv doc/cmetrohash64_1o_64.out doc/cmetrohash64_1o_64.bak &&
    mv doc/cmetrohash64_1o_64.tmp doc/cmetrohash64_1o_64.out
fi


#cmetrohash64_2  	64 bit	(cmetrohash64_2, 64-bit for x64)

if [ "x$BITS" == "x" -o "x$BITS" == "x64" ]; then
    touch doc/cmetrohash64_2_64.out &&
    ./SMHasher cmetrohash64_2 2>&1 | tee doc/cmetrohash64_2_64.tmp &&
    mv doc/cmetrohash64_2_64.out doc/cmetrohash64_2_64.bak &&
    mv doc/cmetrohash64_2_64.tmp doc/cmetrohash64_2_64.out
fi


#falkhash        	64 bit	(falkhash.asm with aesenc, 64-bit for x64)

if [ "x$BITS" == "x" -o "x$BITS" == "x64" ]; then
    touch doc/falkhash_64.out &&
    ./SMHasher falkhash 2>&1 | tee doc/falkhash_64.tmp &&
    mv doc/falkhash_64.out doc/falkhash_64.bak &&
    mv doc/falkhash_64.tmp doc/falkhash_64.out
fi


#FarmHash64      	64 bit	(Google FarmHash64WithSeed)

if [ "x$BITS" == "x" -o "x$BITS" == "x64" ]; then
    touch doc/FarmHash64_64.out &&
    ./SMHasher FarmHash64 2>&1 | tee doc/FarmHash64_64.tmp &&
    mv doc/FarmHash64_64.out doc/FarmHash64_64.bak &&
    mv doc/FarmHash64_64.tmp doc/FarmHash64_64.out
fi


#farmhash64_c    	64 bit	(farmhash64_with_seed (C99))

if [ "x$BITS" == "x" -o "x$BITS" == "x64" ]; then
    touch doc/farmhash64_c_64.out &&
    ./SMHasher farmhash64_c 2>&1 | tee doc/farmhash64_c_64.tmp &&
    mv doc/farmhash64_c_64.out doc/farmhash64_c_64.bak &&
    mv doc/farmhash64_c_64.tmp doc/farmhash64_c_64.out
fi


#fasthash64      	64 bit	(fast-hash 64bit)

if [ "x$BITS" == "x" -o "x$BITS" == "x64" ]; then
    touch doc/fasthash64_64.out &&
    ./SMHasher fasthash64 2>&1 | tee doc/fasthash64_64.tmp &&
    mv doc/fasthash64_64.out doc/fasthash64_64.bak &&
    mv doc/fasthash64_64.tmp doc/fasthash64_64.out
fi


#FNV64           	64 bit	(Fowler-Noll-Vo hash, 64-bit)

if [ "x$BITS" == "x" -o "x$BITS" == "x64" ]; then
    touch doc/FNV64_64.out &&
    ./SMHasher FNV64 2>&1 | tee doc/FNV64_64.tmp &&
    mv doc/FNV64_64.out doc/FNV64_64.bak &&
    mv doc/FNV64_64.tmp doc/FNV64_64.out
fi


#metrohash64_1   	64 bit	(MetroHash64_1 for 64-bit)

if [ "x$BITS" == "x" -o "x$BITS" == "x64" ]; then
    touch doc/metrohash64_1_64.out &&
    ./SMHasher metrohash64_1 2>&1 | tee doc/metrohash64_1_64.tmp &&
    mv doc/metrohash64_1_64.out doc/metrohash64_1_64.bak &&
    mv doc/metrohash64_1_64.tmp doc/metrohash64_1_64.out
fi


#metrohash64_2   	64 bit	(MetroHash64_2 for 64-bit)

if [ "x$BITS" == "x" -o "x$BITS" == "x64" ]; then
    touch doc/metrohash64_2_64.out &&
    ./SMHasher metrohash64_2 2>&1 | tee doc/metrohash64_2_64.tmp &&
    mv doc/metrohash64_2_64.out doc/metrohash64_2_64.bak &&
    mv doc/metrohash64_2_64.tmp doc/metrohash64_2_64.out
fi


#metrohash64crc_1	64 bit	(MetroHash64crc_1 for x64)

if [ "x$BITS" == "x" -o "x$BITS" == "x64" ]; then
    touch doc/metrohash64crc_1_64.out &&
    ./SMHasher metrohash64crc_1 2>&1 | tee doc/metrohash64crc_1_64.tmp &&
    mv doc/metrohash64crc_1_64.out doc/metrohash64crc_1_64.bak &&
    mv doc/metrohash64crc_1_64.tmp doc/metrohash64crc_1_64.out
fi


#metrohash64crc_2	64 bit	(MetroHash64crc_2 for x64)

if [ "x$BITS" == "x" -o "x$BITS" == "x64" ]; then
    touch doc/metrohash64crc_2_64.out &&
    ./SMHasher metrohash64crc_2 2>&1 | tee doc/metrohash64crc_2_64.tmp &&
    mv doc/metrohash64crc_2_64.out doc/metrohash64crc_2_64.bak &&
    mv doc/metrohash64crc_2_64.tmp doc/metrohash64crc_2_64.out
fi


#MUM             	64 bit	(github.com/vnmakarov/mum-hash)

if [ "x$BITS" == "x" -o "x$BITS" == "x64" ]; then
    touch doc/MUM_64.out &&
    ./SMHasher MUM 2>&1 | tee doc/MUM_64.tmp &&
    mv doc/MUM_64.out doc/MUM_64.bak &&
    mv doc/MUM_64.tmp doc/MUM_64.out
fi


#Murmur2B        	64 bit	(MurmurHash2 for x64, 64-bit)

if [ "x$BITS" == "x" -o "x$BITS" == "x64" ]; then
    touch doc/Murmur2B_64.out &&
    ./SMHasher Murmur2B 2>&1 | tee doc/Murmur2B_64.tmp &&
    mv doc/Murmur2B_64.out doc/Murmur2B_64.bak &&
    mv doc/Murmur2B_64.tmp doc/Murmur2B_64.out
fi


#Murmur2C        	64 bit	(MurmurHash2 for x86, 64-bit)

if [ "x$BITS" == "x" -o "x$BITS" == "x64" ]; then
    touch doc/Murmur2C_64.out &&
    ./SMHasher Murmur2C 2>&1 | tee doc/Murmur2C_64.tmp &&
    mv doc/Murmur2C_64.out doc/Murmur2C_64.bak &&
    mv doc/Murmur2C_64.tmp doc/Murmur2C_64.out
fi


#NOP_OAAT_read64 	64 bit	(Noop function (only valid for measuring call + OAAT reading overhead))

if [ "x$BITS" == "x" -o "x$BITS" == "x64" ]; then
    touch doc/NOP_OAAT_read64_64.out &&
    ./SMHasher NOP_OAAT_read64 2>&1 | tee doc/NOP_OAAT_read64_64.tmp &&
    mv doc/NOP_OAAT_read64_64.out doc/NOP_OAAT_read64_64.bak &&
    mv doc/NOP_OAAT_read64_64.tmp doc/NOP_OAAT_read64_64.out
fi


#SipHash13       	64 bit	(SipHash 1-3 - SSSE3 optimized)

if [ "x$BITS" == "x" -o "x$BITS" == "x64" ]; then
    touch doc/SipHash13_64.out &&
    ./SMHasher SipHash13 2>&1 | tee doc/SipHash13_64.tmp &&
    mv doc/SipHash13_64.out doc/SipHash13_64.bak &&
    mv doc/SipHash13_64.tmp doc/SipHash13_64.out
fi


#SipHash         	64 bit	(SipHash 2-4 - SSSE3 optimized)

if [ "x$BITS" == "x" -o "x$BITS" == "x64" ]; then
    touch doc/SipHash_64.out &&
    ./SMHasher SipHash 2>&1 | tee doc/SipHash_64.tmp &&
    mv doc/SipHash_64.out doc/SipHash_64.bak &&
    mv doc/SipHash_64.tmp doc/SipHash_64.out
fi


#Spooky64        	64 bit	(Bob Jenkins' SpookyHash, 64-bit result)

if [ "x$BITS" == "x" -o "x$BITS" == "x64" ]; then
    touch doc/Spooky64_64.out &&
    ./SMHasher Spooky64 2>&1 | tee doc/Spooky64_64.tmp &&
    mv doc/Spooky64_64.out doc/Spooky64_64.bak &&
    mv doc/Spooky64_64.tmp doc/Spooky64_64.out
fi


#t1ha_32be       	64 bit	(Fast Positive Hash (portable, best for: 32-bit, big-endian))

if [ "x$BITS" == "x" -o "x$BITS" == "x64" ]; then
    touch doc/t1ha_32be_64.out &&
    ./SMHasher t1ha_32be 2>&1 | tee doc/t1ha_32be_64.tmp &&
    mv doc/t1ha_32be_64.out doc/t1ha_32be_64.bak &&
    mv doc/t1ha_32be_64.tmp doc/t1ha_32be_64.out
fi


#t1ha_32le       	64 bit	(Fast Positive Hash (portable, best for: 32-bit, little-endian))

if [ "x$BITS" == "x" -o "x$BITS" == "x64" ]; then
    touch doc/t1ha_32le_64.out &&
    ./SMHasher t1ha_32le 2>&1 | tee doc/t1ha_32le_64.tmp &&
    mv doc/t1ha_32le_64.out doc/t1ha_32le_64.bak &&
    mv doc/t1ha_32le_64.tmp doc/t1ha_32le_64.out
fi


#t1ha_64be       	64 bit	(Fast Positive Hash (portable, best for: 64-bit, big-endian))

if [ "x$BITS" == "x" -o "x$BITS" == "x64" ]; then
    touch doc/t1ha_64be_64.out &&
    ./SMHasher t1ha_64be 2>&1 | tee doc/t1ha_64be_64.tmp &&
    mv doc/t1ha_64be_64.out doc/t1ha_64be_64.bak &&
    mv doc/t1ha_64be_64.tmp doc/t1ha_64be_64.out
fi


#t1ha            	64 bit	(Fast Positive Hash (portable, best for: 64-bit, little-endian))

if [ "x$BITS" == "x" -o "x$BITS" == "x64" ]; then
    touch doc/t1ha_64.out &&
    ./SMHasher t1ha 2>&1 | tee doc/t1ha_64.tmp &&
    mv doc/t1ha_64.out doc/t1ha_64.bak &&
    mv doc/t1ha_64.tmp doc/t1ha_64.out
fi


#t1ha_aes        	64 bit	(Fast Positive Hash (machine-specific, requires: AES-NI))

if [ "x$BITS" == "x" -o "x$BITS" == "x64" ]; then
    touch doc/t1ha_aes_64.out &&
    ./SMHasher t1ha_aes 2>&1 | tee doc/t1ha_aes_64.tmp &&
    mv doc/t1ha_aes_64.out doc/t1ha_aes_64.bak &&
    mv doc/t1ha_aes_64.tmp doc/t1ha_aes_64.out
fi


#t1ha_crc        	64 bit	(Fast Positive Hash (machine-specific, requires: SSE4.2 CRC32C))

if [ "x$BITS" == "x" -o "x$BITS" == "x64" ]; then
    touch doc/t1ha_crc_64.out &&
    ./SMHasher t1ha_crc 2>&1 | tee doc/t1ha_crc_64.tmp &&
    mv doc/t1ha_crc_64.out doc/t1ha_crc_64.bak &&
    mv doc/t1ha_crc_64.tmp doc/t1ha_crc_64.out
fi


#xxHash64        	64 bit	(xxHash, 64-bit)

if [ "x$BITS" == "x" -o "x$BITS" == "x64" ]; then
    touch doc/xxHash64_64.out &&
    ./SMHasher xxHash64 2>&1 | tee doc/xxHash64_64.tmp &&
    mv doc/xxHash64_64.out doc/xxHash64_64.bak &&
    mv doc/xxHash64_64.tmp doc/xxHash64_64.out
fi


#City128         	128 bit	(Google CityHash128WithSeed)

if [ "x$BITS" == "x" -o "x$BITS" == "x128" ]; then
    touch doc/City128_128.out &&
    ./SMHasher City128 2>&1 | tee doc/City128_128.tmp &&
    mv doc/City128_128.out doc/City128_128.bak &&
    mv doc/City128_128.tmp doc/City128_128.out
fi


#City128         	128 bit	(Google CityHash128WithSeed (old))

if [ "x$BITS" == "x" -o "x$BITS" == "x128" ]; then
    touch doc/City128_128.out &&
    ./SMHasher City128 2>&1 | tee doc/City128_128.tmp &&
    mv doc/City128_128.out doc/City128_128.bak &&
    mv doc/City128_128.tmp doc/City128_128.out
fi


#CityCrc128      	128 bit	(Google CityHashCrc128WithSeed SSE4.2 (old))

if [ "x$BITS" == "x" -o "x$BITS" == "x128" ]; then
    touch doc/CityCrc128_128.out &&
    ./SMHasher CityCrc128 2>&1 | tee doc/CityCrc128_128.tmp &&
    mv doc/CityCrc128_128.out doc/CityCrc128_128.bak &&
    mv doc/CityCrc128_128.tmp doc/CityCrc128_128.out
fi


#FarmHash128     	128 bit	(Google FarmHash128WithSeed)

if [ "x$BITS" == "x" -o "x$BITS" == "x128" ]; then
    touch doc/FarmHash128_128.out &&
    ./SMHasher FarmHash128 2>&1 | tee doc/FarmHash128_128.tmp &&
    mv doc/FarmHash128_128.out doc/FarmHash128_128.bak &&
    mv doc/FarmHash128_128.tmp doc/FarmHash128_128.out
fi


#farmhash128_c   	128 bit	(farmhash128_with_seed (C99))

if [ "x$BITS" == "x" -o "x$BITS" == "x128" ]; then
    touch doc/farmhash128_c_128.out &&
    ./SMHasher farmhash128_c 2>&1 | tee doc/farmhash128_c_128.tmp &&
    mv doc/farmhash128_c_128.out doc/farmhash128_c_128.bak &&
    mv doc/farmhash128_c_128.tmp doc/farmhash128_c_128.out
fi


#metrohash128_1  	128 bit	(MetroHash128_1 for 64-bit)

if [ "x$BITS" == "x" -o "x$BITS" == "x128" ]; then
    touch doc/metrohash128_1_128.out &&
    ./SMHasher metrohash128_1 2>&1 | tee doc/metrohash128_1_128.tmp &&
    mv doc/metrohash128_1_128.out doc/metrohash128_1_128.bak &&
    mv doc/metrohash128_1_128.tmp doc/metrohash128_1_128.out
fi


#metrohash128_2  	128 bit	(MetroHash128_2 for 64-bit)

if [ "x$BITS" == "x" -o "x$BITS" == "x128" ]; then
    touch doc/metrohash128_2_128.out &&
    ./SMHasher metrohash128_2 2>&1 | tee doc/metrohash128_2_128.tmp &&
    mv doc/metrohash128_2_128.out doc/metrohash128_2_128.bak &&
    mv doc/metrohash128_2_128.tmp doc/metrohash128_2_128.out
fi


#metrohash128crc_1	128 bit	(MetroHash128crc_1 for x64)

if [ "x$BITS" == "x" -o "x$BITS" == "x128" ]; then
    touch doc/metrohash128crc_1_128.out &&
    ./SMHasher metrohash128crc_1 2>&1 | tee doc/metrohash128crc_1_128.tmp &&
    mv doc/metrohash128crc_1_128.out doc/metrohash128crc_1_128.bak &&
    mv doc/metrohash128crc_1_128.tmp doc/metrohash128crc_1_128.out
fi


#metrohash128crc_2	128 bit	(MetroHash128crc_2 for x64)

if [ "x$BITS" == "x" -o "x$BITS" == "x128" ]; then
    touch doc/metrohash128crc_2_128.out &&
    ./SMHasher metrohash128crc_2 2>&1 | tee doc/metrohash128crc_2_128.tmp &&
    mv doc/metrohash128crc_2_128.out doc/metrohash128crc_2_128.bak &&
    mv doc/metrohash128crc_2_128.tmp doc/metrohash128crc_2_128.out
fi


#Murmur3C        	128 bit	(MurmurHash3 for x86, 128-bit)

if [ "x$BITS" == "x" -o "x$BITS" == "x128" ]; then
    touch doc/Murmur3C_128.out &&
    ./SMHasher Murmur3C 2>&1 | tee doc/Murmur3C_128.tmp &&
    mv doc/Murmur3C_128.out doc/Murmur3C_128.bak &&
    mv doc/Murmur3C_128.tmp doc/Murmur3C_128.out
fi


#Murmur3F        	128 bit	(MurmurHash3 for x64, 128-bit)

if [ "x$BITS" == "x" -o "x$BITS" == "x128" ]; then
    touch doc/Murmur3F_128.out &&
    ./SMHasher Murmur3F 2>&1 | tee doc/Murmur3F_128.tmp &&
    mv doc/Murmur3F_128.out doc/Murmur3F_128.bak &&
    mv doc/Murmur3F_128.tmp doc/Murmur3F_128.out
fi


#Spooky128       	128 bit	(Bob Jenkins' SpookyHash, 128-bit result)

if [ "x$BITS" == "x" -o "x$BITS" == "x128" ]; then
    touch doc/Spooky128_128.out &&
    ./SMHasher Spooky128 2>&1 | tee doc/Spooky128_128.tmp &&
    mv doc/Spooky128_128.out doc/Spooky128_128.bak &&
    mv doc/Spooky128_128.tmp doc/Spooky128_128.out
fi


#hasshe2         	256 bit	(SSE2 hasshe2, 256-bit)

if [ "x$BITS" == "x" -o "x$BITS" == "x256" ]; then
    touch doc/hasshe2_256.out &&
    ./SMHasher hasshe2 2>&1 | tee doc/hasshe2_256.tmp &&
    mv doc/hasshe2_256.out doc/hasshe2_256.bak &&
    mv doc/hasshe2_256.tmp doc/hasshe2_256.out
fi

