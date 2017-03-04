#pragma once
//-----------------------------------------------------------------------------
#include "Types.h"
#include "Random.h"

template < class T >
class hashfunc
{
public:

  pfHash m_hash;
  pfHashWithState m_hash_with_state;
  pfSeedState m_seed_state;
  int m_seedbits;
  int m_statebits;
  std::vector<uint8_t> m_state;
  std::vector<uint8_t> m_seed; /* if we want to track where the state comes from */
  const char *m_name;

  hashfunc (
      pfHash hash, pfSeedState seed_state, pfHashWithState hash_with_state,
      int seedbits, int statebits, const char *name
  ) : m_hash(hash),
    m_seed_state(seed_state),
    m_hash_with_state(hash_with_state),
    m_seedbits(seedbits),
    m_statebits(statebits),
    m_name(name)
  {
    if (0) printf("initializing hash object for new interface m_hash: %p m_hash_with_state:%p\n",
        m_hash, m_hash_with_state);
    m_state.resize((statebits+7)/8);
    m_seed.resize((seedbits+7)/8);
  }

  inline operator pfHash ( void ) const
  {
    return m_hash;
  }

  inline operator pfSeedState ( void ) const
  {
    return m_seed_state;
  }

  inline operator pfHashWithState ( void ) const
  {
    return m_hash_with_state;
  }

  inline T operator () ( const void * key, const int len )
  {
    T result;

    m_hash_with_state(key,len,&m_state[0],&result);

    return result;
  }

  inline void operator () ( const void * key, const int len, void * out )
  {
    if (m_hash_with_state) {
      m_hash_with_state(key,len,&m_state[0],out);
    } else if (m_seedbits == 32) {
      m_hash(key,len,*((uint32_t*)&m_seed[0]),out);
    }
  }

  inline T operator () ( const void * key, const int len, const uint32_t seed )
  {
    T result;

    m_hash(key,len,seed,&result);

    return result;
  }

  inline void operator () ( const void * key, const int len, const void *seed, void *out )
  {
    if (m_hash_with_state) {
      _seed_state(m_seedbits,seed,&m_state[0]);
      m_hash_with_state(key,len,&m_state[0],out);
    } else if (m_statebits == 32 && m_seedbits == 32) {
      m_hash(key,len,*((uint32_t *)seed),out);
    } else {
      printf("Error! No hash_with_state for hash\n");
      exit(1);
    }
  }

  inline void operator () ( const void * key, const int len, const uint32_t seed, void * out )
  {
    m_hash(key,len,seed,out);
  }

  inline void _seed_state ( const int seedbits, const void *seed, void *state )
  {
    if (m_seed_state) {
      m_seed_state(seedbits,seed,state);
    } else {
      memcpy( state, seed, sizeof(m_seed) );
    }
    if (state == &m_state[0]) {
      memcpy( &m_seed[0], seed, sizeof(m_seed) );
    }
  }

  inline void seed_state ( const void *seed )
  {
    _seed_state(m_seedbits, seed, &m_state[0]);
  }

  inline void seed_state ( const int seedbits, const void *seed )
  {
    _seed_state(seedbits, seed, &m_state[0]);
  }

  inline void seed_state ( const int seedbits, const void *seed, void *state )
  {
    _seed_state(seedbits, seed, state);
  }

  inline void seed_state_rand ( Rand & r )
  {
    r.rand_p(&m_seed[0], m_seed.size());
    _seed_state(m_seedbits,&m_seed[0],&m_state[0]);
  }

  bool can_seed_state ()
  {
    return m_seed_state ? true : false;
  }

  int seedbits ()
  {
    return m_seedbits;
  }

  int statebits ()
  {
    return m_statebits;
  }

  const char * name ()
  {
    return m_name;
  }
};

//-----------------------------------------------------------------------------
/* vim: set sts=2 sw=2 et: */
