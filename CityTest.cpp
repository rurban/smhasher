#include "City.h"
#include "CityCrc.h"

void CityHash32_with_state_test ( const void * key, int len, const void *state, void * out )
{
  *(uint32*)out = CityHash32WithSeed((const char *)key,len,*((uint32_t*)state));
}

void CityHash64_with_state_test ( const void * key, int len, const void *seed, void * out )
{
  *(uint64*)out = CityHash64WithSeed((const char *)key,len,*((uint64_t*)seed));
}

#if defined(__SSE4_2__) && defined(__x86_64__)
void CityHash128_with_state_test ( const void * key, int len, const void *seed, void * out )
{
  uint128 s(0,0);

  s.first = ((uint64_t*)seed)[0];
  s.second = ((uint64_t*)seed)[1];

  *(uint128*)out = CityHash128WithSeed((const char*)key,len,s);
}

void CityHashCrc128_with_state_test ( const void * key, int len, const void *seed, void * out )
{
  uint128 s(0,0);

  s.first = ((uint64_t*)seed)[0];
  s.second = ((uint64_t*)seed)[1];

  *(uint128*)out = CityHashCrc128WithSeed((const char*)key,len,s);
}
#endif
