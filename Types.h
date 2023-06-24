#pragma once

#include "Platform.h"
#include "Bitvec.h"

#include <memory.h>
#include <vector>
#include <map>
#include <set>
#include <string>
#include <iostream>
#include <cstdint>
using namespace std;
#include <assert.h>

//-----------------------------------------------------------------------------
// If the optimizer detects that a value in a speed test is constant or unused,
// the optimizer may remove references to it or otherwise create code that
// would not occur in a real-world application. To prevent the optimizer from
// doing this we declare two trivial functions that either sink or source data,
// and bar the compiler from optimizing them.

void     blackhole ( uint32_t x );
uint32_t whitehole ( void );

static inline uint8_t bitrev(uint8_t b)
{
  static const unsigned char revbits[16] =
    {
     0x0, 0x8, 0x4, 0xc, 0x2, 0xa, 0x6, 0xe,
     0x1, 0x9, 0x5, 0xd, 0x3, 0xb, 0x7, 0xf
    };
  return (revbits[ b & 0b1111 ] << 4) | revbits[ b >> 4 ];
}

//-----------------------------------------------------------------------------
// We want to verify that every test produces the same result on every platform
// To do this, we hash the results of every test to produce an overall
// verification value for the whole test suite. If two runs produce the same
// verification value, then every test in both run produced the same results

extern uint32_t g_verify;

// Mix the given blob of data into the verification code

void MixVCode ( const void * blob, int len );


//-----------------------------------------------------------------------------
typedef void (*pfHash)(const void *blob, const int len, const uint32_t seed,
                       void *out);

enum HashQuality             {  SKIP,   POOR,   GOOD };
struct HashInfo
{
  pfHash hash;
  int hashbits;
  uint32_t verification;
  const char * name;
  const char * desc;
  enum HashQuality quality;
  const std::vector<uint64_t> secrets;
};

struct ByteVec : public std::vector<uint8_t>
{
  ByteVec ( const void * key, int len )
  {
    resize(len);
    memcpy(&front(),key,len);
  }
};

template< typename hashtype, typename keytype >
struct CollisionMap : public std::map< hashtype, std::vector<keytype> >
{
};

template< typename hashtype >
struct HashSet : public std::set<hashtype>
{
};

//-----------------------------------------------------------------------------

template < class T >
class hashfunc
{
public:

  hashfunc ( pfHash h ) : m_hash(h)
  {
  }

  inline void operator () ( const void * key, const int len, const uint32_t seed, uint32_t * out )
  {
    m_hash(key,len,seed,out);
  }

  inline operator pfHash ( void ) const
  {
    return m_hash;
  }

  inline T operator () ( const void * key, const int len, const uint32_t seed )
  {
    T result;
    m_hash(key,len,seed,(unsigned*)&result);
    return result;
  }
  inline T operator () ( const void * key, const int len, const uint64_t seed )
  {
    T result;
    m_hash(key,len,seed,(unsigned*)&result);
    return result;
  }

  pfHash m_hash;
};

// hash_combine. The magic number 0x9e3779b9 is derived from the inverse golden ratio.
// phi = (1+sqrt(5))/2; 2^32 / phi => 2654435769.497230
template <typename T>
inline void hash_combine (std::uint16_t& seed, const T& val)
{
    seed ^= std::hash<T>{}(val) + 0x9e37U + (seed<<3) + (seed>>1);
}

template <typename T>
inline void hash_combine (std::uint32_t& seed, const T& val)
{
    seed ^= std::hash<T>{}(val) + 0x9e3779b9U + (seed<<6) + (seed>>2);
}

template <typename T>
inline void hash_combine (std::uint64_t& seed, const T& val)
{
    seed ^= std::hash<T>{}(val) + 0x9e3779b97f4a7c15LLU + (seed<<12) + (seed>>4);
}

//-----------------------------------------------------------------------------
// Key-processing callback objects. Simplifies keyset testing a bit.

struct KeyCallback
{
  KeyCallback() : m_count(0)
  {
  }

  virtual ~KeyCallback()
  {
  }

  virtual void operator() ( const void * key, int len )
  {
    m_count++;
  }

  virtual void reserve ( int keycount )
  {
  };

  int m_count;
};

//----------

static void printKey(const void* key, size_t len);

template<typename hashtype>
struct HashCallback : public KeyCallback
{
  typedef std::vector<hashtype> hashvec;

  HashCallback ( pfHash hash, hashvec & hashes ) : m_hashes(hashes), m_pfHash(hash)
  {
    m_hashes.clear();
  }

  virtual void operator () ( const void * key, int len )
  {
    size_t newsize = m_hashes.size() + 1;

    m_hashes.resize(newsize);

    hashtype h;
    m_pfHash(key, len, 0, &h);

    m_hashes.back() = h;
  }

  virtual void reserve ( int keycount )
  {
    m_hashes.reserve(keycount);
  }

  hashvec & m_hashes;
  pfHash m_pfHash;

  //----------

private:

  HashCallback & operator = ( const HashCallback & );
};

//----------

template<typename hashtype>
struct CollisionCallback : public KeyCallback
{
  typedef HashSet<hashtype> hashset;
  typedef CollisionMap<hashtype,ByteVec> collmap;

