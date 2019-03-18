#include "LongNeighborTest.h"
#include "Birthday.h"
#include "Random.h"

#include <cstring>
#include <cmath>
#include <algorithm>
#include <cstdio>
#include <stdint.h>
#include <inttypes.h>

namespace {

  // Change these parameters to vary the thoroughness of the test!

  static const int MINMSGBYTES = 10;
  static const int MAXMSGBYTES = 300;
  static const int BASES_PER_LENGTH = 5;

  static const int MAXRANGE_1DELTA = 1048576;
  static const int MAXRANGE_2DELTA = 2048;
  static const int MAXRANGE_3DELTA = 160;

  static const bool STORE_3BIT_HASHES = true;

  // End of scoping parameters --------------------------------------

  // Increasing this will need extra code in many places!
  static const int MAXFLIPS = 3;

  static const double HORRIBLE = 1e12;
  static const double LENIENCE_THRESHOLD = 0.05;

  static const int LINEWIDTH = 24;
  static const int MAXHASHBITS = 1024;

  struct Delta;
  struct PackedDelta;

  union Hash {
    uint8_t bytes[(MAXHASHBITS+7)/8];
    size_t words[MAXHASHBITS/(8*sizeof(size_t))];
  };

  struct Message {
    uint8_t data[MAXMSGBYTES];
    size_t length;
  };

  static unsigned naiveBinomial(unsigned n, unsigned m) {
    unsigned num = 1;
    unsigned denom = 1;
    if( m > n/2 ) {
      if( m > n )
        return 0;
      else
        m = n-m;
    }
    for( unsigned i=0; i<m; i++ ) {
      num *= n-i ;
      denom *= (i+1) ;
    }
    return num / denom;
  }

  struct Delta {
    unsigned numBitflips;
    unsigned bitflips[MAXFLIPS];
    unsigned zeroesToAppend;

    Delta(): numBitflips(0), zeroesToAppend(0) { }

    // How many deltas are as simple (or simpler) as this?
    unsigned rank() const {
      unsigned maxflipped = 0 ;
      for( unsigned i=0; i<numBitflips; i++ )
        if( bitflips[i] > maxflipped ) maxflipped = bitflips[i];
      unsigned bitrange = maxflipped + 1;
      unsigned count = 1;
      for( unsigned i=1; i<=numBitflips; i++ )
        count += naiveBinomial(bitrange, i);
      return count * (zeroesToAppend + 1);
    }

    bool modify(Message &msg) const {
      if( zeroesToAppend != 0 ) {
        if( msg.length + zeroesToAppend > MAXMSGBYTES ) return false;
        memset(msg.data + msg.length, 0, zeroesToAppend);
        msg.length += zeroesToAppend;
      }
      for( unsigned i=0; i<numBitflips; i++ ) {
        int bitpos = msg.length*8 - 1 - bitflips[i];
        if( bitpos < 0 ) return false;
        // Counts bits in big-endian so bits that are farther to
        // the right in a hex dump will count as more surprising:
        msg.data[bitpos >> 3] ^= 128 >> (bitpos & 7);
      }
      return true;
    }

    void print() const {
      for( unsigned i=0; i<numBitflips; i++ )
        printf("%c%u", i ? ',' : '(', bitflips[i]);
      printf(")+%u", zeroesToAppend);
    }

  };

  class PackedDelta {
    typedef uint32_t codeword_t ;
    static const int CW_BITS = 8*sizeof(codeword_t);
    static const int TAG_BITS = 2;
    static const int BITS_2DELTA = (CW_BITS-TAG_BITS)/2;
    static const int BITS_3DELTA = (CW_BITS-TAG_BITS)/3;

    static const codeword_t NULL_DELTA = -(codeword_t)1;

    codeword_t codeword ;

    static codeword_t pack2(unsigned tag, codeword_t v1, codeword_t v2) {
      if( v1 >= (1u<<BITS_2DELTA) || v2 >= (1u<<BITS_2DELTA) )
        return 0;
      else
        return tag + (v1 << TAG_BITS) + (v2 << (TAG_BITS+BITS_2DELTA));
    }

