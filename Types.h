#pragma once

#include "Platform.h"
#include "Bitvec.h"

#include <memory.h>
#include <vector>
#include <map>
#include <set>
#include <stdio.h>
#include "Blob.h"
//-----------------------------------------------------------------------------
// If the optimizer detects that a value in a speed test is constant or unused,
// the optimizer may remove references to it or otherwise create code that
// would not occur in a real-world application. To prevent the optimizer from
// doing this we declare two trivial functions that either sink or source data,
// and bar the compiler from optimizing them.

void     blackhole ( uint32_t x );
uint32_t whitehole ( void );

extern uint32_t g_verbose;
extern double g_confidence;

//-----------------------------------------------------------------------------
// We want to verify that every test produces the same result on every platform
// To do this, we hash the results of every test to produce an overall
// verification value for the whole test suite. If two runs produce the same
// verification value, then every test in both run produced the same results

extern uint32_t g_verify;

// Mix the given blob of data into the verification code

void MixVCode ( const void * blob, int len );


//-----------------------------------------------------------------------------

typedef void (*pfSeedState) ( const int seedbits, const void *seed, void *state );
typedef void (*pfHashWithState) ( const void *blob, const int len, const void *state, void *out );

typedef struct HashInfo
{
  const char * name;
  const char * desc;
  int seedbits;
  int statebits;
  int hashbits;
  uint32_t verification;
  pfSeedState seed_state;
  pfHashWithState hash_with_state;
} HashInfo;


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



//-----------------------------------------------------------------------------
/* vim: set sts=2 sw=2 et: */