  CollisionCallback ( pfHash hash, hashset & collisions, collmap & cmap )
  : m_pfHash(hash),
    m_collisions(collisions),
    m_collmap(cmap)
  {
  }

  virtual void operator () ( const void * key, int len )
  {
    hashtype h;

    m_pfHash(key,len,0,&h);

    if(m_collisions.count(h))
    {
      m_collmap[h].push_back( ByteVec(key,len) );
    }
  }

  //----------

  pfHash m_pfHash;
  hashset & m_collisions;
  collmap & m_collmap;

private:

  CollisionCallback & operator = ( const CollisionCallback & c );
};

//-----------------------------------------------------------------------------

template <int _bits>
class Blob
{
public:
  Blob ()
  {
    for (size_t i = 0; i < sizeof (bytes); i++)
      {
        bytes[i] = 0;
      }
  }

  Blob (int x)
  {
    for (size_t i = 0; i < sizeof (bytes); i++)
      {
        bytes[i] = 0;
      }
    *(int *)bytes = x;
  }
  Blob (unsigned long long x) { *(unsigned long long *)bytes = x; }
  Blob (unsigned long x) { *(unsigned long *)bytes = x; }

  Blob (const Blob &k)
  {
    for (size_t i = 0; i < sizeof (bytes); i++)
      {
        bytes[i] = k.bytes[i];
      }
  }

  Blob &
  operator= (const Blob &k)
  {
    for (size_t i = 0; i < sizeof (bytes); i++)
      {
        bytes[i] = k.bytes[i];
      }

    return *this;
  }

  Blob (uint64_t a, uint64_t b)
  {
    uint64_t t[2] = { a, b };
    set (&t, 16);
  }

  void
  set (const void *blob, size_t len)
  {
    const uint8_t *k = (const uint8_t *)blob;

    len = len > sizeof (bytes) ? sizeof (bytes) : len;

    for (size_t i = 0; i < len; i++)
      {
        bytes[i] = k[i];
      }

    for (size_t i = len; i < sizeof (bytes); i++)
      {
        bytes[i] = 0;
      }
  }

  uint8_t &
  operator[] (int i)
  {
    return bytes[i];
  }

  const uint8_t &
  operator[] (int i) const
  {
    return bytes[i];
  }

  //----------
  // boolean operations

  bool
  operator<(const Blob &k) const
  {
    for (int i = sizeof (bytes) - 1; i >= 0; i--)
      {
        if (bytes[i] < k.bytes[i])
          return true;
        if (bytes[i] > k.bytes[i])
          return false;
      }

    return false;
  }

  bool
  operator== (const Blob &k) const
  {
    for (size_t i = 0; i < sizeof (bytes); i++)
      {
        if (bytes[i] != k.bytes[i])
          return false;
      }

    return true;
  }

  bool
  operator!= (const Blob &k) const
  {
    return !(*this == k);
  }

  //----------
  // bitwise operations

  Blob
  operator^ (const Blob &k) const
  {
    Blob t;

    for (size_t i = 0; i < sizeof (bytes); i++)
      {
        t.bytes[i] = bytes[i] ^ k.bytes[i];
      }

    return t;
  }

  Blob &
  operator^= (const Blob &k)
  {
    for (size_t i = 0; i < sizeof (bytes); i++)
      {
        bytes[i] ^= k.bytes[i];
      }
    return *this;
  }

  int
  operator& (int x)
  {
    return (*(int *)bytes) & x;
  }
  int
  operator| (int x)
  {
    return (*(int *)bytes) | x;
  }

  Blob &
  operator|= (const Blob &k)
  {
    for (size_t i = 0; i < sizeof (bytes); i++)
      {
        bytes[i] |= k.bytes[i];
      }
    return *this;
  }
  Blob &
  operator|= (uint8_t k)
  {
    bytes[0] |= k;
    return *this;
  }

  Blob &
  operator&= (const Blob &k)
  {
    for (size_t i = 0; i < sizeof (bytes); i++)
      {
        bytes[i] &= k.bytes[i];
      }
    return *this;
  }

  Blob
  operator<< (int c)
  {
    Blob t = *this;

    lshift (&t.bytes[0], sizeof (bytes), c);

    return t;
  }

  Blob
  operator>> (int c)
  {
    Blob t = *this;

    rshift (&t.bytes[0], sizeof (bytes), c);

    return t;
  }

  Blob &
  operator<<= (int c)
  {
    lshift (&bytes[0], sizeof (bytes), c);

    return *this;
  }

  Blob &
  operator>>= (int c)
  {
    rshift (&bytes[0], sizeof (bytes), c);

    return *this;
  }

  Blob &
  bitreverse (void)
  {
    assert (_bits % 8 == 0);
    const int j = _bits / 8;
    for (int i = 0; i < j; i++)
      {
        bytes[j - i] = bitrev (bytes[i]);
      }
    return *this;
  }

  friend ostream&
  operator<< (ostream& out, const Blob<_bits>& t)
  {
    out << "0x" << hex;
    for (size_t i = 0; i < _bits/8; i++)
      out << t.bytes[i];
    out << dec;
    return out;
  }
  
  //----------

private:
  uint8_t bytes[(_bits + 7) / 8];
};

typedef Blob<128> uint128_t;
typedef Blob<256> uint256_t;

//-----------------------------------------------------------------------------