    static codeword_t pack3(unsigned tag, codeword_t v1, codeword_t v2, codeword_t v3) {
      if( v1 >= (1u<<BITS_3DELTA) || v2 >= (1u<<BITS_3DELTA) || v3 >= (1u<<BITS_3DELTA) )
        return 0;
      else
        return tag + (v1 << TAG_BITS) +
          (v2 << (TAG_BITS+BITS_3DELTA)) +
          (v3 << (TAG_BITS+2*BITS_3DELTA));
    }

  public:

    struct SanityCheck {
      SanityCheck() {
        if( 2 + 2*BITS_2DELTA > 8*sizeof(codeword_t) )
          fprintf(stderr, "2delta range is too large to pack!\n");
        if( 2 + 3*BITS_3DELTA > 8*sizeof(codeword_t) )
          fprintf(stderr, "3delte range is too large to pack!\n");
        if( MAXRANGE_2DELTA > 1 << BITS_2DELTA )
          fprintf(stderr, "Not enough bits allocated to 2delta!\n");
        if( MAXRANGE_3DELTA > 1 << BITS_3DELTA )
          fprintf(stderr, "Not enough bits allocated to 3delta!\n");
      }
    };

    // zeroing out the backing memory _must_ produce an invalid PackedDelta
    // since we use that to clear the hash table
    operator bool() {
      return codeword != 0;
    }

    PackedDelta(): codeword(0) {}

    PackedDelta(const Delta &v) {
      if( v.zeroesToAppend == 0 ) {
        switch( v.numBitflips ) {
        case 0:
          codeword = NULL_DELTA;
          break;
        case 1:
          codeword = 4 + (v.bitflips[0] << 3);
          break;
        case 2:
          codeword = pack2(1, v.bitflips[0], v.bitflips[1]);
          break;
        case 3:
          codeword = pack3(2, v.bitflips[0], v.bitflips[1], v.bitflips[2]);
          break ;
        default:
          codeword = 0;
          break ;
        }
      } else if( v.numBitflips == 2 ) {
        codeword = pack3(3, v.bitflips[0], v.bitflips[1], v.zeroesToAppend);
      } else
        codeword = 0;
    }

    operator Delta() const {
      Delta out;
      if( codeword == NULL_DELTA )
        return out;
      unsigned width = (codeword & 3) == 1 ? BITS_2DELTA : BITS_3DELTA;
      unsigned tag = codeword & 3;
      switch( tag ) {
      case 0:
        out.numBitflips = 1;
        out.bitflips[0] = codeword >> 3;
        return out;
        break;
      case 1:
        width = BITS_2DELTA;
        out.numBitflips = 2;
        break ;
      case 2:
        width = BITS_3DELTA;
        out.numBitflips = 3;
        break ;
      case 3:
        width = BITS_3DELTA;
        out.numBitflips = 2;
        break;
      }
      unsigned mask = ~(-1 << width);
      for( int i=0; i<MAXFLIPS; i++ )
        out.bitflips[i] = (codeword >> (2 + i*width)) & mask ;
      if( tag == 3 )
        out.zeroesToAppend = out.bitflips[2];
      return out;
    }

  };

  static PackedDelta::SanityCheck runTheCheck;

  class TestLongNeighbors {

    const pfHash hashfunc;
    const unsigned hashbits;
    const size_t hashbytes;

    size_t hashCapacity;
    PackedDelta* hashTable;
    uint8_t* fullHashes;

    unsigned const genRange1;
    unsigned const genRange2;
    unsigned const genRange3;

    unsigned basesTested;
    double expectedBads;
    unsigned actualBads;

    Message worstBase;
    Delta worstD1, worstD2;
    double worstSurprise;

    int horribleCollisionsFound;
    int horribleCollisionsShown;

