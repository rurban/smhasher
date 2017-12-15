#pragma once
//-----------------------------------------------------------------------------
#include "Types.h"
#include "Random.h"

template < class T >
class hashfunc
{
public:

  pfHashWithState m_hash_with_state;
  pfSeedState m_seed_state;
  int m_seedbits;
  int m_statebits;
  std::vector<uint8_t> m_state;
  std::vector<uint8_t> m_seed; /* if we want to track where the state comes from */
  const char *m_name;

  hashfunc (
      pfSeedState seed_state, pfHashWithState hash_with_state,
      int seedbits, int statebits, const char *name
  ) :
    m_seed_state(seed_state),
    m_hash_with_state(hash_with_state),
    m_seedbits(seedbits),
    m_statebits(statebits),
    m_name(name)
  {
    m_state.resize((statebits+7)/8);
    m_seed.resize((seedbits+7)/8);
  }
  void dump_state () {
    printf("Hash: %s seedbits:%d statebits: %d hashbits %d\n",
        m_name, m_seedbits, m_statebits, (int)sizeof(T) * 8);
    printf("seed: 0x");
    for (int i= 0; i < m_seed.size(); i++)
      printf("%02x", m_seed[i]);
    printf("\n");
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
    m_hash_with_state(key,len,&m_state[0],out);
  }

  inline void operator () ( const void * key, const int len, const void *seed, void *out )
  {
    _seed_state(m_seedbits,seed,&m_state[0]);
    m_hash_with_state(key,len,&m_state[0],out);
  }

  inline void _seed_state ( const int seedbits, const void *seed, void *state )
  {
    if (m_seed_state) {
      m_seed_state(seedbits,seed,state);
    } else {
      memcpy( state, seed, m_seed.size() );
    }
    if (state == (void *)&m_state[0] && seed != (void *)&m_seed[0]) {
      memcpy( &m_seed[0], seed, m_seed.size() );
    }
  }

  inline void seed_state_zero ()
  {
    std::fill(m_seed.begin(), m_seed.end(), 0);
    _seed_state(m_seedbits, &m_seed[0], &m_state[0]);
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
    //dump_state();
  }

  bool can_seed_state ()
  {
    return m_seed_state ? true : false;
  }

  int seedbits ()
  {
    return m_seedbits;
  }

  int seedbytes ()
  {
    return (m_seedbits+7)/8;
  }

  int statebytes ()
  {
    return (m_statebits+7)/8;
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

//----------

template<typename hashtype>
struct HashCallback : public KeyCallback
{
  typedef std::vector<hashtype> hashvec;

  HashCallback ( hashfunc<hashtype> hash, hashvec & hashes ) : m_hashes(hashes), m_hashfunc(hash)
  {
    m_hashes.clear();
  }

  virtual void operator () ( const void * key, int len )
  {
    size_t newsize = m_hashes.size() + 1;

    m_hashes.resize(newsize);

    m_hashfunc(key,len,&m_hashes.back());
  }

  virtual void reserve ( int keycount )
  {
    m_hashes.reserve(keycount);
  }

  hashvec & m_hashes;
  hashfunc<hashtype> m_hashfunc;

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

  CollisionCallback ( hashfunc<hashtype>hash, hashset & collisions, collmap & cmap )
  : m_hashfunc(hash),
    m_collisions(collisions),
    m_collmap(cmap)
  {
  }

  virtual void operator () ( const void * key, int len )
  {
    hashtype h;

    m_hashfunc(key,len,&h);

    if(m_collisions.count(h))
    {
      m_collmap[h].push_back( ByteVec(key,len) );
    }
  }

  //----------

  hashfunc<hashtype> m_hashfunc;
  hashset & m_collisions;
  collmap & m_collmap;

private:

  CollisionCallback & operator = ( const CollisionCallback & c );
};


//-----------------------------------------------------------------------------
/* vim: set sts=2 sw=2 et: */
