make
rm -f doc/*.tmp
export BITS=

#BeagleHash_32_112 | 112|  32|Yves Orton's hash for 64-bit in 32-bit mode (112-bit seed).

if [ "x$BITS" == "x" -o "x$BITS" == "x32" ]; then
    touch doc/BeagleHash_32_112.32.out &&
    ./SMHasher BeagleHash_32_112 2>&1 | tee doc/BeagleHash_32_112.32.tmp &&
    mv doc/BeagleHash_32_112.32.out doc/BeagleHash_32_112.32.bak &&
    mv doc/BeagleHash_32_112.32.tmp doc/BeagleHash_32_112.32.out
fi


#BeagleHash_32_127 | 127|  32|Yves Orton's hash for 64-bit in 32-bit mode (127-bit seed).

if [ "x$BITS" == "x" -o "x$BITS" == "x32" ]; then
    touch doc/BeagleHash_32_127.32.out &&
    ./SMHasher BeagleHash_32_127 2>&1 | tee doc/BeagleHash_32_127.32.tmp &&
    mv doc/BeagleHash_32_127.32.out doc/BeagleHash_32_127.32.bak &&
    mv doc/BeagleHash_32_127.32.tmp doc/BeagleHash_32_127.32.out
fi


#BeagleHash_32_32  |  32|  32|Yves Orton's hash for 64-bit in 32-bit mode (32-bit seed).

if [ "x$BITS" == "x" -o "x$BITS" == "x32" ]; then
    touch doc/BeagleHash_32_32.32.out &&
    ./SMHasher BeagleHash_32_32 2>&1 | tee doc/BeagleHash_32_32.32.tmp &&
    mv doc/BeagleHash_32_32.32.out doc/BeagleHash_32_32.32.bak &&
    mv doc/BeagleHash_32_32.32.tmp doc/BeagleHash_32_32.32.out
fi


#BeagleHash_32_64  |  64|  32|Yves Orton's hash for 64-bit in 32-bit mode (64-bit seed).

if [ "x$BITS" == "x" -o "x$BITS" == "x32" ]; then
    touch doc/BeagleHash_32_64.32.out &&
    ./SMHasher BeagleHash_32_64 2>&1 | tee doc/BeagleHash_32_64.32.tmp &&
    mv doc/BeagleHash_32_64.32.out doc/BeagleHash_32_64.32.bak &&
    mv doc/BeagleHash_32_64.32.tmp doc/BeagleHash_32_64.32.out
fi


#BeagleHash_32_96  |  96|  32|Yves Orton's hash for 64-bit in 32-bit mode (96-bit seed).

if [ "x$BITS" == "x" -o "x$BITS" == "x32" ]; then
    touch doc/BeagleHash_32_96.32.out &&
    ./SMHasher BeagleHash_32_96 2>&1 | tee doc/BeagleHash_32_96.32.tmp &&
    mv doc/BeagleHash_32_96.32.out doc/BeagleHash_32_96.32.bak &&
    mv doc/BeagleHash_32_96.32.tmp doc/BeagleHash_32_96.32.out
fi


#bernstein         |  32|  32|Bernstein, 32-bit

if [ "x$BITS" == "x" -o "x$BITS" == "x32" ]; then
    touch doc/bernstein.32.out &&
    ./SMHasher bernstein 2>&1 | tee doc/bernstein.32.tmp &&
    mv doc/bernstein.32.out doc/bernstein.32.bak &&
    mv doc/bernstein.32.tmp doc/bernstein.32.out
fi


#City32            |  32|  32|Google CityHash32WithSeed (old)

if [ "x$BITS" == "x" -o "x$BITS" == "x32" ]; then
    touch doc/City32.32.out &&
    ./SMHasher City32 2>&1 | tee doc/City32.32.tmp &&
    mv doc/City32.32.out doc/City32.32.bak &&
    mv doc/City32.32.tmp doc/City32.32.out
fi


#FNV1a             |  32|  32|Fowler-Noll-Vo hash, 32-bit

if [ "x$BITS" == "x" -o "x$BITS" == "x32" ]; then
    touch doc/FNV1a.32.out &&
    ./SMHasher FNV1a 2>&1 | tee doc/FNV1a.32.tmp &&
    mv doc/FNV1a.32.out doc/FNV1a.32.bak &&
    mv doc/FNV1a.32.tmp doc/FNV1a.32.out
fi


#FNV1a_YT          |  32|  32|FNV1a-YoshimitsuTRIAD 32-bit sanmayce

if [ "x$BITS" == "x" -o "x$BITS" == "x32" ]; then
    touch doc/FNV1a_YT.32.out &&
    ./SMHasher FNV1a_YT 2>&1 | tee doc/FNV1a_YT.32.tmp &&
    mv doc/FNV1a_YT.32.out doc/FNV1a_YT.32.bak &&
    mv doc/FNV1a_YT.32.tmp doc/FNV1a_YT.32.out
fi


#GoodOAAT          |  32|  32|Small non-multiplicative OAAT

if [ "x$BITS" == "x" -o "x$BITS" == "x32" ]; then
    touch doc/GoodOAAT.32.out &&
    ./SMHasher GoodOAAT 2>&1 | tee doc/GoodOAAT.32.tmp &&
    mv doc/GoodOAAT.32.out doc/GoodOAAT.32.bak &&
    mv doc/GoodOAAT.32.tmp doc/GoodOAAT.32.out
fi


#HalfSipHash       |  32|  32|HalfSipHash 2-4, 32bit

if [ "x$BITS" == "x" -o "x$BITS" == "x32" ]; then
    touch doc/HalfSipHash.32.out &&
    ./SMHasher HalfSipHash 2>&1 | tee doc/HalfSipHash.32.tmp &&
    mv doc/HalfSipHash.32.out doc/HalfSipHash.32.bak &&
    mv doc/HalfSipHash.32.tmp doc/HalfSipHash.32.out
fi


#JenkinsOOAT       |  32|  32|Bob Jenkins' OOAT as in perl 5.18

if [ "x$BITS" == "x" -o "x$BITS" == "x32" ]; then
    touch doc/JenkinsOOAT.32.out &&
    ./SMHasher JenkinsOOAT 2>&1 | tee doc/JenkinsOOAT.32.tmp &&
    mv doc/JenkinsOOAT.32.out doc/JenkinsOOAT.32.bak &&
    mv doc/JenkinsOOAT.32.tmp doc/JenkinsOOAT.32.out
fi


#JenkinsOOAT_perl  |  32|  32|Bob Jenkins' OOAT as in old perl5

if [ "x$BITS" == "x" -o "x$BITS" == "x32" ]; then
    touch doc/JenkinsOOAT_perl.32.out &&
    ./SMHasher JenkinsOOAT_perl 2>&1 | tee doc/JenkinsOOAT_perl.32.tmp &&
    mv doc/JenkinsOOAT_perl.32.out doc/JenkinsOOAT_perl.32.bak &&
    mv doc/JenkinsOOAT_perl.32.tmp doc/JenkinsOOAT_perl.32.out
fi


#lookup3           |  32|  32|Bob Jenkins' lookup3

if [ "x$BITS" == "x" -o "x$BITS" == "x32" ]; then
    touch doc/lookup3.32.out &&
    ./SMHasher lookup3 2>&1 | tee doc/lookup3.32.tmp &&
    mv doc/lookup3.32.out doc/lookup3.32.bak &&
    mv doc/lookup3.32.tmp doc/lookup3.32.out
fi


#Marvin32          |  32|  32|Marvin32 from MicroSoft

if [ "x$BITS" == "x" -o "x$BITS" == "x32" ]; then
    touch doc/Marvin32.32.out &&
    ./SMHasher Marvin32 2>&1 | tee doc/Marvin32.32.tmp &&
    mv doc/Marvin32.32.out doc/Marvin32.32.bak &&
    mv doc/Marvin32.32.tmp doc/Marvin32.32.out
fi


#MicroOAAT         |  32|  32|Small non-mul OAAT that passes collision checks (by funny-falcon)

if [ "x$BITS" == "x" -o "x$BITS" == "x32" ]; then
    touch doc/MicroOAAT.32.out &&
    ./SMHasher MicroOAAT 2>&1 | tee doc/MicroOAAT.32.tmp &&
    mv doc/MicroOAAT.32.out doc/MicroOAAT.32.bak &&
    mv doc/MicroOAAT.32.tmp doc/MicroOAAT.32.out
fi


#Murmur2           |  32|  32|MurmurHash2 for x86, 32-bit

if [ "x$BITS" == "x" -o "x$BITS" == "x32" ]; then
    touch doc/Murmur2.32.out &&
    ./SMHasher Murmur2 2>&1 | tee doc/Murmur2.32.tmp &&
    mv doc/Murmur2.32.out doc/Murmur2.32.bak &&
    mv doc/Murmur2.32.tmp doc/Murmur2.32.out
fi


#Murmur2A          |  32|  32|MurmurHash2A for x86, 32-bit

if [ "x$BITS" == "x" -o "x$BITS" == "x32" ]; then
    touch doc/Murmur2A.32.out &&
    ./SMHasher Murmur2A 2>&1 | tee doc/Murmur2A.32.tmp &&
    mv doc/Murmur2A.32.out doc/Murmur2A.32.bak &&
    mv doc/Murmur2A.32.tmp doc/Murmur2A.32.out
fi


#Murmur3A          |  32|  32|MurmurHash3 for x86, 32-bit

if [ "x$BITS" == "x" -o "x$BITS" == "x32" ]; then
    touch doc/Murmur3A.32.out &&
    ./SMHasher Murmur3A 2>&1 | tee doc/Murmur3A.32.tmp &&
    mv doc/Murmur3A.32.out doc/Murmur3A.32.bak &&
    mv doc/Murmur3A.32.tmp doc/Murmur3A.32.out
fi


#MurmurOAAT        |  32|  32|Murmur one-at-a-time

if [ "x$BITS" == "x" -o "x$BITS" == "x32" ]; then
    touch doc/MurmurOAAT.32.out &&
    ./SMHasher MurmurOAAT 2>&1 | tee doc/MurmurOAAT.32.tmp &&
    mv doc/MurmurOAAT.32.out doc/MurmurOAAT.32.bak &&
    mv doc/MurmurOAAT.32.tmp doc/MurmurOAAT.32.out
fi


#Phat4             |  96|  32|Yves Orton's 32 bit hash with 96 bit seed

if [ "x$BITS" == "x" -o "x$BITS" == "x32" ]; then
    touch doc/Phat4.32.out &&
    ./SMHasher Phat4 2>&1 | tee doc/Phat4.32.tmp &&
    mv doc/Phat4.32.out doc/Phat4.32.bak &&
    mv doc/Phat4.32.tmp doc/Phat4.32.out
fi


#PMurHash32        |  32|  32|Shane Day's portable-ized MurmurHash3 for x86, 32-bit.

if [ "x$BITS" == "x" -o "x$BITS" == "x32" ]; then
    touch doc/PMurHash32.32.out &&
    ./SMHasher PMurHash32 2>&1 | tee doc/PMurHash32.32.tmp &&
    mv doc/PMurHash32.32.out doc/PMurHash32.32.bak &&
    mv doc/PMurHash32.32.tmp doc/PMurHash32.32.out
fi


#Spooky32          |  32|  32|Bob Jenkins' SpookyHash, 32-bit result

if [ "x$BITS" == "x" -o "x$BITS" == "x32" ]; then
    touch doc/Spooky32.32.out &&
    ./SMHasher Spooky32 2>&1 | tee doc/Spooky32.32.tmp &&
    mv doc/Spooky32.32.out doc/Spooky32.32.bak &&
    mv doc/Spooky32.32.tmp doc/Spooky32.32.out
fi


#superfast         |  32|  32|Paul Hsieh's SuperFastHash

if [ "x$BITS" == "x" -o "x$BITS" == "x32" ]; then
    touch doc/superfast.32.out &&
    ./SMHasher superfast 2>&1 | tee doc/superfast.32.tmp &&
    mv doc/superfast.32.out doc/superfast.32.bak &&
    mv doc/superfast.32.tmp doc/superfast.32.out
fi


#xxHash32          |  32|  32|xxHash, 32-bit for x64

if [ "x$BITS" == "x" -o "x$BITS" == "x32" ]; then
    touch doc/xxHash32.32.out &&
    ./SMHasher xxHash32 2>&1 | tee doc/xxHash32.32.tmp &&
    mv doc/xxHash32.32.out doc/xxHash32.32.bak &&
    mv doc/xxHash32.32.tmp doc/xxHash32.32.out
fi


#Zaphod32          |  95|  32|Yves Orton's 32 bit hash with 95 bit seed

if [ "x$BITS" == "x" -o "x$BITS" == "x32" ]; then
    touch doc/Zaphod32.32.out &&
    ./SMHasher Zaphod32 2>&1 | tee doc/Zaphod32.32.tmp &&
    mv doc/Zaphod32.32.out doc/Zaphod32.32.bak &&
    mv doc/Zaphod32.32.tmp doc/Zaphod32.32.out
fi


#BeagleHash_64_112 | 112|  64|Yves Orton's hash for 64-bit (112 bit seed).

if [ "x$BITS" == "x" -o "x$BITS" == "x64" ]; then
    touch doc/BeagleHash_64_112.64.out &&
    ./SMHasher BeagleHash_64_112 2>&1 | tee doc/BeagleHash_64_112.64.tmp &&
    mv doc/BeagleHash_64_112.64.out doc/BeagleHash_64_112.64.bak &&
    mv doc/BeagleHash_64_112.64.tmp doc/BeagleHash_64_112.64.out
fi


#BeagleHash_64_127 | 127|  64|Yves Orton's hash for 64-bit (127 bit seed).

if [ "x$BITS" == "x" -o "x$BITS" == "x64" ]; then
    touch doc/BeagleHash_64_127.64.out &&
    ./SMHasher BeagleHash_64_127 2>&1 | tee doc/BeagleHash_64_127.64.tmp &&
    mv doc/BeagleHash_64_127.64.out doc/BeagleHash_64_127.64.bak &&
    mv doc/BeagleHash_64_127.64.tmp doc/BeagleHash_64_127.64.out
fi


#BeagleHash_64_32  |  32|  64|Yves Orton's hash for 64-bit. (32 bit seed)

if [ "x$BITS" == "x" -o "x$BITS" == "x64" ]; then
    touch doc/BeagleHash_64_32.64.out &&
    ./SMHasher BeagleHash_64_32 2>&1 | tee doc/BeagleHash_64_32.64.tmp &&
    mv doc/BeagleHash_64_32.64.out doc/BeagleHash_64_32.64.bak &&
    mv doc/BeagleHash_64_32.64.tmp doc/BeagleHash_64_32.64.out
fi


#BeagleHash_64_64  |  64|  64|Yves Orton's hash for 64-bit. (64 bit seed)

if [ "x$BITS" == "x" -o "x$BITS" == "x64" ]; then
    touch doc/BeagleHash_64_64.64.out &&
    ./SMHasher BeagleHash_64_64 2>&1 | tee doc/BeagleHash_64_64.64.tmp &&
    mv doc/BeagleHash_64_64.64.out doc/BeagleHash_64_64.64.bak &&
    mv doc/BeagleHash_64_64.64.tmp doc/BeagleHash_64_64.64.out
fi


#BeagleHash_64_96  |  96|  64|Yves Orton's hash for 64-bit (96 bit seed).

if [ "x$BITS" == "x" -o "x$BITS" == "x64" ]; then
    touch doc/BeagleHash_64_96.64.out &&
    ./SMHasher BeagleHash_64_96 2>&1 | tee doc/BeagleHash_64_96.64.tmp &&
    mv doc/BeagleHash_64_96.64.out doc/BeagleHash_64_96.64.bak &&
    mv doc/BeagleHash_64_96.64.tmp doc/BeagleHash_64_96.64.out
fi


#City64            |  64|  64|Google CityHash64WithSeed (old)

if [ "x$BITS" == "x" -o "x$BITS" == "x64" ]; then
    touch doc/City64.64.out &&
    ./SMHasher City64 2>&1 | tee doc/City64.64.tmp &&
    mv doc/City64.64.out doc/City64.64.bak &&
    mv doc/City64.64.tmp doc/City64.64.out
fi


#FarmHash64        |  32|  64|Google FarmHash64WithSeed

if [ "x$BITS" == "x" -o "x$BITS" == "x64" ]; then
    touch doc/FarmHash64.64.out &&
    ./SMHasher FarmHash64 2>&1 | tee doc/FarmHash64.64.tmp &&
    mv doc/FarmHash64.64.out doc/FarmHash64.64.bak &&
    mv doc/FarmHash64.64.tmp doc/FarmHash64.64.out
fi


#farmhash64_c      |  32|  64|farmhash64_with_seed (C99)

if [ "x$BITS" == "x" -o "x$BITS" == "x64" ]; then
    touch doc/farmhash64_c.64.out &&
    ./SMHasher farmhash64_c 2>&1 | tee doc/farmhash64_c.64.tmp &&
    mv doc/farmhash64_c.64.out doc/farmhash64_c.64.bak &&
    mv doc/farmhash64_c.64.tmp doc/farmhash64_c.64.out
fi


#FNV64             |  32|  64|Fowler-Noll-Vo hash, 64-bit

if [ "x$BITS" == "x" -o "x$BITS" == "x64" ]; then
    touch doc/FNV64.64.out &&
    ./SMHasher FNV64 2>&1 | tee doc/FNV64.64.tmp &&
    mv doc/FNV64.64.out doc/FNV64.64.bak &&
    mv doc/FNV64.64.tmp doc/FNV64.64.out
fi


#metrohash64_1     |  32|  64|MetroHash64_1 for 64-bit

if [ "x$BITS" == "x" -o "x$BITS" == "x64" ]; then
    touch doc/metrohash64_1.64.out &&
    ./SMHasher metrohash64_1 2>&1 | tee doc/metrohash64_1.64.tmp &&
    mv doc/metrohash64_1.64.out doc/metrohash64_1.64.bak &&
    mv doc/metrohash64_1.64.tmp doc/metrohash64_1.64.out
fi


#metrohash64_2     |  32|  64|MetroHash64_2 for 64-bit

if [ "x$BITS" == "x" -o "x$BITS" == "x64" ]; then
    touch doc/metrohash64_2.64.out &&
    ./SMHasher metrohash64_2 2>&1 | tee doc/metrohash64_2.64.tmp &&
    mv doc/metrohash64_2.64.out doc/metrohash64_2.64.bak &&
    mv doc/metrohash64_2.64.tmp doc/metrohash64_2.64.out
fi


#metrohash64crc_1  |  32|  64|MetroHash64crc_1 for x64

if [ "x$BITS" == "x" -o "x$BITS" == "x64" ]; then
    touch doc/metrohash64crc_1.64.out &&
    ./SMHasher metrohash64crc_1 2>&1 | tee doc/metrohash64crc_1.64.tmp &&
    mv doc/metrohash64crc_1.64.out doc/metrohash64crc_1.64.bak &&
    mv doc/metrohash64crc_1.64.tmp doc/metrohash64crc_1.64.out
fi


#metrohash64crc_2  |  32|  64|MetroHash64crc_2 for x64

if [ "x$BITS" == "x" -o "x$BITS" == "x64" ]; then
    touch doc/metrohash64crc_2.64.out &&
    ./SMHasher metrohash64crc_2 2>&1 | tee doc/metrohash64crc_2.64.tmp &&
    mv doc/metrohash64crc_2.64.out doc/metrohash64crc_2.64.bak &&
    mv doc/metrohash64crc_2.64.tmp doc/metrohash64crc_2.64.out
fi


#MUM               |  32|  64|github.com/vnmakarov/mum-hash

if [ "x$BITS" == "x" -o "x$BITS" == "x64" ]; then
    touch doc/MUM.64.out &&
    ./SMHasher MUM 2>&1 | tee doc/MUM.64.tmp &&
    mv doc/MUM.64.out doc/MUM.64.bak &&
    mv doc/MUM.64.tmp doc/MUM.64.out
fi


#Murmur2B          |  32|  64|MurmurHash2 for x64, 64-bit

if [ "x$BITS" == "x" -o "x$BITS" == "x64" ]; then
    touch doc/Murmur2B.64.out &&
    ./SMHasher Murmur2B 2>&1 | tee doc/Murmur2B.64.tmp &&
    mv doc/Murmur2B.64.out doc/Murmur2B.64.bak &&
    mv doc/Murmur2B.64.tmp doc/Murmur2B.64.out
fi


#Murmur2C          |  32|  64|MurmurHash2 for x86, 64-bit

if [ "x$BITS" == "x" -o "x$BITS" == "x64" ]; then
    touch doc/Murmur2C.64.out &&
    ./SMHasher Murmur2C 2>&1 | tee doc/Murmur2C.64.tmp &&
    mv doc/Murmur2C.64.out doc/Murmur2C.64.bak &&
    mv doc/Murmur2C.64.tmp doc/Murmur2C.64.out
fi


#NOP_OAAT_read64   |  32|  64|Noop function (only valid for measuring call + OAAT reading overhead)

if [ "x$BITS" == "x" -o "x$BITS" == "x64" ]; then
    touch doc/NOP_OAAT_read64.64.out &&
    ./SMHasher NOP_OAAT_read64 2>&1 | tee doc/NOP_OAAT_read64.64.tmp &&
    mv doc/NOP_OAAT_read64.64.out doc/NOP_OAAT_read64.64.bak &&
    mv doc/NOP_OAAT_read64.64.tmp doc/NOP_OAAT_read64.64.out
fi


#SBOX              | 128|  64|Yves Orton's 64 bit substitution box hash for up to 32 char strings

if [ "x$BITS" == "x" -o "x$BITS" == "x64" ]; then
    touch doc/SBOX.64.out &&
    ./SMHasher SBOX 2>&1 | tee doc/SBOX.64.tmp &&
    mv doc/SBOX.64.out doc/SBOX.64.bak &&
    mv doc/SBOX.64.tmp doc/SBOX.64.out
fi


#SipHash           | 128|  64|SipHash 2-4

if [ "x$BITS" == "x" -o "x$BITS" == "x64" ]; then
    touch doc/SipHash.64.out &&
    ./SMHasher SipHash 2>&1 | tee doc/SipHash.64.tmp &&
    mv doc/SipHash.64.out doc/SipHash.64.bak &&
    mv doc/SipHash.64.tmp doc/SipHash.64.out
fi


#SipHash13         | 128|  64|SipHash 1-3

if [ "x$BITS" == "x" -o "x$BITS" == "x64" ]; then
    touch doc/SipHash13.64.out &&
    ./SMHasher SipHash13 2>&1 | tee doc/SipHash13.64.tmp &&
    mv doc/SipHash13.64.out doc/SipHash13.64.bak &&
    mv doc/SipHash13.64.tmp doc/SipHash13.64.out
fi


#Spooky64          |  64|  64|Bob Jenkins' SpookyHash, 64-bit result

if [ "x$BITS" == "x" -o "x$BITS" == "x64" ]; then
    touch doc/Spooky64.64.out &&
    ./SMHasher Spooky64 2>&1 | tee doc/Spooky64.64.tmp &&
    mv doc/Spooky64.64.out doc/Spooky64.64.bak &&
    mv doc/Spooky64.64.tmp doc/Spooky64.64.out
fi


#t1ha_32be         |  32|  64|Fast Positive Hash (portable, best for: 32-bit, big-endian)

if [ "x$BITS" == "x" -o "x$BITS" == "x64" ]; then
    touch doc/t1ha_32be.64.out &&
    ./SMHasher t1ha_32be 2>&1 | tee doc/t1ha_32be.64.tmp &&
    mv doc/t1ha_32be.64.out doc/t1ha_32be.64.bak &&
    mv doc/t1ha_32be.64.tmp doc/t1ha_32be.64.out
fi


#t1ha_32le         |  32|  64|Fast Positive Hash (portable, best for: 32-bit, little-endian)

if [ "x$BITS" == "x" -o "x$BITS" == "x64" ]; then
    touch doc/t1ha_32le.64.out &&
    ./SMHasher t1ha_32le 2>&1 | tee doc/t1ha_32le.64.tmp &&
    mv doc/t1ha_32le.64.out doc/t1ha_32le.64.bak &&
    mv doc/t1ha_32le.64.tmp doc/t1ha_32le.64.out
fi


#t1ha              |  64|  64|Fast Positive Hash (portable, best for: 64-bit, little-endian)

if [ "x$BITS" == "x" -o "x$BITS" == "x64" ]; then
    touch doc/t1ha.64.out &&
    ./SMHasher t1ha 2>&1 | tee doc/t1ha.64.tmp &&
    mv doc/t1ha.64.out doc/t1ha.64.bak &&
    mv doc/t1ha.64.tmp doc/t1ha.64.out
fi


#t1ha_64be         |  64|  64|Fast Positive Hash (portable, best for: 64-bit, big-endian)

if [ "x$BITS" == "x" -o "x$BITS" == "x64" ]; then
    touch doc/t1ha_64be.64.out &&
    ./SMHasher t1ha_64be 2>&1 | tee doc/t1ha_64be.64.tmp &&
    mv doc/t1ha_64be.64.out doc/t1ha_64be.64.bak &&
    mv doc/t1ha_64be.64.tmp doc/t1ha_64be.64.out
fi


#t1ha_aes          |  32|  64|Fast Positive Hash (machine-specific, requires: AES-NI)

if [ "x$BITS" == "x" -o "x$BITS" == "x64" ]; then
    touch doc/t1ha_aes.64.out &&
    ./SMHasher t1ha_aes 2>&1 | tee doc/t1ha_aes.64.tmp &&
    mv doc/t1ha_aes.64.out doc/t1ha_aes.64.bak &&
    mv doc/t1ha_aes.64.tmp doc/t1ha_aes.64.out
fi


#t1ha_crc          |  64|  64|Fast Positive Hash (machine-specific, requires: SSE4.2 CRC32C)

if [ "x$BITS" == "x" -o "x$BITS" == "x64" ]; then
    touch doc/t1ha_crc.64.out &&
    ./SMHasher t1ha_crc 2>&1 | tee doc/t1ha_crc.64.tmp &&
    mv doc/t1ha_crc.64.out doc/t1ha_crc.64.bak &&
    mv doc/t1ha_crc.64.tmp doc/t1ha_crc.64.out
fi


#xxHash64          |  32|  64|xxHash, 64-bit

if [ "x$BITS" == "x" -o "x$BITS" == "x64" ]; then
    touch doc/xxHash64.64.out &&
    ./SMHasher xxHash64 2>&1 | tee doc/xxHash64.64.tmp &&
    mv doc/xxHash64.64.out doc/xxHash64.64.bak &&
    mv doc/xxHash64.64.tmp doc/xxHash64.64.out
fi


#Zaphod64          | 191|  64|Yves Orton's 64 bit hash with 191 bit seed

if [ "x$BITS" == "x" -o "x$BITS" == "x64" ]; then
    touch doc/Zaphod64.64.out &&
    ./SMHasher Zaphod64 2>&1 | tee doc/Zaphod64.64.tmp &&
    mv doc/Zaphod64.64.out doc/Zaphod64.64.bak &&
    mv doc/Zaphod64.64.tmp doc/Zaphod64.64.out
fi


#City128           | 128| 128|Google CityHash128WithSeed (old)

if [ "x$BITS" == "x" -o "x$BITS" == "x128" ]; then
    touch doc/City128.128.out &&
    ./SMHasher City128 2>&1 | tee doc/City128.128.tmp &&
    mv doc/City128.128.out doc/City128.128.bak &&
    mv doc/City128.128.tmp doc/City128.128.out
fi


#CityCrc128        | 128| 128|Google CityHashCrc128WithSeed SSE4.2 (old)

if [ "x$BITS" == "x" -o "x$BITS" == "x128" ]; then
    touch doc/CityCrc128.128.out &&
    ./SMHasher CityCrc128 2>&1 | tee doc/CityCrc128.128.tmp &&
    mv doc/CityCrc128.128.out doc/CityCrc128.128.bak &&
    mv doc/CityCrc128.128.tmp doc/CityCrc128.128.out
fi


#FarmHash128       |  32| 128|Google FarmHash128WithSeed

if [ "x$BITS" == "x" -o "x$BITS" == "x128" ]; then
    touch doc/FarmHash128.128.out &&
    ./SMHasher FarmHash128 2>&1 | tee doc/FarmHash128.128.tmp &&
    mv doc/FarmHash128.128.out doc/FarmHash128.128.bak &&
    mv doc/FarmHash128.128.tmp doc/FarmHash128.128.out
fi


#farmhash128_c     |  32| 128|farmhash128_with_seed (C99)

if [ "x$BITS" == "x" -o "x$BITS" == "x128" ]; then
    touch doc/farmhash128_c.128.out &&
    ./SMHasher farmhash128_c 2>&1 | tee doc/farmhash128_c.128.tmp &&
    mv doc/farmhash128_c.128.out doc/farmhash128_c.128.bak &&
    mv doc/farmhash128_c.128.tmp doc/farmhash128_c.128.out
fi


#metrohash128_1    |  32| 128|MetroHash128_1 for 64-bit

if [ "x$BITS" == "x" -o "x$BITS" == "x128" ]; then
    touch doc/metrohash128_1.128.out &&
    ./SMHasher metrohash128_1 2>&1 | tee doc/metrohash128_1.128.tmp &&
    mv doc/metrohash128_1.128.out doc/metrohash128_1.128.bak &&
    mv doc/metrohash128_1.128.tmp doc/metrohash128_1.128.out
fi


#metrohash128_2    |  32| 128|MetroHash128_2 for 64-bit

if [ "x$BITS" == "x" -o "x$BITS" == "x128" ]; then
    touch doc/metrohash128_2.128.out &&
    ./SMHasher metrohash128_2 2>&1 | tee doc/metrohash128_2.128.tmp &&
    mv doc/metrohash128_2.128.out doc/metrohash128_2.128.bak &&
    mv doc/metrohash128_2.128.tmp doc/metrohash128_2.128.out
fi


#metrohash128crc_1 |  32| 128|MetroHash128crc_1 for x64

if [ "x$BITS" == "x" -o "x$BITS" == "x128" ]; then
    touch doc/metrohash128crc_1.128.out &&
    ./SMHasher metrohash128crc_1 2>&1 | tee doc/metrohash128crc_1.128.tmp &&
    mv doc/metrohash128crc_1.128.out doc/metrohash128crc_1.128.bak &&
    mv doc/metrohash128crc_1.128.tmp doc/metrohash128crc_1.128.out
fi


#metrohash128crc_2 |  32| 128|MetroHash128crc_2 for x64

if [ "x$BITS" == "x" -o "x$BITS" == "x128" ]; then
    touch doc/metrohash128crc_2.128.out &&
    ./SMHasher metrohash128crc_2 2>&1 | tee doc/metrohash128crc_2.128.tmp &&
    mv doc/metrohash128crc_2.128.out doc/metrohash128crc_2.128.bak &&
    mv doc/metrohash128crc_2.128.tmp doc/metrohash128crc_2.128.out
fi


#Murmur3C          |  32| 128|MurmurHash3 for x86, 128-bit

if [ "x$BITS" == "x" -o "x$BITS" == "x128" ]; then
    touch doc/Murmur3C.128.out &&
    ./SMHasher Murmur3C 2>&1 | tee doc/Murmur3C.128.tmp &&
    mv doc/Murmur3C.128.out doc/Murmur3C.128.bak &&
    mv doc/Murmur3C.128.tmp doc/Murmur3C.128.out
fi


#Murmur3F          |  32| 128|MurmurHash3 for x64, 128-bit

if [ "x$BITS" == "x" -o "x$BITS" == "x128" ]; then
    touch doc/Murmur3F.128.out &&
    ./SMHasher Murmur3F 2>&1 | tee doc/Murmur3F.128.tmp &&
    mv doc/Murmur3F.128.out doc/Murmur3F.128.bak &&
    mv doc/Murmur3F.128.tmp doc/Murmur3F.128.out
fi


#Spooky128         | 128| 128|Bob Jenkins' SpookyHash, 128-bit result

if [ "x$BITS" == "x" -o "x$BITS" == "x128" ]; then
    touch doc/Spooky128.128.out &&
    ./SMHasher Spooky128 2>&1 | tee doc/Spooky128.128.tmp &&
    mv doc/Spooky128.128.out doc/Spooky128.128.bak &&
    mv doc/Spooky128.128.tmp doc/Spooky128.128.out
fi


#hasshe2           |  32| 256|SSE2 hasshe2, 256-bit

if [ "x$BITS" == "x" -o "x$BITS" == "x256" ]; then
    touch doc/hasshe2.256.out &&
    ./SMHasher hasshe2 2>&1 | tee doc/hasshe2.256.tmp &&
    mv doc/hasshe2.256.out doc/hasshe2.256.bak &&
    mv doc/hasshe2.256.tmp doc/hasshe2.256.out
fi