    Hash curhash;
    size_t addrmask;
    size_t *addrptr;
    size_t *skipptr;
    size_t duphashto;

    bool foundCollisionYet;
    size_t hashesStored;
    uint64_t hashtabLookups;
    uint64_t hashtabSkips;

    static size_t makeRange(size_t max, double bits) {
      if( max > 8*MAXMSGBYTES )
        max = 8*MAXMSGBYTES;
      if( bits >= 30 )
        return max ;
      else
        return std::min(max, (size_t)exp2(bits));
    }

    void clearHashtable(size_t need) {
      need *= 3;
      if( hashCapacity < need ) {
        do hashCapacity <<= 1; while( hashCapacity < need );
        delete[] hashTable;
        hashTable = new PackedDelta[hashCapacity];
        delete[] fullHashes;
        fullHashes = new uint8_t[hashCapacity*hashbytes];
      }
      size_t capacityUsed = hashCapacity ;
      while( (capacityUsed>>1) > need )
        capacityUsed >>= 1;
      memset(hashTable, 0, sizeof(PackedDelta)*capacityUsed);
      addrmask = capacityUsed-1;
      foundCollisionYet = false;
      hashesStored = 0;
      hashtabLookups = 0;
      hashtabSkips = 0;

      if( hashbytes >= 2*sizeof(size_t) ) {
        addrptr = &curhash.words[1];
        skipptr = &curhash.words[0];
        duphashto = 0;
      } else {
        static size_t ONE = 1;
        addrptr = &curhash.words[0];
        skipptr = &ONE ;
        duphashto = std::max(hashbytes, sizeof(size_t)-hashbytes);
      }
    }

    void justHash(Message const &m, Hash &h) {
      hashfunc(m.data, m.length, 0, h.bytes);
    }

    static void showBytesWithComparison(uint8_t const *a, size_t alen, uint8_t const *b, size_t blen) {
      if( alen == blen )
        printf(" length %zu bytes:\n", alen);
      else if( alen > blen )
        printf(" lengths %zu and %zu bytes:\n", alen, blen);
      else {
        printf(" lengths %zu and %zu bytes (WHOOPS, last %zu bytes not shown!):\n", alen, blen, blen-alen);
        blen = alen ;
      }
      for( size_t offset = 0; offset < alen; offset += LINEWIDTH ) {
        printf("%03zx:", offset);
        size_t end = std::min(alen, offset + LINEWIDTH);
        for( size_t i=offset; i<end; i++ )
          printf(" %02X", a[i]);
        printf("\n");
        int moveright = 5;
        for( size_t i=offset; i<end; i++ ) {
          if( (i < blen && a[i] == b[i]) || i > blen ) {
            moveright += 3;
          } else {
            if( moveright )
              printf("%*s", moveright, "^");
            moveright = 0;
            if( i < blen ) {
              printf("%02X^", b[i]);
            } else {
              printf("eof");
              break ;
            }
          }
        }
        if( moveright != 5 + 3*(int)(end-offset) )
          printf("\n");
      }
    }

    void showCollision(const char* what, Message const &base, Delta d1, Delta d2, double surprise) {
      Message m1 = base, m2 = base;
      if( d1.zeroesToAppend >= d2.zeroesToAppend )
        d1.modify(m1), d2.modify(m2);
      else
        d2.modify(m1), d1.modify(m2);
      printf("%s has surprise score %g and", what, surprise);
      showBytesWithComparison(m1.data, m1.length, m2.data, m2.length);
      Hash h1, h2 ;
      justHash(m1, h1);
      justHash(m2, h2);
      printf("The hashes are");
      showBytesWithComparison(h1.bytes, hashbytes, h2.bytes, hashbytes);
    }

    // Compute collision probability _relative to a particular base_
    double actualCollisionProbability(Delta const &d1, Delta const &d2, bool remember) {
      unsigned rank1 = d1.rank();
      unsigned rank2 = d2.rank();
      if( remember || rank2 <= rank1 )
        return Birthday::collisionProbability(hashbits, std::max(rank1, rank2), 0);
      else
        return Birthday::collisionProbability(hashbits, rank1, rank2-rank1);
    }

