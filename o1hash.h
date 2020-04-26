/*
  Author: Wang Yi <godspeed_china@yeah.net>
  This is a quick and dirty hash function designed for O(1) speed.
  It makes your hash table application fly in most cases.
  It samples first, middle and last 4 bytes to produce the hash.
  Do not use it in very serious applications as it's not secure.
*/
#ifndef O1HASH_LITTLE_ENDIAN
#if defined(_WIN32) || defined(__LITTLE_ENDIAN__) || (defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
#define O1HASH_LITTLE_ENDIAN 1
#elif defined(__BIG_ENDIAN__) || (defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
#define O1HASH_LITTLE_ENDIAN 0
#endif
#endif
#if (O1HASH_LITTLE_ENDIAN)
static inline unsigned _o1r4(const uint8_t *p) {	unsigned v;	memcpy(&v, p, 4);	return v;}
#else
#if defined(__GNUC__) || defined(__INTEL_COMPILER) || defined(__clang__)
static inline unsigned _o1r4(const uint8_t *p) {	unsigned v;	memcpy(&v, p, 4);	return __builtin_bswap32(v);}
#elif defined(_MSC_VER)
static inline unsigned _o1r4(const uint8_t *p) {	unsigned v;	memcpy(&v, p, 4);	return _byteswap_ulong(v);}
#endif
#endif
static inline uint64_t o1hash(const void *key, size_t len) {
  const uint8_t *p=(const uint8_t*)key;
  if(len>=4) {
    unsigned first=_o1r4(p), middle=_o1r4(p+(len>>1)-2), last=_o1r4(p+len-4);
    return (uint64_t)(first+last)*middle;
  }
  if(len){
    uint64_t tail=((((unsigned)p[0])<<16) | (((unsigned)p[len>>1])<<8) | p[len-1]);
    return tail*0xa0761d6478bd642full;
  }
  return  0;
}