    // return true if a collision is found
    bool hashAndTest(Message const &base, Delta const &delta, bool remember) {
      if( foundCollisionYet ) {
        hashtabLookups++ ;
        if( remember && PackedDelta(delta) )
          hashesStored++ ;
        return false;
      }

      Message msg = base ;
      if( !delta.modify(msg) ) {
        fprintf(stderr, "Message modification failed!\n");
        return false;
      }

      justHash(msg, curhash);
      if( duphashto )
        memcpy(curhash.bytes + duphashto, curhash.bytes, hashbytes);

      hashtabLookups++ ;
      size_t addr = *addrptr & addrmask ;
      while( hashTable[addr] ) {
        if( 0 == memcmp(curhash.bytes, fullHashes + addr*hashbytes, hashbytes) ) {
          foundCollisionYet = true;
          Delta prevdelta = hashTable[addr];
          double probability = actualCollisionProbability(prevdelta, delta, remember);
          // There probability now is for each length of message. What we're really interested
          // in is the probability that there's a bad delta of this rarity in a message of this
          // _or shorter_ length, so pretend we have that many Poisson trials
          probability = -expm1(base.length * log1p(-probability));
          double surprise = 1/probability - 1;
          if( prevdelta.zeroesToAppend || delta.zeroesToAppend ) {
            // A plain delta without length changes is _generally_ more striking.
            surprise /= 10;
          }
          bool isWorse = false;
          if( surprise > worstSurprise ) {
            worstBase = base;
            worstD1 = prevdelta;
            worstD2 = delta;
            worstSurprise = surprise;
            isWorse = true;
          }
          if( surprise >= HORRIBLE ) {
            horribleCollisionsFound++;
            if( horribleCollisionsShown < 3 || isWorse ) {
              showCollision("\nThis horrible collision", base, prevdelta, delta, surprise);
              horribleCollisionsShown++;
            }
          }
          actualBads++;
          return true;
        }
        hashtabSkips++ ;
        addr = (addr + (*skipptr | 1)) & addrmask;
      }
      // The hash was previously unknown
      if( remember ) {
        PackedDelta packed = delta;
        if( !packed ) {
          fprintf(stderr, "Oops! We met an unpackable delta\n");
        } else {
          hashesStored++ ;
          hashTable[addr] = delta;
          memcpy(fullHashes + addr*hashbytes, curhash.bytes, hashbytes);
        }
      }
      return false;
    }

    void testOneBase(Message const &base) {
      basesTested++;
      unsigned myRange1 = std::min((unsigned)(base.length*8), genRange1);
      unsigned myRange2 = std::min((unsigned)(base.length*8), genRange2);
      unsigned myRange3 = std::min((unsigned)(base.length*8), genRange3);

      unsigned numAppendExps = 0;
      while( numAppendExps < 5 && base.length + (1<<numAppendExps) <= MAXMSGBYTES )
        numAppendExps++ ;

      unsigned expectedStores = 1 + myRange1 + naiveBinomial(myRange2, 2);
      if( STORE_3BIT_HASHES )
        expectedStores += naiveBinomial(myRange3, 3) + numAppendExps * naiveBinomial(myRange3, 2);
      clearHashtable(expectedStores);

      Delta delta;
      unsigned &f1 = delta.bitflips[0];
      unsigned &f2 = delta.bitflips[1];
      unsigned &f3 = delta.bitflips[2];

      hashAndTest(base, delta, true);

      delta.numBitflips = 1;
      for( f1=0; f1 < myRange1; f1++ )
        hashAndTest(base, delta, true);

      delta.numBitflips = 2;
      for( f1=0; f1 < myRange2; f1++ )
        for( f2=0; f2 < f1; f2++ )
          hashAndTest(base, delta, true);

      delta.numBitflips = 3;
      for( f1=0; f1 < myRange3; f1++ )
        for( f2=0; f2 < f1; f2++ ) {
          delta.numBitflips = 3;
          delta.zeroesToAppend = 0;
          for( f3=0; f3 < f2; f3++ )
            hashAndTest(base, delta, STORE_3BIT_HASHES);
          delta.numBitflips = 2;
          for( unsigned log=0; log < numAppendExps; log++ ) {
            delta.zeroesToAppend = 1 << log;
            hashAndTest(base, delta, STORE_3BIT_HASHES);
          }
        }

      expectedBads += Birthday::collisionProbability(hashbits, hashesStored, hashtabLookups-hashesStored);
    }

    // return true for success
    bool testAllTheBases() {
      Rand random;
      Message base;
      bool passing ;
      for( base.length=MINMSGBYTES; base.length <= MAXMSGBYTES; base.length++ ) {
        random.reseed((uint64_t)base.length);

        for( int i=0; i<BASES_PER_LENGTH; i++ ) {
          if( i == 0 )
            memset(base.data, 0, base.length);
          else if( i == 1 )
            memset(base.data, -1, base.length);
          else
            random.rand_p(base.data, base.length);
          testOneBase(base);
        }

        unsigned threshold ;
        if( expectedBads < LENIENCE_THRESHOLD )
          threshold = 2;
        else
          threshold = (unsigned)(expectedBads * 1.3 + 4);
        passing = worstSurprise < HORRIBLE && actualBads < threshold;
        if( !passing && base.length >= worstBase.length + 20 )
          break;

        if( base.length % 16 == 0 || base.length == MAXMSGBYTES )
          fprintf(stderr, "[%zu]", base.length);
        else
          fprintf(stderr, ".");
      }
      fprintf(stderr, "\n");
      return passing;
    }

  public:

    TestLongNeighbors(pfHash hashfunc, unsigned hashbits):
      hashfunc(hashfunc),
      hashbits(hashbits),
      hashbytes(hashbits/8),
      hashCapacity(1024),
      hashTable(new PackedDelta[1024]),
      fullHashes(new uint8_t[1024*hashbits/8]),
      genRange1(makeRange(MAXRANGE_1DELTA, hashbits/2.0)),
      genRange2(makeRange(MAXRANGE_2DELTA, hashbits/4.0)),
      genRange3(makeRange(MAXRANGE_3DELTA, hashbits/6.0))
    { }

    ~TestLongNeighbors() {
      delete[] hashTable;
      delete[] fullHashes;
    }

    bool run() {
      worstSurprise = -1;
      basesTested = 0;
      expectedBads = 0;
      actualBads = 0;
      horribleCollisionsFound = 0;
      horribleCollisionsShown = 0;
      if( genRange1 > genRange2 )
        printf("Looking for 2-bit collisions in the last %u bits.\n", genRange1);
      printf("Looking for 4-bit collisions in the last %u bits.\n", genRange2);
      if( genRange3 != 0 )
        printf("Looking for %d-bit collisions in the last %u bits.\n",
               STORE_3BIT_HASHES ? 6 : 5, genRange3);
      printf("Trying bases of length %d to %d, %d of each length.\n",
             MINMSGBYTES, MAXMSGBYTES, BASES_PER_LENGTH);

      bool pass = testAllTheBases();
      printf("\nAmong %u tested bases, we expected %g bads. Actually there were %u.\n",
             basesTested, expectedBads, actualBads);

      if( worstSurprise >= 0 && horribleCollisionsShown == 0 )
        showCollision("The most striking collision", worstBase, worstD1, worstD2, worstSurprise);
      return pass;
    }

  };

}

bool testLongNeighbors(pfHash hashFunc, int hashbits) {
  if( hashbits % 8 != 0 ) {
    printf("(Omitting because a %d-bit has doe not fit into a whole number of bytes.)\n", hashbits);
    return true;
  }
  TestLongNeighbors tester(hashFunc, hashbits);
  return tester.run();
}

