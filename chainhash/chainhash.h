/* ChainHash: a keyed 64-bit hash for long inputs with a proven collision bound.
 * Thomas Dybdahl Ahle, 2026. MIT license. C99/C++11, one header, no allocation.
 *
 * Key: 64 uniformly random bytes (CHAINHASH_KEY_BYTES), chainhash_key_from_bytes.
 * chainhash_key_from_seed expands a 64-bit seed; it is for benchmarks and tests.
 * Hash: chainhash(&key, data, len) returns uint64_t. Streaming: chainhash_init,
 * chainhash_update, chainhash_final. Region-aligned parallel evaluation:
 * chainhash_partial and chainhash_join. Portable C, x86 XMM/YMM/ZMM and ARM NEON
 * backends, every stride, reduction schedule and chunking compute the same digest.
 * Bound: for two distinct messages fixed independently of the key, each of at
 * most 8L bytes, Pr[collision] <= (p(L)+d(L))/2^64, where p is the block count
 * and d <= 32; see docs/THEOREM.md. The definition and index map: docs/SPEC.md.
 * Canonical little endian. Explicit ISA entry points require chainhash_has_backend().
 * AArch64: compile with -march=armv8-a+crypto (Apple: -march=native+crypto).
 * Define CHAINHASH_PORTABLE to omit all hardware code.
 * Long-input loop structure inspired by Orson Peters's PolymurHash:
 * https://github.com/orlp/polymur-hash
 */
#ifndef CHAINHASH_H
#define CHAINHASH_H
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <assert.h>
#define CHAINHASH_KEY_BYTES 64
/* ph[4C..4C+3] = kappa[4C], kappa[4C+2], kappa[4C+1], kappa[4C+3]. */
/* sp[2e], sp[2e+1] = kappa[1]*y^e, kappa[3]*y^e (e = 0..3): the short path's lane weights. */
typedef struct { uint64_t ph[32], yp[9], yh[9], c[5], tau, sp[8]; } chainhash_key;
typedef struct { uint64_t lo, hi; } ch_raw;
enum { CH_PORTABLE=0, CH_XMM=1, CH_YMM=2, CH_ZMM=3, CH_NEON=4 };
static inline uint64_t ch_word(const uint8_t *p,size_t n,size_t off) {
    uint64_t a=0; unsigned i;
    if(off>=n) return 0;
#if defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    if(n-off>=8) { memcpy(&a,p+off,8); return a; }
#endif
    for(i=0;i<8 && i<n-off;i++) a|=(uint64_t)p[off+i]<<(8*i);
    return a;
}
static inline unsigned ch_lanes(size_t n) { return n>48 ? 4 : n ? (unsigned)((n-1)/16+1) : 1; }
static inline ch_raw ch_clmul(uint64_t a,uint64_t b) {
    ch_raw r={0,0}; unsigned i;
    for(i=0;i<64;i++) { uint64_t m=0-((b>>i)&1); r.lo^=(a<<i)&m; if(i) r.hi^=(a>>(64-i))&m; }
    return r;
}
static inline uint64_t ch_reduce(ch_raw a) {
    uint64_t h=a.hi, q=(h>>63)^(h>>61)^(h>>60);
    return a.lo^h^(h<<1)^(h<<3)^(h<<4)^q^(q<<1)^(q<<3)^(q<<4);
}
static inline uint64_t ch_mul(uint64_t a,uint64_t b) { return ch_reduce(ch_clmul(a,b)); }
/* Key expansion multiplies with the dispatched hardware product (defined below; the
 * same field product as ch_mul, which a portable build uses). */
static inline int chainhash_backend(void);
static inline uint64_t ch_fmul(uint64_t a,uint64_t b,int backend);
static inline void ch_schedule(chainhash_key *k,uint64_t y) {
    unsigned i; int b=chainhash_backend(); k->yp[0]=1; k->yh[0]=27;
    for(i=1;i<=8;i++) { k->yp[i]=ch_fmul(k->yp[i-1],y,b); k->yh[i]=ch_fmul(27,k->yp[i],b); }
    for(i=0;i<4;i++) { k->sp[2*i]=ch_fmul(k->ph[2],k->yp[i],b); k->sp[2*i+1]=ch_fmul(k->ph[3],k->yp[i],b); }
}
static inline chainhash_key chainhash_key_from_words(const uint64_t w[39]) {
    chainhash_key k; unsigned c;
    for(c=0;c<8;c++) { k.ph[4*c]=w[4*c]; k.ph[4*c+1]=w[4*c+2]; k.ph[4*c+2]=w[4*c+1]; k.ph[4*c+3]=w[4*c+3]; }
    ch_schedule(&k,w[32]); for(c=0;c<5;c++) k.c[c]=w[33+c]; k.tau=w[38]; return k;
}
static inline chainhash_key chainhash_key_from_bytes(const uint8_t b[64]) {
    uint64_t w[39],s=ch_word(b,64,0),v=s; unsigned i; int hw=chainhash_backend();
    for(i=0;i<32;i++) { w[i]=v; v=ch_fmul(v,s,hw); }
    for(i=0;i<7;i++) w[32+i]=ch_word(b,64,8*(i+1));
    return chainhash_key_from_words(w);
}
/* Convenience benchmark seed expansion, not 64 bytes of independent entropy. */
static inline chainhash_key chainhash_key_from_seed(uint64_t seed) {
    uint8_t b[64]; unsigned i,j;
    for(i=0;i<8;i++) { uint64_t z=(seed+=UINT64_C(0x9e3779b97f4a7c15)); z=(z^(z>>30))*UINT64_C(0xbf58476d1ce4e5b9); z=(z^(z>>27))*UINT64_C(0x94d049bb133111eb); z^=z>>31; for(j=0;j<8;j++) b[8*i+j]=(uint8_t)(z>>(8*j)); }
    return chainhash_key_from_bytes(b);
}
#if !defined(CHAINHASH_PORTABLE) && (defined(__x86_64__) || defined(__i386__)) && (defined(__GNUC__) || defined(__clang__))
#define CH_X86 1
#include <immintrin.h>
#define CH_T128 __attribute__((target("avx,pclmul")))
#define CH_T256 __attribute__((target("avx2,pclmul,vpclmulqdq")))
#define CH_T512 __attribute__((target("avx2,pclmul,avx512f,avx512bw,vpclmulqdq")))
/* CPUID by inline assembly: GCC 9's <cpuid.h> has no include guard, so a
 * translation unit that included it twice (both ChainHash headers) failed. */
static inline void ch_cpuid(unsigned leaf,unsigned sub,unsigned r[4]) {
    __asm__ __volatile__("cpuid":"=a"(r[0]),"=b"(r[1]),"=c"(r[2]),"=d"(r[3]):"a"(leaf),"c"(sub));
}
static inline int ch_detect(void) {
    unsigned r[4],m,l,h;
    ch_cpuid(0,0,r); m=r[0]; if(m<1) return 0;
    ch_cpuid(1,0,r); if((r[2]&((1u<<1)|(1u<<27)|(1u<<28)))!=((1u<<1)|(1u<<27)|(1u<<28))) return 0;
    __asm__ volatile("xgetbv":"=a"(l),"=d"(h):"c"(0)); if((l&6)!=6) return 0;
    if(m<7) return 1;
    ch_cpuid(7,0,r); if(!(r[1]&(1u<<5)) || !(r[2]&(1u<<10))) return 1;
    return (l&0xe6)==0xe6 && (r[1]&(1u<<16)) && (r[1]&(1u<<30)) ? 3:2;   /* ZMM: AVX512F and AVX512BW */
}
CH_T128 static inline ch_raw ch_hwprod(uint64_t a,uint64_t b) {
    ch_raw r; __m128i v=_mm_clmulepi64_si128(_mm_set_epi64x(0,(long long)a),_mm_set_epi64x(0,(long long)b),0); _mm_storeu_si128((__m128i_u *)&r,v); return r;
}
#elif !defined(CHAINHASH_PORTABLE) && defined(__aarch64__) && !defined(__AARCH64EB__) && (defined(__GNUC__) || defined(__clang__))
#define CH_ARM 1
#include <arm_neon.h>
/* PMULL (FEAT_PMULL) is optional: without it at compile time the NEON code carries a
 * target attribute (CH_NBEGIN/CH_NEND) and the backend is NEON only on a CPU that
 * reports PMULL at run time (Linux HWCAP, Apple sysctl), portable otherwise. */
/* NEON accumulation form: 1 accumulates every product with a PMULL/PMULL2 followed by
 * an EOR into the product's register, which Apple cores issue as one fused operation;
 * 0 uses PMULL, PMULL2 and EOR3 (fewer operations on cores that do not fuse the pair).
 * Default: 1 on Apple targets. Same digest either way. */
#ifndef CHAINHASH_NEON_FUSE
#if defined(__APPLE__)
#define CHAINHASH_NEON_FUSE 1
#else
#define CHAINHASH_NEON_FUSE 0
#endif
#endif
#if defined(__ARM_FEATURE_AES) || defined(__ARM_FEATURE_CRYPTO)
#define CH_PMULL 1
#define CH_NBEGIN
#define CH_NEND
#else
#define CH_PMULL 2
#ifdef __clang__
#define CH_NBEGIN _Pragma("clang attribute push(__attribute__((target(\"aes\"))),apply_to=function)")
#define CH_NEND _Pragma("clang attribute pop")
#else
#define CH_NBEGIN _Pragma("GCC push_options") _Pragma("GCC target(\"+crypto\")")
#define CH_NEND _Pragma("GCC pop_options")
#endif
#endif
#if CH_PMULL == 2
#if defined(__linux__)
#include <sys/auxv.h>
#elif defined(__APPLE__)
/* Declared here: <sys/sysctl.h> does not compile under strict _POSIX_C_SOURCE. */
#ifdef __cplusplus
extern "C"
#endif
int sysctlbyname(const char *,void *,size_t *,void *,size_t);
#endif
/* 1 if the CPU has an extension: Linux AT_HWCAP bit (PMULL 1<<4, SHA3 1<<17);
 * Apple sysctl, then the older name, then dflt. Other systems report absent. */
static inline int ch_arm_has(unsigned long hwcap,const char *feat,const char *old,int dflt) {
#if defined(__linux__)
    (void)feat; (void)old; (void)dflt; return (getauxval(AT_HWCAP)&hwcap)!=0;
#elif defined(__APPLE__)
    int v=0; size_t n=sizeof(v); (void)hwcap;
    if(sysctlbyname(feat,&v,&n,NULL,0)==0) return v!=0;
    v=0; n=sizeof(v); if(old && sysctlbyname(old,&v,&n,NULL,0)==0) return v!=0;
    return dflt;
#else
    (void)hwcap; (void)feat; (void)old; (void)dflt; return 0;
#endif
}
#endif
CH_NBEGIN
static inline uint64x2_t ch_ll(uint64x2_t a,uint64x2_t b) { uint64x2_t r; __asm__("pmull %0.1q, %1.1d, %2.1d":"=w"(r):"w"(a),"w"(b)); return r; }
static inline uint64x2_t ch_hh(uint64x2_t a,uint64x2_t b) { uint64x2_t r; __asm__("pmull2 %0.1q, %1.2d, %2.2d":"=w"(r):"w"(a),"w"(b)); return r; }
/* acc^lo(a)*lo(b), acc^hi(a)*hi(b): the EOR writes its PMULL's destination right
 * after it, which Apple cores issue as one fused operation. */
static inline uint64x2_t ch_fll(uint64x2_t acc,uint64x2_t a,uint64x2_t b) { uint64x2_t r; __asm__("pmull %0.1q, %1.1d, %2.1d\n\teor %0.16b, %0.16b, %3.16b":"=&w"(r):"w"(a),"w"(b),"w"(acc)); return r; }
static inline uint64x2_t ch_fhh(uint64x2_t acc,uint64x2_t a,uint64x2_t b) { uint64x2_t r; __asm__("pmull2 %0.1q, %1.2d, %2.2d\n\teor %0.16b, %0.16b, %3.16b":"=&w"(r):"w"(a),"w"(b),"w"(acc)); return r; }
static inline uint64x2_t ch_xor3(uint64x2_t a,uint64x2_t b,uint64x2_t c) {
#if defined(__ARM_FEATURE_SHA3)
    return veor3q_u64(a,b,c);
#else
    return veorq_u64(a,veorq_u64(b,c));
#endif
}
static inline ch_raw ch_hwprod(uint64_t a,uint64_t b) { ch_raw r; vst1q_u64(&r.lo,ch_ll(vcombine_u64(vcreate_u64(a),vcreate_u64(0)),vcombine_u64(vcreate_u64(b),vcreate_u64(0)))); return r; }
CH_NEND
#endif
static inline int chainhash_backend(void) {
#ifdef CH_X86
    static int cache=-1; int v=__atomic_load_n(&cache,__ATOMIC_RELAXED); if(v<0) { v=ch_detect(); __atomic_store_n(&cache,v,__ATOMIC_RELAXED); } return v;
#elif defined(CH_ARM)
#if CH_PMULL == 2
    static int cache=-1; int v=__atomic_load_n(&cache,__ATOMIC_RELAXED); if(v<0) { v=ch_arm_has(1ul<<4,"hw.optional.arm.FEAT_PMULL",NULL,1)?CH_NEON:0; __atomic_store_n(&cache,v,__ATOMIC_RELAXED); } return v;
#else
    return CH_NEON;
#endif
#else
    return 0;
#endif
}
static inline int chainhash_has_backend(int b) { int h=chainhash_backend(); return b==0 || (h==4 ? b==4 : b>0 && b<=h); }
#ifdef CH_X86
/* Whether PCLMULQDQ on XMM issues every cycle: Intel cores with ADX (Broadwell and
 * later). AMD issues it once per two cycles at every width, Intel Sandy Bridge to
 * Haswell once per two to eight (uops.info). */
static inline int ch_pclmul_fast(void) {
    static int cache=-1; int v=__atomic_load_n(&cache,__ATOMIC_RELAXED);
    if(v<0) { unsigned r[4],m; ch_cpuid(0,0,r); m=r[0];
        v=r[1]==0x756e6547u && r[3]==0x49656e69u && r[2]==0x6c65746eu;
        if(v) { if(m>=7) { ch_cpuid(7,0,r); v=(r[1]>>19)&1; } else v=0; }
        __atomic_store_n(&cache,v,__ATOMIC_RELAXED); }
    return v;
}
#endif
static inline ch_raw ch_prod(uint64_t a,uint64_t b,int backend) {
#if defined(CH_X86) || defined(CH_ARM)
    if(backend) return ch_hwprod(a,b);
#else
    (void)backend;
#endif
    return ch_clmul(a,b);
}
static inline uint64_t ch_fmul(uint64_t a,uint64_t b,int backend) { return ch_reduce(ch_prod(a,b,backend)); }
static inline uint64_t ch_pow(uint64_t a,uint64_t n,int backend) { uint64_t r=1; while(n) { if(n&1) r=ch_fmul(r,a,backend); n>>=1; if(n) a=ch_fmul(a,a,backend); } return r; }
/* Software prefetch in the bulk kernels: hint CH_PF_T0/T1/NTA (AArch64 PLDL1KEEP/
 * PLDL2KEEP/PLDL1STRM) every step bytes of the region dist bytes ahead, formed only
 * while that region lies inside the input. A hint has no architectural effect, so
 * no setting changes a digest. hint and step are compile-time constants of each
 * kernel instance; CH_PF_OFF is the shipped kernel. */
#ifndef CHAINHASH_PF_HINTS
#define CHAINHASH_PF_HINTS
enum { CH_PF_OFF=0, CH_PF_T0=1, CH_PF_T1=2, CH_PF_NTA=3 };
#endif
#if defined(CH_X86) || defined(CH_ARM)
#define CH_INLINE static inline __attribute__((always_inline))
#define CH_PREFETCH(p,regions,bytes,hint,step,dist) do { if((hint) && (dist)<=((regions)-1)*(size_t)(bytes)) { unsigned o_; \
    for(o_=0;o_<(bytes);o_+=(step)) { const char *q_=(const char *)(p)+(dist)+o_; \
        if((hint)==CH_PF_T0) __builtin_prefetch(q_,0,3); else if((hint)==CH_PF_T1) __builtin_prefetch(q_,0,2); else __builtin_prefetch(q_,0,0); } } } while(0)
/* The prefetching instances of a kernel NAME##_run(args...,hint,step,dist): hint and step
 * fixed, dist run time. */
#define CH_PF_SWITCH(NAME,...) switch(hint*2+(step==128)) { \
    case 2: return NAME##_run(__VA_ARGS__,CH_PF_T0,64,dist);  case 3: return NAME##_run(__VA_ARGS__,CH_PF_T0,128,dist); \
    case 4: return NAME##_run(__VA_ARGS__,CH_PF_T1,64,dist);  case 5: return NAME##_run(__VA_ARGS__,CH_PF_T1,128,dist); \
    case 6: return NAME##_run(__VA_ARGS__,CH_PF_NTA,64,dist); case 7: return NAME##_run(__VA_ARGS__,CH_PF_NTA,128,dist); \
    default: return NAME##_run(__VA_ARGS__,CH_PF_OFF,64,0); }
#endif
#ifdef CH_X86
/* Low lane as an integer: _mm_cvtsi128_si64 is x86-64 only in GCC. */
CH_T128 static inline uint64_t ch_lane0(__m128i v) {
#if defined(__x86_64__)
    return (uint64_t)_mm_cvtsi128_si64(v);
#else
    ch_raw r; _mm_storeu_si128((__m128i_u *)&r,v); return r.lo;
#endif
}
CH_T128 static inline __m128i ch_vreduce(__m128i a) {
    const __m128i r=_mm_set1_epi64x(27);
    __m128i t=_mm_clmulepi64_si128(a,r,0x11),u=_mm_clmulepi64_si128(t,r,0x11);
    return _mm_xor_si128(a,_mm_xor_si128(t,u));
}
/* Latency form of the reduction and the finalizer. Values travel in lane 1 of a vector (lane 0
 * is scratch): every product selects its lanes by immediate, and lane 1 of an unreduced
 * product a = lo + X^64 hi is reduced in four dependent steps. With h = hi, h*27 = h ^ h<<1 ^
 * h<<3 ^ h<<4 plus the carry-out q = h>>63 ^ h>>61 ^ h>>60 times 27, and q*27 is a function of
 * the top nibble h>>60 alone: a 16-entry PSHUFB table. ch_fold1(a,c): lane 1 = (a mod p) ^ c.hi. */
static const uint8_t ch_q27[16]={0,27,45,54,90,65,119,108,175,180,130,153,245,238,216,195};
CH_T128 static inline __m128i ch_fold1(__m128i a,__m128i c) {
    __m128i t=_mm_shuffle_epi8(_mm_loadu_si128((const __m128i_u *)ch_q27),_mm_srli_epi64(a,60));
    __m128i u=_mm_xor_si128(_mm_xor_si128(a,c),_mm_unpacklo_epi64(a,a));
    __m128i v=_mm_xor_si128(_mm_slli_epi64(a,1),_mm_slli_epi64(a,3));
    return _mm_xor_si128(_mm_xor_si128(u,v),_mm_xor_si128(_mm_slli_epi64(a,4),t));
}
CH_T128 static inline __m128i ch_dup(const uint64_t *p) { return _mm_castpd_si128(_mm_loaddup_pd((const double *)p)); }
/* The finalizer on lane 1 of v (reduced): x = v + tau, q = x^2, R = (q^c0)(x^q^c1) raw,
 * out = z*(R^c3) ^ c4 with z = x^c2. The reductions of q^c0 and x^q^c1 share one fold of x^2
 * with the constants injected; z*(R^c3) = z*R.lo ^ w*R.hi ^ z*c3 with w = z*X^64 mod p, and
 * z*c3 ^ c4 are formed while R is computed, so the chain is three products and two folds. */
CH_T128 static inline uint64_t ch_fin1(const chainhash_key *k,__m128i v) {
    const __m128i zero=_mm_setzero_si128();
    __m128i x=_mm_add_epi64(v,ch_dup(&k->tau)),Q=_mm_clmulepi64_si128(x,x,0x11);
    __m128i z=_mm_unpackhi_epi64(zero,_mm_xor_si128(x,ch_dup(k->c+2))),w=ch_fold1(z,zero);
    __m128i F=ch_fold1(_mm_clmulepi64_si128(z,ch_dup(k->c+3),0x01),ch_dup(k->c+4));
    __m128i R=_mm_clmulepi64_si128(ch_fold1(Q,ch_dup(k->c)),ch_fold1(Q,_mm_xor_si128(x,ch_dup(k->c+1))),0x11);
    __m128i r=_mm_xor_si128(_mm_clmulepi64_si128(z,R,0x01),_mm_clmulepi64_si128(w,R,0x11));
    return (uint64_t)_mm_extract_epi64(ch_fold1(r,F),1);
}
/* The finalizer on lane 0 of v (reduced). */
CH_T128 static inline uint64_t ch_finish128(const chainhash_key *k,__m128i v) { return ch_fin1(k,_mm_unpacklo_epi64(v,v)); }
CH_T128 static uint64_t ch_fastfinish(const chainhash_key *k,uint64_t v) { return ch_fin1(k,_mm_set_epi64x((long long)v,0)); }
#elif defined(CH_ARM)
CH_NBEGIN
static inline uint64x2_t ch_vreduce(uint64x2_t a) { const uint64x2_t r=vdupq_n_u64(27); uint64x2_t t=ch_hh(a,r); return ch_xor3(a,t,ch_hh(t,r)); }
static inline uint64x2_t ch_v64(uint64_t v) { return vcombine_u64(vcreate_u64(v),vcreate_u64(0)); }
static inline uint64x2_t ch_ld(const uint8_t *p) { return vreinterpretq_u64_u8(vld1q_u8(p)); }
/* ch_nred(a,c): lane 0 = (a mod p) ^ c.lo, the constant injected beside the two reduction
 * products (a^c is formed while they run). */
static inline uint64x2_t ch_nred(uint64x2_t a,uint64x2_t c) {
    const uint64x2_t r=vdupq_n_u64(27); uint64x2_t t=ch_hh(a,r); return ch_xor3(veorq_u64(a,c),t,ch_hh(t,r));
}
/* The finalizer on lane 0 of v (reduced), as ch_fin1 on x86: q^c0 and x^q^c1 share the
 * reduction products of x^2 with the constants injected, and the last product absorbs the
 * reduction of R: z*(R^c3) = z*R.lo ^ w*R.hi ^ z*c3 with w = z*X^64 mod p and z*c3 ^ c4
 * formed while R is computed. Three products and two reductions after the twist. */
static inline uint64_t ch_finish_neon_vec(const chainhash_key *k,uint64x2_t v) {
    const uint64x2_t r=vdupq_n_u64(27);
    uint64x2_t x=vaddq_u64(v,vdupq_n_u64(k->tau)),Q=ch_ll(x,x),z=veorq_u64(x,vdupq_n_u64(k->c[2]));
    uint64x2_t t=ch_hh(Q,r),u=ch_hh(t,r),a=ch_xor3(veorq_u64(Q,vdupq_n_u64(k->c[0])),t,u),b=ch_xor3(veorq_u64(Q,veorq_u64(x,vdupq_n_u64(k->c[1]))),t,u);
    uint64x2_t zr=ch_ll(z,r),w=vdupq_laneq_u64(veorq_u64(zr,ch_hh(zr,r)),0);
    uint64x2_t F=ch_nred(ch_ll(z,vdupq_n_u64(k->c[3])),vdupq_n_u64(k->c[4]));
    uint64x2_t R=ch_ll(a,b);
    return vgetq_lane_u64(ch_nred(veorq_u64(ch_ll(z,R),ch_hh(w,R)),F),0);
}
static inline uint64_t ch_fastfinish(const chainhash_key *k,uint64_t v) {
    return ch_finish_neon_vec(k,ch_v64(v));
}
/* Up to 64 bytes (every partner absent): one product level against the key-side lane
 * weights sp, the raw leading term leading*y^L added, one reduction; registers only. */
static inline uint64x2_t ch_nprod(uint64x2_t a,const uint64_t *P,int hi) {
    uint64x2_t K=vld1q_u64(P),r=ch_ll(a,K); return hi ? veorq_u64(r,ch_hh(a,K)) : r;
}
static inline uint64_t ch_short_neon(const chainhash_key *k,const uint8_t *p,size_t n,uint64_t leading) {
    static const uint8_t idx[32]={0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,
        0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80};
    const uint64x2_t ka=vld1q_u64(k->ph);
    unsigned lanes=ch_lanes(n);
    uint64x2_t acc=ch_ll(ch_v64(leading),ch_v64(k->yp[lanes]));
    if(n<=16) {
        uint64_t lo,hi;
        if(n>8) {
            memcpy(&lo,p,8); memcpy(&hi,p+n-8,8); hi>>=8*(16-n);
            acc=veorq_u64(acc,ch_nprod(veorq_u64(vcombine_u64(vcreate_u64(lo),vcreate_u64(hi)),ka),k->sp,1));
        } else if(n) {
            if(n>=4) { uint32_t a,b; memcpy(&a,p,4); memcpy(&b,p+n-4,4); lo=a|(uint64_t)b<<(8*(n-4)); }
            else lo=(uint64_t)p[0]|(uint64_t)p[n>>1]<<(8*(n>>1))|(uint64_t)p[n-1]<<(8*(n-1));
            acc=veorq_u64(acc,ch_nprod(veorq_u64(ch_v64(lo),ka),k->sp,0));
        }
    } else {
        size_t r=n-16*(size_t)(lanes-1);
        uint64x2_t last=vreinterpretq_u64_u8(vqtbl1q_u8(vld1q_u8(p+n-16),vld1q_u8(idx+16-r)));
        last=ch_nprod(veorq_u64(last,ka),k->sp,r>8);
        switch(lanes) {
        case 2: acc=ch_xor3(acc,ch_nprod(veorq_u64(ch_ld(p),ka),k->sp+2,1),last); break;
        case 3: acc=ch_xor3(acc,veorq_u64(ch_nprod(veorq_u64(ch_ld(p),ka),k->sp+4,1),ch_nprod(veorq_u64(ch_ld(p+16),ka),k->sp+2,1)),last); break;
        default: acc=ch_xor3(veorq_u64(acc,ch_nprod(veorq_u64(ch_ld(p),ka),k->sp+6,1)),veorq_u64(ch_nprod(veorq_u64(ch_ld(p+16),ka),k->sp+4,1),ch_nprod(veorq_u64(ch_ld(p+32),ka),k->sp+2,1)),last); break;
        }
    }
    return ch_finish_neon_vec(k,ch_nred(acc,vdupq_n_u64(0)));
}
/* The 16 bytes at q+o with the bytes at or past q+rem zero, read in place: a partial word
 * as the 16 bytes ending at q+rem shifted down by TBL. The caller guarantees that q+rem-16
 * is inside the input. */
static inline uint64x2_t ch_nword(const uint8_t *q,size_t o,size_t rem) {
    static const uint8_t idx[32]={0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,
        0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80};
    if(o+16<=rem) return ch_ld(q+o);
    if(o>=rem) return vdupq_n_u64(0);
    return vreinterpretq_u64_u8(vqtbl1q_u8(vld1q_u8(q+rem-16),vld1q_u8(idx+16-(rem-o))));
}
/* Final partial regions above 64 bytes (the only caller): raw products and the Horner fold
 * in SIMD registers, the last partial chunk read in place (n > 64, so the 16 bytes before
 * its end are input). Only the final digest leaves lane 0. */
static uint64_t ch_tail_neon(const chainhash_key *k,const uint8_t *p,size_t n,uint64_t leading) {
    uint64x2_t u[4]={vdupq_n_u64(0),vdupq_n_u64(0),vdupq_n_u64(0),vdupq_n_u64(0)};
    unsigned c=0,j,lanes=n>48?4:n?(unsigned)((n-1)/16+1):1;
    size_t rem=n;
    while(rem) {
        size_t take=rem<128?rem:128;
        uint64x2_t ka=vld1q_u64(k->ph+4*c),kb=vld1q_u64(k->ph+4*c+2);
        for(j=0;j<4;j++) if(16*j<take) {
            uint64x2_t a=veorq_u64(take==128 ? ch_ld(p+16*j) : ch_nword(p,16*j,take),ka);
            uint64x2_t b=veorq_u64(take==128 ? ch_ld(p+64+16*j) : ch_nword(p,64+16*j,take),kb);
            if(take<=16*j+8) { a=vsetq_lane_u64(0,a,1); b=vsetq_lane_u64(0,b,1); }
            u[j]=ch_xor3(u[j],ch_ll(a,b),ch_hh(a,b));
        }
        rem-=take; p+=take; ++c;
    }
    uint64x2_t acc=ch_ll(ch_v64(leading),ch_v64(k->yp[lanes]));
    for(j=0;j<lanes;j++) {
        unsigned e=lanes-1-j;
        if(!e) acc=veorq_u64(acc,u[j]);
        else { uint64x2_t pw=vcombine_u64(vcreate_u64(k->yp[e]),vcreate_u64(k->yh[e])); acc=ch_xor3(acc,ch_ll(u[j],pw),ch_hh(u[j],pw)); }
    }
    return ch_finish_neon_vec(k,ch_nred(acc,vdupq_n_u64(0)));
}
CH_NEND
#endif
static inline uint64_t ch_finish(const chainhash_key *k,uint64_t v,int b) {
#if defined(CH_X86) || defined(CH_ARM)
    if(b) return ch_fastfinish(k,v);
#endif
    uint64_t q,r; v+=k->tau; q=ch_fmul(v,v,b); r=ch_fmul(q^k->c[0],v^q^k->c[1],b); return ch_fmul(v^k->c[2],r^k->c[3],b)^k->c[4]; }
/* Partial region: a pair is active iff its FIRST word has a byte. */
static inline void ch_region_scalar(const chainhash_key *k,const uint8_t *p,size_t n,ch_raw out[4],int b) {
    unsigned c,j,e; memset(out,0,4*sizeof(*out));
    for(c=0;c<8;c++) for(j=0;j<4;j++) for(e=0;e<2;e++) {
        size_t off=128*c+16*j+8*e;
        if(off<n) { ch_raw v=ch_prod(ch_word(p,n,off)^k->ph[4*c+e],ch_word(p,n,off+64)^k->ph[4*c+2+e],b); out[j].lo^=v.lo; out[j].hi^=v.hi; }
    }
}

#ifdef CH_X86
/* Loads for the partial final region: never outside the input. ch_ldend returns the
 * avail (1..16) bytes that end at e, zero-extended; it reads e-16..e-1, so it needs 16
 * input bytes before e. ch_ldsmall assembles a 1..15-byte input from overlapping loads. */
static const uint8_t ch_shift_idx[32]={0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,
    0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80};
CH_T128 static inline __m128i ch_ldend(const uint8_t *e,size_t avail) {
    return _mm_shuffle_epi8(_mm_loadu_si128((const __m128i_u *)(e-16)),_mm_loadu_si128((const __m128i_u *)(ch_shift_idx+16-avail)));
}
CH_T128 static inline __m128i ch_ldsmall(const uint8_t *p,size_t n) {
    uint64_t lo,hi=0;
    if(n>=8) { memcpy(&lo,p,8); if(n>8) { memcpy(&hi,p+n-8,8); hi>>=8*(16-n); } }
    else if(n>=4) { uint32_t a,b; memcpy(&a,p,4); memcpy(&b,p+n-4,4); lo=a|(uint64_t)b<<(8*(n-4)); }
    else lo=(uint64_t)p[0]|(uint64_t)p[n>>1]<<(8*(n>>1))|(uint64_t)p[n-1]<<(8*(n-1));
    return _mm_set_epi64x((long long)hi,(long long)lo);
}
/* The 16 bytes at offset o of an n-byte input, zero-extended past n (o<n). */
CH_T128 static inline __m128i ch_ldword(const uint8_t *p,size_t n,size_t o) {
    if(n>=o+16) return _mm_loadu_si128((const __m128i_u *)(p+o));
    return n>=16 ? ch_ldend(p+n,n-o) : ch_ldsmall(p,n);
}
/* Partial final region (n<1024) for the x86 backends, as ch_tail_neon: raw lane products,
 * Horner weights (y^e, X^64 y^e) on raw lanes, one reduction, and the finalizer, all in
 * vector registers. A pair is active iff its first word has a byte; an inactive second
 * pair of a lane has its data vector's high half cleared. The Horner weights are loaded
 * as single words and selected by the product immediate. */
CH_T128 static inline __m128i ch_weigh(const chainhash_key *k,__m128i u,unsigned e) {
    return _mm_xor_si128(_mm_clmulepi64_si128(u,_mm_loadl_epi64((const __m128i *)(k->yp+e)),0x00),
                         _mm_clmulepi64_si128(u,_mm_loadl_epi64((const __m128i *)(k->yh+e)),0x01));
}
/* Up to 64 bytes every partner word is absent, so lane j contributes (w ^ kappa0)*kappa1 ^
 * (w' ^ kappa2)*kappa3 weighted by y^(L-1-j): with the key-side weights sp (kappa1*y^e, kappa3*y^e)
 * every data word meets one product, all products are independent, and the raw sum is folded
 * once with the leading term leading*y^L (formed while the data load) injected. Registers only;
 * each lane count has its own straight-line code. */
CH_T128 static inline __m128i ch_sprod(__m128i a,const uint64_t *P,int hi) {
    __m128i K=_mm_loadu_si128((const __m128i_u *)P),r=_mm_clmulepi64_si128(a,K,0x00);
    return hi ? _mm_xor_si128(r,_mm_clmulepi64_si128(a,K,0x11)) : r;
}
CH_T128 static inline __m128i ch_sword(const uint8_t *p,__m128i ka,const uint64_t *P) {
    return ch_sprod(_mm_xor_si128(_mm_loadu_si128((const __m128i_u *)p),ka),P,1);
}
CH_T128 static inline uint64_t ch_short128(const chainhash_key *k,const uint8_t *p,size_t n,uint64_t leading) {
    const __m128i ka=_mm_loadu_si128((const __m128i_u *)k->ph);
    unsigned lanes=ch_lanes(n);
    __m128i C=ch_fold1(_mm_clmulepi64_si128(_mm_cvtsi64_si128((long long)leading),_mm_loadl_epi64((const __m128i *)(k->yp+lanes)),0),_mm_setzero_si128()),acc;
    if(n<=16) {
        uint64_t lo,hi; __m128i K=_mm_loadu_si128((const __m128i_u *)k->sp);
        if(n>8) {
            memcpy(&lo,p,8); memcpy(&hi,p+n-8,8); hi>>=8*(16-n);
            acc=_mm_xor_si128(_mm_clmulepi64_si128(_mm_xor_si128(_mm_cvtsi64_si128((long long)lo),ka),K,0x00),
                              _mm_clmulepi64_si128(_mm_xor_si128(_mm_cvtsi64_si128((long long)hi),_mm_unpackhi_epi64(ka,ka)),K,0x10));
        } else {
            if(n>=4) { uint32_t a,b; memcpy(&a,p,4); memcpy(&b,p+n-4,4); lo=a|(uint64_t)b<<(8*(n-4)); }
            else if(n) lo=(uint64_t)p[0]|(uint64_t)p[n>>1]<<(8*(n>>1))|(uint64_t)p[n-1]<<(8*(n-1));
            else return ch_fin1(k,C);
            acc=_mm_clmulepi64_si128(_mm_xor_si128(_mm_cvtsi64_si128((long long)lo),ka),K,0x00);
        }
    } else {
        size_t r=n-16*(size_t)(lanes-1);   /* bytes in the last lane, 1..16 */
        __m128i last=ch_sprod(_mm_xor_si128(ch_ldend(p+n,r),ka),k->sp,r>8);
        switch(lanes) {
        case 2: acc=_mm_xor_si128(ch_sword(p,ka,k->sp+2),last); break;
        case 3: acc=_mm_xor_si128(_mm_xor_si128(ch_sword(p,ka,k->sp+4),ch_sword(p+16,ka,k->sp+2)),last); break;
        default: acc=_mm_xor_si128(_mm_xor_si128(ch_sword(p,ka,k->sp+6),ch_sword(p+16,ka,k->sp+4)),_mm_xor_si128(ch_sword(p+32,ka,k->sp+2),last)); break;
        }
    }
    return ch_fin1(k,ch_fold1(acc,C));
}
CH_T128 __attribute__((noinline)) static uint64_t ch_tail128(const chainhash_key *k,const uint8_t *p,size_t n,uint64_t leading) {
    size_t full=n/128,rem=n%128; unsigned j,c;
    __m128i acc,u[4]={_mm_setzero_si128(),_mm_setzero_si128(),_mm_setzero_si128(),_mm_setzero_si128()};
    if(n<=64) return ch_short128(k,p,n,leading);
    for(c=0;c<=full;c++) {
        size_t take=c<full?128:rem; const uint8_t *q=p+128*c;
        const __m128i ka=_mm_loadu_si128((const __m128i_u *)(k->ph+4*c)),kb=_mm_loadu_si128((const __m128i_u *)(k->ph+4*c+2));
        for(j=0;j<4 && 16*j<take;j++) {
            size_t o=16*j; __m128i a,b;
            a=_mm_xor_si128(take==128 ? _mm_loadu_si128((const __m128i_u *)(q+o)) : ch_ldword(q,take,o),ka);
            b=_mm_xor_si128(take==128 ? _mm_loadu_si128((const __m128i_u *)(q+64+o)) : take>64+o ? ch_ldword(q,take,64+o) : _mm_setzero_si128(),kb);
            if(take<=o+8) a=_mm_move_epi64(a);
            u[j]=_mm_xor_si128(u[j],_mm_xor_si128(_mm_clmulepi64_si128(a,b,0),_mm_clmulepi64_si128(a,b,0x11)));
        }
    }
    acc=_mm_clmulepi64_si128(_mm_set_epi64x(0,(long long)leading),_mm_loadl_epi64((const __m128i *)(k->yp+4)),0);
    acc=_mm_xor_si128(_mm_xor_si128(acc,ch_weigh(k,u[0],3)),_mm_xor_si128(ch_weigh(k,u[1],2),_mm_xor_si128(ch_weigh(k,u[2],1),u[3])));
    return ch_fin1(k,ch_fold1(acc,_mm_setzero_si128()));
}
CH_T128 static inline void ch_region128(const chainhash_key *k,const uint8_t *p,ch_raw out[4]) {
    unsigned j; for(j=0;j<4;j+=1) {
    __m128i a=_mm_setzero_si128();
    { __m128i x=_mm_xor_si128(_mm_loadu_si128((const __m128i_u *)(p+0+16*j)),_mm_loadu_si128((const __m128i_u *)(k->ph+0))), y=_mm_xor_si128(_mm_loadu_si128((const __m128i_u *)(p+64+16*j)),_mm_loadu_si128((const __m128i_u *)(k->ph+2))); a=_mm_xor_si128(a,_mm_xor_si128(_mm_clmulepi64_si128(x,y,0),_mm_clmulepi64_si128(x,y,0x11))); }
    { __m128i x=_mm_xor_si128(_mm_loadu_si128((const __m128i_u *)(p+128+16*j)),_mm_loadu_si128((const __m128i_u *)(k->ph+4))), y=_mm_xor_si128(_mm_loadu_si128((const __m128i_u *)(p+192+16*j)),_mm_loadu_si128((const __m128i_u *)(k->ph+6))); a=_mm_xor_si128(a,_mm_xor_si128(_mm_clmulepi64_si128(x,y,0),_mm_clmulepi64_si128(x,y,0x11))); }
    { __m128i x=_mm_xor_si128(_mm_loadu_si128((const __m128i_u *)(p+256+16*j)),_mm_loadu_si128((const __m128i_u *)(k->ph+8))), y=_mm_xor_si128(_mm_loadu_si128((const __m128i_u *)(p+320+16*j)),_mm_loadu_si128((const __m128i_u *)(k->ph+10))); a=_mm_xor_si128(a,_mm_xor_si128(_mm_clmulepi64_si128(x,y,0),_mm_clmulepi64_si128(x,y,0x11))); }
    { __m128i x=_mm_xor_si128(_mm_loadu_si128((const __m128i_u *)(p+384+16*j)),_mm_loadu_si128((const __m128i_u *)(k->ph+12))), y=_mm_xor_si128(_mm_loadu_si128((const __m128i_u *)(p+448+16*j)),_mm_loadu_si128((const __m128i_u *)(k->ph+14))); a=_mm_xor_si128(a,_mm_xor_si128(_mm_clmulepi64_si128(x,y,0),_mm_clmulepi64_si128(x,y,0x11))); }
    { __m128i x=_mm_xor_si128(_mm_loadu_si128((const __m128i_u *)(p+512+16*j)),_mm_loadu_si128((const __m128i_u *)(k->ph+16))), y=_mm_xor_si128(_mm_loadu_si128((const __m128i_u *)(p+576+16*j)),_mm_loadu_si128((const __m128i_u *)(k->ph+18))); a=_mm_xor_si128(a,_mm_xor_si128(_mm_clmulepi64_si128(x,y,0),_mm_clmulepi64_si128(x,y,0x11))); }
    { __m128i x=_mm_xor_si128(_mm_loadu_si128((const __m128i_u *)(p+640+16*j)),_mm_loadu_si128((const __m128i_u *)(k->ph+20))), y=_mm_xor_si128(_mm_loadu_si128((const __m128i_u *)(p+704+16*j)),_mm_loadu_si128((const __m128i_u *)(k->ph+22))); a=_mm_xor_si128(a,_mm_xor_si128(_mm_clmulepi64_si128(x,y,0),_mm_clmulepi64_si128(x,y,0x11))); }
    { __m128i x=_mm_xor_si128(_mm_loadu_si128((const __m128i_u *)(p+768+16*j)),_mm_loadu_si128((const __m128i_u *)(k->ph+24))), y=_mm_xor_si128(_mm_loadu_si128((const __m128i_u *)(p+832+16*j)),_mm_loadu_si128((const __m128i_u *)(k->ph+26))); a=_mm_xor_si128(a,_mm_xor_si128(_mm_clmulepi64_si128(x,y,0),_mm_clmulepi64_si128(x,y,0x11))); }
    { __m128i x=_mm_xor_si128(_mm_loadu_si128((const __m128i_u *)(p+896+16*j)),_mm_loadu_si128((const __m128i_u *)(k->ph+28))), y=_mm_xor_si128(_mm_loadu_si128((const __m128i_u *)(p+960+16*j)),_mm_loadu_si128((const __m128i_u *)(k->ph+30))); a=_mm_xor_si128(a,_mm_xor_si128(_mm_clmulepi64_si128(x,y,0),_mm_clmulepi64_si128(x,y,0x11))); }
    _mm_storeu_si128((__m128i_u *)(out+j),a); }
}
CH_T256 static inline void ch_region256(const chainhash_key *k,const uint8_t *p,ch_raw out[4]) {
    unsigned j; for(j=0;j<4;j+=2) {
    __m256i a=_mm256_setzero_si256();
    { __m256i x=_mm256_xor_si256(_mm256_loadu_si256((const __m256i_u *)(p+0+16*j)),_mm256_broadcastsi128_si256(_mm_loadu_si128((const __m128i_u *)(k->ph+0)))), y=_mm256_xor_si256(_mm256_loadu_si256((const __m256i_u *)(p+64+16*j)),_mm256_broadcastsi128_si256(_mm_loadu_si128((const __m128i_u *)(k->ph+2)))); a=_mm256_xor_si256(a,_mm256_xor_si256(_mm256_clmulepi64_epi128(x,y,0),_mm256_clmulepi64_epi128(x,y,0x11))); }
    { __m256i x=_mm256_xor_si256(_mm256_loadu_si256((const __m256i_u *)(p+128+16*j)),_mm256_broadcastsi128_si256(_mm_loadu_si128((const __m128i_u *)(k->ph+4)))), y=_mm256_xor_si256(_mm256_loadu_si256((const __m256i_u *)(p+192+16*j)),_mm256_broadcastsi128_si256(_mm_loadu_si128((const __m128i_u *)(k->ph+6)))); a=_mm256_xor_si256(a,_mm256_xor_si256(_mm256_clmulepi64_epi128(x,y,0),_mm256_clmulepi64_epi128(x,y,0x11))); }
    { __m256i x=_mm256_xor_si256(_mm256_loadu_si256((const __m256i_u *)(p+256+16*j)),_mm256_broadcastsi128_si256(_mm_loadu_si128((const __m128i_u *)(k->ph+8)))), y=_mm256_xor_si256(_mm256_loadu_si256((const __m256i_u *)(p+320+16*j)),_mm256_broadcastsi128_si256(_mm_loadu_si128((const __m128i_u *)(k->ph+10)))); a=_mm256_xor_si256(a,_mm256_xor_si256(_mm256_clmulepi64_epi128(x,y,0),_mm256_clmulepi64_epi128(x,y,0x11))); }
    { __m256i x=_mm256_xor_si256(_mm256_loadu_si256((const __m256i_u *)(p+384+16*j)),_mm256_broadcastsi128_si256(_mm_loadu_si128((const __m128i_u *)(k->ph+12)))), y=_mm256_xor_si256(_mm256_loadu_si256((const __m256i_u *)(p+448+16*j)),_mm256_broadcastsi128_si256(_mm_loadu_si128((const __m128i_u *)(k->ph+14)))); a=_mm256_xor_si256(a,_mm256_xor_si256(_mm256_clmulepi64_epi128(x,y,0),_mm256_clmulepi64_epi128(x,y,0x11))); }
    { __m256i x=_mm256_xor_si256(_mm256_loadu_si256((const __m256i_u *)(p+512+16*j)),_mm256_broadcastsi128_si256(_mm_loadu_si128((const __m128i_u *)(k->ph+16)))), y=_mm256_xor_si256(_mm256_loadu_si256((const __m256i_u *)(p+576+16*j)),_mm256_broadcastsi128_si256(_mm_loadu_si128((const __m128i_u *)(k->ph+18)))); a=_mm256_xor_si256(a,_mm256_xor_si256(_mm256_clmulepi64_epi128(x,y,0),_mm256_clmulepi64_epi128(x,y,0x11))); }
    { __m256i x=_mm256_xor_si256(_mm256_loadu_si256((const __m256i_u *)(p+640+16*j)),_mm256_broadcastsi128_si256(_mm_loadu_si128((const __m128i_u *)(k->ph+20)))), y=_mm256_xor_si256(_mm256_loadu_si256((const __m256i_u *)(p+704+16*j)),_mm256_broadcastsi128_si256(_mm_loadu_si128((const __m128i_u *)(k->ph+22)))); a=_mm256_xor_si256(a,_mm256_xor_si256(_mm256_clmulepi64_epi128(x,y,0),_mm256_clmulepi64_epi128(x,y,0x11))); }
    { __m256i x=_mm256_xor_si256(_mm256_loadu_si256((const __m256i_u *)(p+768+16*j)),_mm256_broadcastsi128_si256(_mm_loadu_si128((const __m128i_u *)(k->ph+24)))), y=_mm256_xor_si256(_mm256_loadu_si256((const __m256i_u *)(p+832+16*j)),_mm256_broadcastsi128_si256(_mm_loadu_si128((const __m128i_u *)(k->ph+26)))); a=_mm256_xor_si256(a,_mm256_xor_si256(_mm256_clmulepi64_epi128(x,y,0),_mm256_clmulepi64_epi128(x,y,0x11))); }
    { __m256i x=_mm256_xor_si256(_mm256_loadu_si256((const __m256i_u *)(p+896+16*j)),_mm256_broadcastsi128_si256(_mm_loadu_si128((const __m128i_u *)(k->ph+28)))), y=_mm256_xor_si256(_mm256_loadu_si256((const __m256i_u *)(p+960+16*j)),_mm256_broadcastsi128_si256(_mm_loadu_si128((const __m128i_u *)(k->ph+30)))); a=_mm256_xor_si256(a,_mm256_xor_si256(_mm256_clmulepi64_epi128(x,y,0),_mm256_clmulepi64_epi128(x,y,0x11))); }
    _mm256_storeu_si256((__m256i_u *)(out+j),a); }
}
CH_T512 static inline void ch_region512(const chainhash_key *k,const uint8_t *p,ch_raw out[4]) {
    unsigned j; for(j=0;j<4;j+=4) {
    __m512i a=_mm512_setzero_si512();
    { __m512i x=_mm512_xor_si512(_mm512_loadu_si512(p+0+16*j),_mm512_broadcast_i32x4(_mm_loadu_si128((const __m128i_u *)(k->ph+0)))), y=_mm512_xor_si512(_mm512_loadu_si512(p+64+16*j),_mm512_broadcast_i32x4(_mm_loadu_si128((const __m128i_u *)(k->ph+2)))); a=_mm512_xor_si512(a,_mm512_xor_si512(_mm512_clmulepi64_epi128(x,y,0),_mm512_clmulepi64_epi128(x,y,0x11))); }
    { __m512i x=_mm512_xor_si512(_mm512_loadu_si512(p+128+16*j),_mm512_broadcast_i32x4(_mm_loadu_si128((const __m128i_u *)(k->ph+4)))), y=_mm512_xor_si512(_mm512_loadu_si512(p+192+16*j),_mm512_broadcast_i32x4(_mm_loadu_si128((const __m128i_u *)(k->ph+6)))); a=_mm512_xor_si512(a,_mm512_xor_si512(_mm512_clmulepi64_epi128(x,y,0),_mm512_clmulepi64_epi128(x,y,0x11))); }
    { __m512i x=_mm512_xor_si512(_mm512_loadu_si512(p+256+16*j),_mm512_broadcast_i32x4(_mm_loadu_si128((const __m128i_u *)(k->ph+8)))), y=_mm512_xor_si512(_mm512_loadu_si512(p+320+16*j),_mm512_broadcast_i32x4(_mm_loadu_si128((const __m128i_u *)(k->ph+10)))); a=_mm512_xor_si512(a,_mm512_xor_si512(_mm512_clmulepi64_epi128(x,y,0),_mm512_clmulepi64_epi128(x,y,0x11))); }
    { __m512i x=_mm512_xor_si512(_mm512_loadu_si512(p+384+16*j),_mm512_broadcast_i32x4(_mm_loadu_si128((const __m128i_u *)(k->ph+12)))), y=_mm512_xor_si512(_mm512_loadu_si512(p+448+16*j),_mm512_broadcast_i32x4(_mm_loadu_si128((const __m128i_u *)(k->ph+14)))); a=_mm512_xor_si512(a,_mm512_xor_si512(_mm512_clmulepi64_epi128(x,y,0),_mm512_clmulepi64_epi128(x,y,0x11))); }
    { __m512i x=_mm512_xor_si512(_mm512_loadu_si512(p+512+16*j),_mm512_broadcast_i32x4(_mm_loadu_si128((const __m128i_u *)(k->ph+16)))), y=_mm512_xor_si512(_mm512_loadu_si512(p+576+16*j),_mm512_broadcast_i32x4(_mm_loadu_si128((const __m128i_u *)(k->ph+18)))); a=_mm512_xor_si512(a,_mm512_xor_si512(_mm512_clmulepi64_epi128(x,y,0),_mm512_clmulepi64_epi128(x,y,0x11))); }
    { __m512i x=_mm512_xor_si512(_mm512_loadu_si512(p+640+16*j),_mm512_broadcast_i32x4(_mm_loadu_si128((const __m128i_u *)(k->ph+20)))), y=_mm512_xor_si512(_mm512_loadu_si512(p+704+16*j),_mm512_broadcast_i32x4(_mm_loadu_si128((const __m128i_u *)(k->ph+22)))); a=_mm512_xor_si512(a,_mm512_xor_si512(_mm512_clmulepi64_epi128(x,y,0),_mm512_clmulepi64_epi128(x,y,0x11))); }
    { __m512i x=_mm512_xor_si512(_mm512_loadu_si512(p+768+16*j),_mm512_broadcast_i32x4(_mm_loadu_si128((const __m128i_u *)(k->ph+24)))), y=_mm512_xor_si512(_mm512_loadu_si512(p+832+16*j),_mm512_broadcast_i32x4(_mm_loadu_si128((const __m128i_u *)(k->ph+26)))); a=_mm512_xor_si512(a,_mm512_xor_si512(_mm512_clmulepi64_epi128(x,y,0),_mm512_clmulepi64_epi128(x,y,0x11))); }
    { __m512i x=_mm512_xor_si512(_mm512_loadu_si512(p+896+16*j),_mm512_broadcast_i32x4(_mm_loadu_si128((const __m128i_u *)(k->ph+28)))), y=_mm512_xor_si512(_mm512_loadu_si512(p+960+16*j),_mm512_broadcast_i32x4(_mm_loadu_si128((const __m128i_u *)(k->ph+30)))); a=_mm512_xor_si512(a,_mm512_xor_si512(_mm512_clmulepi64_epi128(x,y,0),_mm512_clmulepi64_epi128(x,y,0x11))); }
    _mm512_storeu_si512((out+j),a); }
}
/* Keep narrow live ranges bounded: the 16-register ISA cannot retain
 * sixteen PH keys, four raw states and four PH sums simultaneously. These
 * loads deliberately stay in the loop; each pattern serves all four lanes. */
CH_T128 static inline __m128i ch_key128(const uint64_t *p) {
    __m128i r; __asm__ volatile("vmovdqu {%1, %0|%0, %1}" : "=x"(r) : "m"(*(const __m128i_u *)p)); return r;
}
CH_T256 static inline __m256i ch_key256(const uint64_t *p) {
    __m256i r; __asm__ volatile("vbroadcasti128 {%1, %0|%0, %1}" : "=x"(r) : "m"(*(const __m128i_u *)p)); return r;
}
CH_T128 static inline __m128i ch_xor128(__m128i a,__m128i b,__m128i c) {
    __m128i r=_mm_xor_si128(a,_mm_xor_si128(b,c)); __asm__("" : "+x"(r)); return r;
}
CH_T256 static inline __m256i ch_xor256(__m256i a,__m256i b,__m256i c) {
    __m256i r=_mm256_xor_si256(a,_mm256_xor_si256(b,c)); __asm__("" : "+x"(r)); return r;
}
CH_T128 CH_INLINE uint64_t ch_bulk128_run(const chainhash_key *k,const uint8_t *p,size_t regions,size_t len,ch_raw *st,const int hint,const unsigned step,size_t dist) {
    __m128i s0=_mm_setzero_si128();
    __m128i s1=_mm_setzero_si128();
    __m128i s2=_mm_setzero_si128();
    __m128i s3=_mm_set_epi64x(0,(long long)len);
    const __m128i y=_mm_set_epi64x((long long)k->yh[4],(long long)k->yp[4]);
    if(st) { s0=_mm_loadu_si128((const __m128i_u *)st); s1=_mm_loadu_si128((const __m128i_u *)(st+1)); s2=_mm_loadu_si128((const __m128i_u *)(st+2)); s3=_mm_loadu_si128((const __m128i_u *)(st+3)); }
    do {
        CH_PREFETCH(p,regions,1024,hint,step,dist);
        __m128i u0=_mm_setzero_si128();
        __m128i u1=_mm_setzero_si128();
        __m128i u2=_mm_setzero_si128();
        __m128i u3=_mm_setzero_si128();
        { const __m128i ka=ch_key128(k->ph+0), kb=ch_key128(k->ph+2);
          { __m128i a=_mm_xor_si128(_mm_loadu_si128((const __m128i_u *)(p+0)),ka), b=_mm_xor_si128(_mm_loadu_si128((const __m128i_u *)(p+64)),kb); u0=ch_xor128(u0,_mm_clmulepi64_si128(a,b,0),_mm_clmulepi64_si128(a,b,0x11)); }
          { __m128i a=_mm_xor_si128(_mm_loadu_si128((const __m128i_u *)(p+16)),ka), b=_mm_xor_si128(_mm_loadu_si128((const __m128i_u *)(p+80)),kb); u1=ch_xor128(u1,_mm_clmulepi64_si128(a,b,0),_mm_clmulepi64_si128(a,b,0x11)); }
          { __m128i a=_mm_xor_si128(_mm_loadu_si128((const __m128i_u *)(p+32)),ka), b=_mm_xor_si128(_mm_loadu_si128((const __m128i_u *)(p+96)),kb); u2=ch_xor128(u2,_mm_clmulepi64_si128(a,b,0),_mm_clmulepi64_si128(a,b,0x11)); }
          { __m128i a=_mm_xor_si128(_mm_loadu_si128((const __m128i_u *)(p+48)),ka), b=_mm_xor_si128(_mm_loadu_si128((const __m128i_u *)(p+112)),kb); u3=ch_xor128(u3,_mm_clmulepi64_si128(a,b,0),_mm_clmulepi64_si128(a,b,0x11)); }
        }
        { const __m128i ka=ch_key128(k->ph+4), kb=ch_key128(k->ph+6);
          { __m128i a=_mm_xor_si128(_mm_loadu_si128((const __m128i_u *)(p+128)),ka), b=_mm_xor_si128(_mm_loadu_si128((const __m128i_u *)(p+192)),kb); u0=ch_xor128(u0,_mm_clmulepi64_si128(a,b,0),_mm_clmulepi64_si128(a,b,0x11)); }
          { __m128i a=_mm_xor_si128(_mm_loadu_si128((const __m128i_u *)(p+144)),ka), b=_mm_xor_si128(_mm_loadu_si128((const __m128i_u *)(p+208)),kb); u1=ch_xor128(u1,_mm_clmulepi64_si128(a,b,0),_mm_clmulepi64_si128(a,b,0x11)); }
          { __m128i a=_mm_xor_si128(_mm_loadu_si128((const __m128i_u *)(p+160)),ka), b=_mm_xor_si128(_mm_loadu_si128((const __m128i_u *)(p+224)),kb); u2=ch_xor128(u2,_mm_clmulepi64_si128(a,b,0),_mm_clmulepi64_si128(a,b,0x11)); }
          { __m128i a=_mm_xor_si128(_mm_loadu_si128((const __m128i_u *)(p+176)),ka), b=_mm_xor_si128(_mm_loadu_si128((const __m128i_u *)(p+240)),kb); u3=ch_xor128(u3,_mm_clmulepi64_si128(a,b,0),_mm_clmulepi64_si128(a,b,0x11)); }
        }
        { const __m128i ka=ch_key128(k->ph+8), kb=ch_key128(k->ph+10);
          { __m128i a=_mm_xor_si128(_mm_loadu_si128((const __m128i_u *)(p+256)),ka), b=_mm_xor_si128(_mm_loadu_si128((const __m128i_u *)(p+320)),kb); u0=ch_xor128(u0,_mm_clmulepi64_si128(a,b,0),_mm_clmulepi64_si128(a,b,0x11)); }
          { __m128i a=_mm_xor_si128(_mm_loadu_si128((const __m128i_u *)(p+272)),ka), b=_mm_xor_si128(_mm_loadu_si128((const __m128i_u *)(p+336)),kb); u1=ch_xor128(u1,_mm_clmulepi64_si128(a,b,0),_mm_clmulepi64_si128(a,b,0x11)); }
          { __m128i a=_mm_xor_si128(_mm_loadu_si128((const __m128i_u *)(p+288)),ka), b=_mm_xor_si128(_mm_loadu_si128((const __m128i_u *)(p+352)),kb); u2=ch_xor128(u2,_mm_clmulepi64_si128(a,b,0),_mm_clmulepi64_si128(a,b,0x11)); }
          { __m128i a=_mm_xor_si128(_mm_loadu_si128((const __m128i_u *)(p+304)),ka), b=_mm_xor_si128(_mm_loadu_si128((const __m128i_u *)(p+368)),kb); u3=ch_xor128(u3,_mm_clmulepi64_si128(a,b,0),_mm_clmulepi64_si128(a,b,0x11)); }
        }
        { const __m128i ka=ch_key128(k->ph+12), kb=ch_key128(k->ph+14);
          { __m128i a=_mm_xor_si128(_mm_loadu_si128((const __m128i_u *)(p+384)),ka), b=_mm_xor_si128(_mm_loadu_si128((const __m128i_u *)(p+448)),kb); u0=ch_xor128(u0,_mm_clmulepi64_si128(a,b,0),_mm_clmulepi64_si128(a,b,0x11)); }
          { __m128i a=_mm_xor_si128(_mm_loadu_si128((const __m128i_u *)(p+400)),ka), b=_mm_xor_si128(_mm_loadu_si128((const __m128i_u *)(p+464)),kb); u1=ch_xor128(u1,_mm_clmulepi64_si128(a,b,0),_mm_clmulepi64_si128(a,b,0x11)); }
          { __m128i a=_mm_xor_si128(_mm_loadu_si128((const __m128i_u *)(p+416)),ka), b=_mm_xor_si128(_mm_loadu_si128((const __m128i_u *)(p+480)),kb); u2=ch_xor128(u2,_mm_clmulepi64_si128(a,b,0),_mm_clmulepi64_si128(a,b,0x11)); }
          { __m128i a=_mm_xor_si128(_mm_loadu_si128((const __m128i_u *)(p+432)),ka), b=_mm_xor_si128(_mm_loadu_si128((const __m128i_u *)(p+496)),kb); u3=ch_xor128(u3,_mm_clmulepi64_si128(a,b,0),_mm_clmulepi64_si128(a,b,0x11)); }
        }
        { const __m128i ka=ch_key128(k->ph+16), kb=ch_key128(k->ph+18);
          { __m128i a=_mm_xor_si128(_mm_loadu_si128((const __m128i_u *)(p+512)),ka), b=_mm_xor_si128(_mm_loadu_si128((const __m128i_u *)(p+576)),kb); u0=ch_xor128(u0,_mm_clmulepi64_si128(a,b,0),_mm_clmulepi64_si128(a,b,0x11)); }
          { __m128i a=_mm_xor_si128(_mm_loadu_si128((const __m128i_u *)(p+528)),ka), b=_mm_xor_si128(_mm_loadu_si128((const __m128i_u *)(p+592)),kb); u1=ch_xor128(u1,_mm_clmulepi64_si128(a,b,0),_mm_clmulepi64_si128(a,b,0x11)); }
          { __m128i a=_mm_xor_si128(_mm_loadu_si128((const __m128i_u *)(p+544)),ka), b=_mm_xor_si128(_mm_loadu_si128((const __m128i_u *)(p+608)),kb); u2=ch_xor128(u2,_mm_clmulepi64_si128(a,b,0),_mm_clmulepi64_si128(a,b,0x11)); }
          { __m128i a=_mm_xor_si128(_mm_loadu_si128((const __m128i_u *)(p+560)),ka), b=_mm_xor_si128(_mm_loadu_si128((const __m128i_u *)(p+624)),kb); u3=ch_xor128(u3,_mm_clmulepi64_si128(a,b,0),_mm_clmulepi64_si128(a,b,0x11)); }
        }
        { const __m128i ka=ch_key128(k->ph+20), kb=ch_key128(k->ph+22);
          { __m128i a=_mm_xor_si128(_mm_loadu_si128((const __m128i_u *)(p+640)),ka), b=_mm_xor_si128(_mm_loadu_si128((const __m128i_u *)(p+704)),kb); u0=ch_xor128(u0,_mm_clmulepi64_si128(a,b,0),_mm_clmulepi64_si128(a,b,0x11)); }
          { __m128i a=_mm_xor_si128(_mm_loadu_si128((const __m128i_u *)(p+656)),ka), b=_mm_xor_si128(_mm_loadu_si128((const __m128i_u *)(p+720)),kb); u1=ch_xor128(u1,_mm_clmulepi64_si128(a,b,0),_mm_clmulepi64_si128(a,b,0x11)); }
          { __m128i a=_mm_xor_si128(_mm_loadu_si128((const __m128i_u *)(p+672)),ka), b=_mm_xor_si128(_mm_loadu_si128((const __m128i_u *)(p+736)),kb); u2=ch_xor128(u2,_mm_clmulepi64_si128(a,b,0),_mm_clmulepi64_si128(a,b,0x11)); }
          { __m128i a=_mm_xor_si128(_mm_loadu_si128((const __m128i_u *)(p+688)),ka), b=_mm_xor_si128(_mm_loadu_si128((const __m128i_u *)(p+752)),kb); u3=ch_xor128(u3,_mm_clmulepi64_si128(a,b,0),_mm_clmulepi64_si128(a,b,0x11)); }
        }
        { const __m128i ka=ch_key128(k->ph+24), kb=ch_key128(k->ph+26);
          { __m128i a=_mm_xor_si128(_mm_loadu_si128((const __m128i_u *)(p+768)),ka), b=_mm_xor_si128(_mm_loadu_si128((const __m128i_u *)(p+832)),kb); u0=ch_xor128(u0,_mm_clmulepi64_si128(a,b,0),_mm_clmulepi64_si128(a,b,0x11)); }
          { __m128i a=_mm_xor_si128(_mm_loadu_si128((const __m128i_u *)(p+784)),ka), b=_mm_xor_si128(_mm_loadu_si128((const __m128i_u *)(p+848)),kb); u1=ch_xor128(u1,_mm_clmulepi64_si128(a,b,0),_mm_clmulepi64_si128(a,b,0x11)); }
          { __m128i a=_mm_xor_si128(_mm_loadu_si128((const __m128i_u *)(p+800)),ka), b=_mm_xor_si128(_mm_loadu_si128((const __m128i_u *)(p+864)),kb); u2=ch_xor128(u2,_mm_clmulepi64_si128(a,b,0),_mm_clmulepi64_si128(a,b,0x11)); }
          { __m128i a=_mm_xor_si128(_mm_loadu_si128((const __m128i_u *)(p+816)),ka), b=_mm_xor_si128(_mm_loadu_si128((const __m128i_u *)(p+880)),kb); u3=ch_xor128(u3,_mm_clmulepi64_si128(a,b,0),_mm_clmulepi64_si128(a,b,0x11)); }
        }
        { const __m128i ka=ch_key128(k->ph+28), kb=ch_key128(k->ph+30);
          { __m128i a=_mm_xor_si128(_mm_loadu_si128((const __m128i_u *)(p+896)),ka), b=_mm_xor_si128(_mm_loadu_si128((const __m128i_u *)(p+960)),kb); u0=ch_xor128(u0,_mm_clmulepi64_si128(a,b,0),_mm_clmulepi64_si128(a,b,0x11)); }
          { __m128i a=_mm_xor_si128(_mm_loadu_si128((const __m128i_u *)(p+912)),ka), b=_mm_xor_si128(_mm_loadu_si128((const __m128i_u *)(p+976)),kb); u1=ch_xor128(u1,_mm_clmulepi64_si128(a,b,0),_mm_clmulepi64_si128(a,b,0x11)); }
          { __m128i a=_mm_xor_si128(_mm_loadu_si128((const __m128i_u *)(p+928)),ka), b=_mm_xor_si128(_mm_loadu_si128((const __m128i_u *)(p+992)),kb); u2=ch_xor128(u2,_mm_clmulepi64_si128(a,b,0),_mm_clmulepi64_si128(a,b,0x11)); }
          { __m128i a=_mm_xor_si128(_mm_loadu_si128((const __m128i_u *)(p+944)),ka), b=_mm_xor_si128(_mm_loadu_si128((const __m128i_u *)(p+1008)),kb); u3=ch_xor128(u3,_mm_clmulepi64_si128(a,b,0),_mm_clmulepi64_si128(a,b,0x11)); }
        }
        s0=ch_xor128(u0,_mm_clmulepi64_si128(s0,y,0),_mm_clmulepi64_si128(s0,y,0x11));
        s1=ch_xor128(u1,_mm_clmulepi64_si128(s1,y,0),_mm_clmulepi64_si128(s1,y,0x11));
        s2=ch_xor128(u2,_mm_clmulepi64_si128(s2,y,0),_mm_clmulepi64_si128(s2,y,0x11));
        s3=ch_xor128(u3,_mm_clmulepi64_si128(s3,y,0),_mm_clmulepi64_si128(s3,y,0x11));
        p+=1024;
    } while(--regions);
    if(st) { _mm_storeu_si128((__m128i_u *)st,s0); _mm_storeu_si128((__m128i_u *)(st+1),s1); _mm_storeu_si128((__m128i_u *)(st+2),s2); _mm_storeu_si128((__m128i_u *)(st+3),s3); return 0; }
    __m128i acc=_mm_setzero_si128(),pw;
    pw=_mm_set_epi64x((long long)k->yh[3],(long long)k->yp[3]);
    acc=_mm_xor_si128(acc,_mm_xor_si128(_mm_clmulepi64_si128(s0,pw,0),_mm_clmulepi64_si128(s0,pw,0x11)));
    pw=_mm_set_epi64x((long long)k->yh[2],(long long)k->yp[2]);
    acc=_mm_xor_si128(acc,_mm_xor_si128(_mm_clmulepi64_si128(s1,pw,0),_mm_clmulepi64_si128(s1,pw,0x11)));
    pw=_mm_set_epi64x((long long)k->yh[1],(long long)k->yp[1]);
    acc=_mm_xor_si128(acc,_mm_xor_si128(_mm_clmulepi64_si128(s2,pw,0),_mm_clmulepi64_si128(s2,pw,0x11)));
    pw=_mm_set_epi64x((long long)k->yh[0],(long long)k->yp[0]);
    acc=_mm_xor_si128(acc,_mm_xor_si128(_mm_clmulepi64_si128(s3,pw,0),_mm_clmulepi64_si128(s3,pw,0x11)));
    __m128i a=acc;
    return ch_lane0(ch_vreduce(a));
}
CH_T128 static uint64_t ch_bulk128(const chainhash_key *k,const uint8_t *p,size_t regions,size_t len,ch_raw *st) { return ch_bulk128_run(k,p,regions,len,st,CH_PF_OFF,64,0); }
CH_T128 static uint64_t ch_bulk128_pf(const chainhash_key *k,const uint8_t *p,size_t regions,size_t len,ch_raw *st,int hint,unsigned step,size_t dist) { CH_PF_SWITCH(ch_bulk128,k,p,regions,len,st) }
CH_T256 CH_INLINE uint64_t ch_bulk256_run(const chainhash_key *k,const uint8_t *p,size_t regions,size_t len,ch_raw *st,const int hint,const unsigned step,size_t dist) {
    __m256i s0=_mm256_setzero_si256();
    __m256i s1=_mm256_set_epi64x(0,(long long)len,0,0);
    const __m256i y=_mm256_set_epi64x((long long)k->yh[4],(long long)k->yp[4],(long long)k->yh[4],(long long)k->yp[4]);
    if(st) { s0=_mm256_loadu_si256((const __m256i_u *)st); s1=_mm256_loadu_si256((const __m256i_u *)(st+2)); }
    do {
        CH_PREFETCH(p,regions,1024,hint,step,dist);
        __m256i u0=_mm256_setzero_si256();
        __m256i u1=_mm256_setzero_si256();
        { const __m256i ka=ch_key256(k->ph+0), kb=ch_key256(k->ph+2);
          { __m256i a=_mm256_xor_si256(_mm256_loadu_si256((const __m256i_u *)(p+0)),ka), b=_mm256_xor_si256(_mm256_loadu_si256((const __m256i_u *)(p+64)),kb); u0=ch_xor256(u0,_mm256_clmulepi64_epi128(a,b,0),_mm256_clmulepi64_epi128(a,b,0x11)); }
          { __m256i a=_mm256_xor_si256(_mm256_loadu_si256((const __m256i_u *)(p+32)),ka), b=_mm256_xor_si256(_mm256_loadu_si256((const __m256i_u *)(p+96)),kb); u1=ch_xor256(u1,_mm256_clmulepi64_epi128(a,b,0),_mm256_clmulepi64_epi128(a,b,0x11)); }
        }
        { const __m256i ka=ch_key256(k->ph+4), kb=ch_key256(k->ph+6);
          { __m256i a=_mm256_xor_si256(_mm256_loadu_si256((const __m256i_u *)(p+128)),ka), b=_mm256_xor_si256(_mm256_loadu_si256((const __m256i_u *)(p+192)),kb); u0=ch_xor256(u0,_mm256_clmulepi64_epi128(a,b,0),_mm256_clmulepi64_epi128(a,b,0x11)); }
          { __m256i a=_mm256_xor_si256(_mm256_loadu_si256((const __m256i_u *)(p+160)),ka), b=_mm256_xor_si256(_mm256_loadu_si256((const __m256i_u *)(p+224)),kb); u1=ch_xor256(u1,_mm256_clmulepi64_epi128(a,b,0),_mm256_clmulepi64_epi128(a,b,0x11)); }
        }
        { const __m256i ka=ch_key256(k->ph+8), kb=ch_key256(k->ph+10);
          { __m256i a=_mm256_xor_si256(_mm256_loadu_si256((const __m256i_u *)(p+256)),ka), b=_mm256_xor_si256(_mm256_loadu_si256((const __m256i_u *)(p+320)),kb); u0=ch_xor256(u0,_mm256_clmulepi64_epi128(a,b,0),_mm256_clmulepi64_epi128(a,b,0x11)); }
          { __m256i a=_mm256_xor_si256(_mm256_loadu_si256((const __m256i_u *)(p+288)),ka), b=_mm256_xor_si256(_mm256_loadu_si256((const __m256i_u *)(p+352)),kb); u1=ch_xor256(u1,_mm256_clmulepi64_epi128(a,b,0),_mm256_clmulepi64_epi128(a,b,0x11)); }
        }
        { const __m256i ka=ch_key256(k->ph+12), kb=ch_key256(k->ph+14);
          { __m256i a=_mm256_xor_si256(_mm256_loadu_si256((const __m256i_u *)(p+384)),ka), b=_mm256_xor_si256(_mm256_loadu_si256((const __m256i_u *)(p+448)),kb); u0=ch_xor256(u0,_mm256_clmulepi64_epi128(a,b,0),_mm256_clmulepi64_epi128(a,b,0x11)); }
          { __m256i a=_mm256_xor_si256(_mm256_loadu_si256((const __m256i_u *)(p+416)),ka), b=_mm256_xor_si256(_mm256_loadu_si256((const __m256i_u *)(p+480)),kb); u1=ch_xor256(u1,_mm256_clmulepi64_epi128(a,b,0),_mm256_clmulepi64_epi128(a,b,0x11)); }
        }
        { const __m256i ka=ch_key256(k->ph+16), kb=ch_key256(k->ph+18);
          { __m256i a=_mm256_xor_si256(_mm256_loadu_si256((const __m256i_u *)(p+512)),ka), b=_mm256_xor_si256(_mm256_loadu_si256((const __m256i_u *)(p+576)),kb); u0=ch_xor256(u0,_mm256_clmulepi64_epi128(a,b,0),_mm256_clmulepi64_epi128(a,b,0x11)); }
          { __m256i a=_mm256_xor_si256(_mm256_loadu_si256((const __m256i_u *)(p+544)),ka), b=_mm256_xor_si256(_mm256_loadu_si256((const __m256i_u *)(p+608)),kb); u1=ch_xor256(u1,_mm256_clmulepi64_epi128(a,b,0),_mm256_clmulepi64_epi128(a,b,0x11)); }
        }
        { const __m256i ka=ch_key256(k->ph+20), kb=ch_key256(k->ph+22);
          { __m256i a=_mm256_xor_si256(_mm256_loadu_si256((const __m256i_u *)(p+640)),ka), b=_mm256_xor_si256(_mm256_loadu_si256((const __m256i_u *)(p+704)),kb); u0=ch_xor256(u0,_mm256_clmulepi64_epi128(a,b,0),_mm256_clmulepi64_epi128(a,b,0x11)); }
          { __m256i a=_mm256_xor_si256(_mm256_loadu_si256((const __m256i_u *)(p+672)),ka), b=_mm256_xor_si256(_mm256_loadu_si256((const __m256i_u *)(p+736)),kb); u1=ch_xor256(u1,_mm256_clmulepi64_epi128(a,b,0),_mm256_clmulepi64_epi128(a,b,0x11)); }
        }
        { const __m256i ka=ch_key256(k->ph+24), kb=ch_key256(k->ph+26);
          { __m256i a=_mm256_xor_si256(_mm256_loadu_si256((const __m256i_u *)(p+768)),ka), b=_mm256_xor_si256(_mm256_loadu_si256((const __m256i_u *)(p+832)),kb); u0=ch_xor256(u0,_mm256_clmulepi64_epi128(a,b,0),_mm256_clmulepi64_epi128(a,b,0x11)); }
          { __m256i a=_mm256_xor_si256(_mm256_loadu_si256((const __m256i_u *)(p+800)),ka), b=_mm256_xor_si256(_mm256_loadu_si256((const __m256i_u *)(p+864)),kb); u1=ch_xor256(u1,_mm256_clmulepi64_epi128(a,b,0),_mm256_clmulepi64_epi128(a,b,0x11)); }
        }
        { const __m256i ka=ch_key256(k->ph+28), kb=ch_key256(k->ph+30);
          { __m256i a=_mm256_xor_si256(_mm256_loadu_si256((const __m256i_u *)(p+896)),ka), b=_mm256_xor_si256(_mm256_loadu_si256((const __m256i_u *)(p+960)),kb); u0=ch_xor256(u0,_mm256_clmulepi64_epi128(a,b,0),_mm256_clmulepi64_epi128(a,b,0x11)); }
          { __m256i a=_mm256_xor_si256(_mm256_loadu_si256((const __m256i_u *)(p+928)),ka), b=_mm256_xor_si256(_mm256_loadu_si256((const __m256i_u *)(p+992)),kb); u1=ch_xor256(u1,_mm256_clmulepi64_epi128(a,b,0),_mm256_clmulepi64_epi128(a,b,0x11)); }
        }
        s0=ch_xor256(u0,_mm256_clmulepi64_epi128(s0,y,0),_mm256_clmulepi64_epi128(s0,y,0x11));
        s1=ch_xor256(u1,_mm256_clmulepi64_epi128(s1,y,0),_mm256_clmulepi64_epi128(s1,y,0x11));
        p+=1024;
    } while(--regions);
    if(st) { _mm256_storeu_si256((__m256i_u *)st,s0); _mm256_storeu_si256((__m256i_u *)(st+2),s1); return 0; }
    __m256i acc=_mm256_setzero_si256(),pw;
    pw=_mm256_set_epi64x((long long)k->yh[2],(long long)k->yp[2],(long long)k->yh[3],(long long)k->yp[3]);
    acc=_mm256_xor_si256(acc,_mm256_xor_si256(_mm256_clmulepi64_epi128(s0,pw,0),_mm256_clmulepi64_epi128(s0,pw,0x11)));
    pw=_mm256_set_epi64x((long long)k->yh[0],(long long)k->yp[0],(long long)k->yh[1],(long long)k->yp[1]);
    acc=_mm256_xor_si256(acc,_mm256_xor_si256(_mm256_clmulepi64_epi128(s1,pw,0),_mm256_clmulepi64_epi128(s1,pw,0x11)));
    __m128i a=_mm_xor_si128(_mm256_castsi256_si128(acc),_mm256_extracti128_si256(acc,1));
    return ch_lane0(ch_vreduce(a));
}
CH_T256 static uint64_t ch_bulk256(const chainhash_key *k,const uint8_t *p,size_t regions,size_t len,ch_raw *st) { return ch_bulk256_run(k,p,regions,len,st,CH_PF_OFF,64,0); }
CH_T256 static uint64_t ch_bulk256_pf(const chainhash_key *k,const uint8_t *p,size_t regions,size_t len,ch_raw *st,int hint,unsigned step,size_t dist) { CH_PF_SWITCH(ch_bulk256,k,p,regions,len,st) }
/* The only cross-lane fold is AFTER all complete regions. */
CH_T512 static inline uint64_t ch_fold512(__m512i s,const chainhash_key *k) {
    __m512i powers=_mm512_set_epi64((long long)k->yh[0],(long long)k->yp[0],(long long)k->yh[1],(long long)k->yp[1],(long long)k->yh[2],(long long)k->yp[2],(long long)k->yh[3],(long long)k->yp[3]);
    s=_mm512_xor_si512(_mm512_clmulepi64_epi128(s,powers,0),_mm512_clmulepi64_epi128(s,powers,0x11));
    __m256i h=_mm256_xor_si256(_mm512_castsi512_si256(s),_mm512_extracti64x4_epi64(s,1));
    __m128i q=_mm_xor_si128(_mm256_castsi256_si128(h),_mm256_extracti128_si256(h,1));
    ch_raw r; _mm_storeu_si128((__m128i_u *)&r,q); return ch_reduce(r);
}
/* Tail loads never cross the input object: the last partial chunk is read with
 * masked byte loads (bytes past the input are zero and are not accessed); both
 * keyed multiplicands are masked by first-word presence. Horner weights act on all
 * four lanes in two vector products; they are gathered from the key in registers.
 * No vector load reads back narrower stores (store-forwarding stalls). */
CH_T512 static uint64_t ch_tail512(const chainhash_key *k,const uint8_t *p,size_t n,uint64_t leading) {
    /* Lane j of `lanes` takes weight (yp[e],yh[e]), e=lanes-1-j, and e=0 past the last lane. */
    static const uint64_t widx[4][8]={{0,8,0,8,0,8,0,8},{1,9,0,8,0,8,0,8},{2,10,1,9,0,8,0,8},{3,11,2,10,1,9,0,8}};
    __m512i acc=_mm512_setzero_si512(); unsigned c=0,lanes=ch_lanes(n); size_t rem=n;
    while(rem) {
        __m512i a,b; size_t take=rem<128?rem:128;
        if(take==128) { a=_mm512_loadu_si512(p); b=_mm512_loadu_si512(p+64); }
        else { a=_mm512_maskz_loadu_epi8(take>=64?~(__mmask64)0:((__mmask64)1<<take)-1,p);
               b=_mm512_maskz_loadu_epi8(take<=64?(__mmask64)0:((__mmask64)1<<(take-64))-1,p+64); }
        a=_mm512_xor_si512(a,_mm512_broadcast_i32x4(_mm_loadu_si128((const __m128i_u *)(k->ph+4*c))));
        b=_mm512_xor_si512(b,_mm512_broadcast_i32x4(_mm_loadu_si128((const __m128i_u *)(k->ph+4*c+2))));
        if(take<64) { __mmask8 mask=(__mmask8)((1u<<((take+7)/8))-1); a=_mm512_maskz_mov_epi64(mask,a); b=_mm512_maskz_mov_epi64(mask,b); }
        acc=_mm512_ternarylogic_epi64(acc,_mm512_clmulepi64_epi128(a,b,0),_mm512_clmulepi64_epi128(a,b,0x11),0x96);
        rem-=take; p+=take; ++c;
    }
    __m512i pw=_mm512_permutex2var_epi64(_mm512_loadu_si512(k->yp),_mm512_loadu_si512(widx[lanes-1]),_mm512_loadu_si512(k->yh));
    acc=_mm512_xor_si512(_mm512_clmulepi64_epi128(acc,pw,0),_mm512_clmulepi64_epi128(acc,pw,0x11));
    __m256i h=_mm256_xor_si256(_mm512_castsi512_si256(acc),_mm512_extracti64x4_epi64(acc,1));
    __m128i v=_mm_xor_si128(_mm256_castsi256_si128(h),_mm256_extracti128_si256(h,1));
    v=_mm_xor_si128(v,_mm_clmulepi64_si128(_mm_set_epi64x(0,(long long)leading),_mm_set_epi64x(0,(long long)k->yp[lanes]),0));
    return ch_lane0(ch_vreduce(v));
}
CH_T512 CH_INLINE uint64_t ch_bulk512_run(const chainhash_key *k,const uint8_t *p,size_t regions,size_t len,ch_raw *st,const int hint,const unsigned step,size_t dist) {
    __m512i s=_mm512_set_epi64(0,(long long)len,0,0,0,0,0,0);
    const __m512i y=_mm512_broadcast_i32x4(_mm_set_epi64x((long long)k->yh[4],(long long)k->yp[4]));
    const __m512i a0=_mm512_broadcast_i32x4(_mm_loadu_si128((const __m128i_u *)(k->ph+0))), b0=_mm512_broadcast_i32x4(_mm_loadu_si128((const __m128i_u *)(k->ph+2)));
    const __m512i a1=_mm512_broadcast_i32x4(_mm_loadu_si128((const __m128i_u *)(k->ph+4))), b1=_mm512_broadcast_i32x4(_mm_loadu_si128((const __m128i_u *)(k->ph+6)));
    const __m512i a2=_mm512_broadcast_i32x4(_mm_loadu_si128((const __m128i_u *)(k->ph+8))), b2=_mm512_broadcast_i32x4(_mm_loadu_si128((const __m128i_u *)(k->ph+10)));
    const __m512i a3=_mm512_broadcast_i32x4(_mm_loadu_si128((const __m128i_u *)(k->ph+12))), b3=_mm512_broadcast_i32x4(_mm_loadu_si128((const __m128i_u *)(k->ph+14)));
    const __m512i a4=_mm512_broadcast_i32x4(_mm_loadu_si128((const __m128i_u *)(k->ph+16))), b4=_mm512_broadcast_i32x4(_mm_loadu_si128((const __m128i_u *)(k->ph+18)));
    const __m512i a5=_mm512_broadcast_i32x4(_mm_loadu_si128((const __m128i_u *)(k->ph+20))), b5=_mm512_broadcast_i32x4(_mm_loadu_si128((const __m128i_u *)(k->ph+22)));
    const __m512i a6=_mm512_broadcast_i32x4(_mm_loadu_si128((const __m128i_u *)(k->ph+24))), b6=_mm512_broadcast_i32x4(_mm_loadu_si128((const __m128i_u *)(k->ph+26)));
    const __m512i a7=_mm512_broadcast_i32x4(_mm_loadu_si128((const __m128i_u *)(k->ph+28))), b7=_mm512_broadcast_i32x4(_mm_loadu_si128((const __m128i_u *)(k->ph+30)));
    if(st) s=_mm512_loadu_si512(st);
    do {
        CH_PREFETCH(p,regions,1024,hint,step,dist);
        __m512i u0,u1,u2,u3;
        { __m512i a=_mm512_xor_si512(_mm512_loadu_si512(p+0),a0), b=_mm512_xor_si512(_mm512_loadu_si512(p+64),b0);
          u0=_mm512_xor_si512(_mm512_clmulepi64_epi128(a,b,0),_mm512_clmulepi64_epi128(a,b,0x11)); }
        { __m512i a=_mm512_xor_si512(_mm512_loadu_si512(p+128),a1), b=_mm512_xor_si512(_mm512_loadu_si512(p+192),b1);
          u1=_mm512_xor_si512(_mm512_clmulepi64_epi128(a,b,0),_mm512_clmulepi64_epi128(a,b,0x11)); }
        { __m512i a=_mm512_xor_si512(_mm512_loadu_si512(p+256),a2), b=_mm512_xor_si512(_mm512_loadu_si512(p+320),b2);
          u2=_mm512_xor_si512(_mm512_clmulepi64_epi128(a,b,0),_mm512_clmulepi64_epi128(a,b,0x11)); }
        { __m512i a=_mm512_xor_si512(_mm512_loadu_si512(p+384),a3), b=_mm512_xor_si512(_mm512_loadu_si512(p+448),b3);
          u3=_mm512_xor_si512(_mm512_clmulepi64_epi128(a,b,0),_mm512_clmulepi64_epi128(a,b,0x11)); }
        { __m512i a=_mm512_xor_si512(_mm512_loadu_si512(p+512),a4), b=_mm512_xor_si512(_mm512_loadu_si512(p+576),b4);
          u0=_mm512_ternarylogic_epi64(u0,_mm512_clmulepi64_epi128(a,b,0),_mm512_clmulepi64_epi128(a,b,0x11),0x96); }
        { __m512i a=_mm512_xor_si512(_mm512_loadu_si512(p+640),a5), b=_mm512_xor_si512(_mm512_loadu_si512(p+704),b5);
          u1=_mm512_ternarylogic_epi64(u1,_mm512_clmulepi64_epi128(a,b,0),_mm512_clmulepi64_epi128(a,b,0x11),0x96); }
        { __m512i a=_mm512_xor_si512(_mm512_loadu_si512(p+768),a6), b=_mm512_xor_si512(_mm512_loadu_si512(p+832),b6);
          u2=_mm512_ternarylogic_epi64(u2,_mm512_clmulepi64_epi128(a,b,0),_mm512_clmulepi64_epi128(a,b,0x11),0x96); }
        { __m512i a=_mm512_xor_si512(_mm512_loadu_si512(p+896),a7), b=_mm512_xor_si512(_mm512_loadu_si512(p+960),b7);
          u3=_mm512_ternarylogic_epi64(u3,_mm512_clmulepi64_epi128(a,b,0),_mm512_clmulepi64_epi128(a,b,0x11),0x96); }
        s=_mm512_ternarylogic_epi64(_mm512_clmulepi64_epi128(s,y,0),_mm512_clmulepi64_epi128(s,y,0x11),_mm512_xor_si512(_mm512_ternarylogic_epi64(u0,u1,u2,0x96),u3),0x96);
        p+=1024;
    } while(--regions);
    if(st) { _mm512_storeu_si512(st,s); return 0; }
    return ch_fold512(s,k);
}
CH_T512 static uint64_t ch_bulk512(const chainhash_key *k,const uint8_t *p,size_t regions,size_t len,ch_raw *st) { return ch_bulk512_run(k,p,regions,len,st,CH_PF_OFF,64,0); }
CH_T512 static uint64_t ch_bulk512_pf(const chainhash_key *k,const uint8_t *p,size_t regions,size_t len,ch_raw *st,int hint,unsigned step,size_t dist) { CH_PF_SWITCH(ch_bulk512,k,p,regions,len,st) }
#endif

#ifdef CH_ARM
CH_NBEGIN
static inline void ch_region_neon(const chainhash_key *k,const uint8_t *p,ch_raw out[4]) {
    unsigned j; for(j=0;j<4;j++) { uint64x2_t s=vdupq_n_u64(0);
    { uint64x2_t a=veorq_u64(ch_ld(p+0+16*j),vld1q_u64(k->ph+0)), b=veorq_u64(ch_ld(p+64+16*j),vld1q_u64(k->ph+2)); s=ch_xor3(s,ch_ll(a,b),ch_hh(a,b)); }
    { uint64x2_t a=veorq_u64(ch_ld(p+128+16*j),vld1q_u64(k->ph+4)), b=veorq_u64(ch_ld(p+192+16*j),vld1q_u64(k->ph+6)); s=ch_xor3(s,ch_ll(a,b),ch_hh(a,b)); }
    { uint64x2_t a=veorq_u64(ch_ld(p+256+16*j),vld1q_u64(k->ph+8)), b=veorq_u64(ch_ld(p+320+16*j),vld1q_u64(k->ph+10)); s=ch_xor3(s,ch_ll(a,b),ch_hh(a,b)); }
    { uint64x2_t a=veorq_u64(ch_ld(p+384+16*j),vld1q_u64(k->ph+12)), b=veorq_u64(ch_ld(p+448+16*j),vld1q_u64(k->ph+14)); s=ch_xor3(s,ch_ll(a,b),ch_hh(a,b)); }
    { uint64x2_t a=veorq_u64(ch_ld(p+512+16*j),vld1q_u64(k->ph+16)), b=veorq_u64(ch_ld(p+576+16*j),vld1q_u64(k->ph+18)); s=ch_xor3(s,ch_ll(a,b),ch_hh(a,b)); }
    { uint64x2_t a=veorq_u64(ch_ld(p+640+16*j),vld1q_u64(k->ph+20)), b=veorq_u64(ch_ld(p+704+16*j),vld1q_u64(k->ph+22)); s=ch_xor3(s,ch_ll(a,b),ch_hh(a,b)); }
    { uint64x2_t a=veorq_u64(ch_ld(p+768+16*j),vld1q_u64(k->ph+24)), b=veorq_u64(ch_ld(p+832+16*j),vld1q_u64(k->ph+26)); s=ch_xor3(s,ch_ll(a,b),ch_hh(a,b)); }
    { uint64x2_t a=veorq_u64(ch_ld(p+896+16*j),vld1q_u64(k->ph+28)), b=veorq_u64(ch_ld(p+960+16*j),vld1q_u64(k->ph+30)); s=ch_xor3(s,ch_ll(a,b),ch_hh(a,b)); }
    vst1q_u64(&out[j].lo,s); }
}
CH_INLINE uint64_t ch_bulk_neon_run(const chainhash_key *k,const uint8_t *p,size_t regions,size_t len,ch_raw *st,const int hint,const unsigned step,size_t dist) {
    uint64x2_t s0=vdupq_n_u64(0),s1=s0,s2=s0,s3=vcombine_u64(vcreate_u64(len),vcreate_u64(0));
    const uint64x2_t y=vcombine_u64(vcreate_u64(k->yp[4]),vcreate_u64(k->yh[4]));
    const uint64x2_t a0=vld1q_u64(k->ph+0),b0=vld1q_u64(k->ph+2);
    const uint64x2_t a1=vld1q_u64(k->ph+4),b1=vld1q_u64(k->ph+6);
    const uint64x2_t a2=vld1q_u64(k->ph+8),b2=vld1q_u64(k->ph+10);
    const uint64x2_t a3=vld1q_u64(k->ph+12),b3=vld1q_u64(k->ph+14);
    const uint64x2_t a4=vld1q_u64(k->ph+16),b4=vld1q_u64(k->ph+18);
    const uint64x2_t a5=vld1q_u64(k->ph+20),b5=vld1q_u64(k->ph+22);
    const uint64x2_t a6=vld1q_u64(k->ph+24),b6=vld1q_u64(k->ph+26);
    const uint64x2_t a7=vld1q_u64(k->ph+28),b7=vld1q_u64(k->ph+30);
    if(st) { s0=vld1q_u64(&st[0].lo); s1=vld1q_u64(&st[1].lo); s2=vld1q_u64(&st[2].lo); s3=vld1q_u64(&st[3].lo); }
    do {
        CH_PREFETCH(p,regions,1024,hint,step,dist);
#if CHAINHASH_NEON_FUSE
        /* Lane j keeps two chains, lo and hi products, each seeded with its half of s_j*y^4. */
        uint64x2_t l0=ch_ll(s0,y),h0=ch_hh(s0,y),l1=ch_ll(s1,y),h1=ch_hh(s1,y),l2=ch_ll(s2,y),h2=ch_hh(s2,y),l3=ch_ll(s3,y),h3=ch_hh(s3,y);
        { uint64x2_t a=veorq_u64(ch_ld(p+0),a0), b=veorq_u64(ch_ld(p+64),b0); l0=ch_fll(l0,a,b); h0=ch_fhh(h0,a,b); }
        { uint64x2_t a=veorq_u64(ch_ld(p+16),a0), b=veorq_u64(ch_ld(p+80),b0); l1=ch_fll(l1,a,b); h1=ch_fhh(h1,a,b); }
        { uint64x2_t a=veorq_u64(ch_ld(p+32),a0), b=veorq_u64(ch_ld(p+96),b0); l2=ch_fll(l2,a,b); h2=ch_fhh(h2,a,b); }
        { uint64x2_t a=veorq_u64(ch_ld(p+48),a0), b=veorq_u64(ch_ld(p+112),b0); l3=ch_fll(l3,a,b); h3=ch_fhh(h3,a,b); }
        { uint64x2_t a=veorq_u64(ch_ld(p+128),a1), b=veorq_u64(ch_ld(p+192),b1); l0=ch_fll(l0,a,b); h0=ch_fhh(h0,a,b); }
        { uint64x2_t a=veorq_u64(ch_ld(p+144),a1), b=veorq_u64(ch_ld(p+208),b1); l1=ch_fll(l1,a,b); h1=ch_fhh(h1,a,b); }
        { uint64x2_t a=veorq_u64(ch_ld(p+160),a1), b=veorq_u64(ch_ld(p+224),b1); l2=ch_fll(l2,a,b); h2=ch_fhh(h2,a,b); }
        { uint64x2_t a=veorq_u64(ch_ld(p+176),a1), b=veorq_u64(ch_ld(p+240),b1); l3=ch_fll(l3,a,b); h3=ch_fhh(h3,a,b); }
        { uint64x2_t a=veorq_u64(ch_ld(p+256),a2), b=veorq_u64(ch_ld(p+320),b2); l0=ch_fll(l0,a,b); h0=ch_fhh(h0,a,b); }
        { uint64x2_t a=veorq_u64(ch_ld(p+272),a2), b=veorq_u64(ch_ld(p+336),b2); l1=ch_fll(l1,a,b); h1=ch_fhh(h1,a,b); }
        { uint64x2_t a=veorq_u64(ch_ld(p+288),a2), b=veorq_u64(ch_ld(p+352),b2); l2=ch_fll(l2,a,b); h2=ch_fhh(h2,a,b); }
        { uint64x2_t a=veorq_u64(ch_ld(p+304),a2), b=veorq_u64(ch_ld(p+368),b2); l3=ch_fll(l3,a,b); h3=ch_fhh(h3,a,b); }
        { uint64x2_t a=veorq_u64(ch_ld(p+384),a3), b=veorq_u64(ch_ld(p+448),b3); l0=ch_fll(l0,a,b); h0=ch_fhh(h0,a,b); }
        { uint64x2_t a=veorq_u64(ch_ld(p+400),a3), b=veorq_u64(ch_ld(p+464),b3); l1=ch_fll(l1,a,b); h1=ch_fhh(h1,a,b); }
        { uint64x2_t a=veorq_u64(ch_ld(p+416),a3), b=veorq_u64(ch_ld(p+480),b3); l2=ch_fll(l2,a,b); h2=ch_fhh(h2,a,b); }
        { uint64x2_t a=veorq_u64(ch_ld(p+432),a3), b=veorq_u64(ch_ld(p+496),b3); l3=ch_fll(l3,a,b); h3=ch_fhh(h3,a,b); }
        { uint64x2_t a=veorq_u64(ch_ld(p+512),a4), b=veorq_u64(ch_ld(p+576),b4); l0=ch_fll(l0,a,b); h0=ch_fhh(h0,a,b); }
        { uint64x2_t a=veorq_u64(ch_ld(p+528),a4), b=veorq_u64(ch_ld(p+592),b4); l1=ch_fll(l1,a,b); h1=ch_fhh(h1,a,b); }
        { uint64x2_t a=veorq_u64(ch_ld(p+544),a4), b=veorq_u64(ch_ld(p+608),b4); l2=ch_fll(l2,a,b); h2=ch_fhh(h2,a,b); }
        { uint64x2_t a=veorq_u64(ch_ld(p+560),a4), b=veorq_u64(ch_ld(p+624),b4); l3=ch_fll(l3,a,b); h3=ch_fhh(h3,a,b); }
        { uint64x2_t a=veorq_u64(ch_ld(p+640),a5), b=veorq_u64(ch_ld(p+704),b5); l0=ch_fll(l0,a,b); h0=ch_fhh(h0,a,b); }
        { uint64x2_t a=veorq_u64(ch_ld(p+656),a5), b=veorq_u64(ch_ld(p+720),b5); l1=ch_fll(l1,a,b); h1=ch_fhh(h1,a,b); }
        { uint64x2_t a=veorq_u64(ch_ld(p+672),a5), b=veorq_u64(ch_ld(p+736),b5); l2=ch_fll(l2,a,b); h2=ch_fhh(h2,a,b); }
        { uint64x2_t a=veorq_u64(ch_ld(p+688),a5), b=veorq_u64(ch_ld(p+752),b5); l3=ch_fll(l3,a,b); h3=ch_fhh(h3,a,b); }
        { uint64x2_t a=veorq_u64(ch_ld(p+768),a6), b=veorq_u64(ch_ld(p+832),b6); l0=ch_fll(l0,a,b); h0=ch_fhh(h0,a,b); }
        { uint64x2_t a=veorq_u64(ch_ld(p+784),a6), b=veorq_u64(ch_ld(p+848),b6); l1=ch_fll(l1,a,b); h1=ch_fhh(h1,a,b); }
        { uint64x2_t a=veorq_u64(ch_ld(p+800),a6), b=veorq_u64(ch_ld(p+864),b6); l2=ch_fll(l2,a,b); h2=ch_fhh(h2,a,b); }
        { uint64x2_t a=veorq_u64(ch_ld(p+816),a6), b=veorq_u64(ch_ld(p+880),b6); l3=ch_fll(l3,a,b); h3=ch_fhh(h3,a,b); }
        { uint64x2_t a=veorq_u64(ch_ld(p+896),a7), b=veorq_u64(ch_ld(p+960),b7); l0=ch_fll(l0,a,b); h0=ch_fhh(h0,a,b); }
        { uint64x2_t a=veorq_u64(ch_ld(p+912),a7), b=veorq_u64(ch_ld(p+976),b7); l1=ch_fll(l1,a,b); h1=ch_fhh(h1,a,b); }
        { uint64x2_t a=veorq_u64(ch_ld(p+928),a7), b=veorq_u64(ch_ld(p+992),b7); l2=ch_fll(l2,a,b); h2=ch_fhh(h2,a,b); }
        { uint64x2_t a=veorq_u64(ch_ld(p+944),a7), b=veorq_u64(ch_ld(p+1008),b7); l3=ch_fll(l3,a,b); h3=ch_fhh(h3,a,b); }
        s0=veorq_u64(l0,h0); s1=veorq_u64(l1,h1); s2=veorq_u64(l2,h2); s3=veorq_u64(l3,h3);
#else
        uint64x2_t u0=vdupq_n_u64(0),u1=u0,u2=u0,u3=u0;
        { uint64x2_t a=veorq_u64(ch_ld(p+0),a0), b=veorq_u64(ch_ld(p+64),b0); u0=ch_xor3(u0,ch_ll(a,b),ch_hh(a,b)); }
        { uint64x2_t a=veorq_u64(ch_ld(p+16),a0), b=veorq_u64(ch_ld(p+80),b0); u1=ch_xor3(u1,ch_ll(a,b),ch_hh(a,b)); }
        { uint64x2_t a=veorq_u64(ch_ld(p+32),a0), b=veorq_u64(ch_ld(p+96),b0); u2=ch_xor3(u2,ch_ll(a,b),ch_hh(a,b)); }
        { uint64x2_t a=veorq_u64(ch_ld(p+48),a0), b=veorq_u64(ch_ld(p+112),b0); u3=ch_xor3(u3,ch_ll(a,b),ch_hh(a,b)); }
        { uint64x2_t a=veorq_u64(ch_ld(p+128),a1), b=veorq_u64(ch_ld(p+192),b1); u0=ch_xor3(u0,ch_ll(a,b),ch_hh(a,b)); }
        { uint64x2_t a=veorq_u64(ch_ld(p+144),a1), b=veorq_u64(ch_ld(p+208),b1); u1=ch_xor3(u1,ch_ll(a,b),ch_hh(a,b)); }
        { uint64x2_t a=veorq_u64(ch_ld(p+160),a1), b=veorq_u64(ch_ld(p+224),b1); u2=ch_xor3(u2,ch_ll(a,b),ch_hh(a,b)); }
        { uint64x2_t a=veorq_u64(ch_ld(p+176),a1), b=veorq_u64(ch_ld(p+240),b1); u3=ch_xor3(u3,ch_ll(a,b),ch_hh(a,b)); }
        { uint64x2_t a=veorq_u64(ch_ld(p+256),a2), b=veorq_u64(ch_ld(p+320),b2); u0=ch_xor3(u0,ch_ll(a,b),ch_hh(a,b)); }
        { uint64x2_t a=veorq_u64(ch_ld(p+272),a2), b=veorq_u64(ch_ld(p+336),b2); u1=ch_xor3(u1,ch_ll(a,b),ch_hh(a,b)); }
        { uint64x2_t a=veorq_u64(ch_ld(p+288),a2), b=veorq_u64(ch_ld(p+352),b2); u2=ch_xor3(u2,ch_ll(a,b),ch_hh(a,b)); }
        { uint64x2_t a=veorq_u64(ch_ld(p+304),a2), b=veorq_u64(ch_ld(p+368),b2); u3=ch_xor3(u3,ch_ll(a,b),ch_hh(a,b)); }
        { uint64x2_t a=veorq_u64(ch_ld(p+384),a3), b=veorq_u64(ch_ld(p+448),b3); u0=ch_xor3(u0,ch_ll(a,b),ch_hh(a,b)); }
        { uint64x2_t a=veorq_u64(ch_ld(p+400),a3), b=veorq_u64(ch_ld(p+464),b3); u1=ch_xor3(u1,ch_ll(a,b),ch_hh(a,b)); }
        { uint64x2_t a=veorq_u64(ch_ld(p+416),a3), b=veorq_u64(ch_ld(p+480),b3); u2=ch_xor3(u2,ch_ll(a,b),ch_hh(a,b)); }
        { uint64x2_t a=veorq_u64(ch_ld(p+432),a3), b=veorq_u64(ch_ld(p+496),b3); u3=ch_xor3(u3,ch_ll(a,b),ch_hh(a,b)); }
        { uint64x2_t a=veorq_u64(ch_ld(p+512),a4), b=veorq_u64(ch_ld(p+576),b4); u0=ch_xor3(u0,ch_ll(a,b),ch_hh(a,b)); }
        { uint64x2_t a=veorq_u64(ch_ld(p+528),a4), b=veorq_u64(ch_ld(p+592),b4); u1=ch_xor3(u1,ch_ll(a,b),ch_hh(a,b)); }
        { uint64x2_t a=veorq_u64(ch_ld(p+544),a4), b=veorq_u64(ch_ld(p+608),b4); u2=ch_xor3(u2,ch_ll(a,b),ch_hh(a,b)); }
        { uint64x2_t a=veorq_u64(ch_ld(p+560),a4), b=veorq_u64(ch_ld(p+624),b4); u3=ch_xor3(u3,ch_ll(a,b),ch_hh(a,b)); }
        { uint64x2_t a=veorq_u64(ch_ld(p+640),a5), b=veorq_u64(ch_ld(p+704),b5); u0=ch_xor3(u0,ch_ll(a,b),ch_hh(a,b)); }
        { uint64x2_t a=veorq_u64(ch_ld(p+656),a5), b=veorq_u64(ch_ld(p+720),b5); u1=ch_xor3(u1,ch_ll(a,b),ch_hh(a,b)); }
        { uint64x2_t a=veorq_u64(ch_ld(p+672),a5), b=veorq_u64(ch_ld(p+736),b5); u2=ch_xor3(u2,ch_ll(a,b),ch_hh(a,b)); }
        { uint64x2_t a=veorq_u64(ch_ld(p+688),a5), b=veorq_u64(ch_ld(p+752),b5); u3=ch_xor3(u3,ch_ll(a,b),ch_hh(a,b)); }
        { uint64x2_t a=veorq_u64(ch_ld(p+768),a6), b=veorq_u64(ch_ld(p+832),b6); u0=ch_xor3(u0,ch_ll(a,b),ch_hh(a,b)); }
        { uint64x2_t a=veorq_u64(ch_ld(p+784),a6), b=veorq_u64(ch_ld(p+848),b6); u1=ch_xor3(u1,ch_ll(a,b),ch_hh(a,b)); }
        { uint64x2_t a=veorq_u64(ch_ld(p+800),a6), b=veorq_u64(ch_ld(p+864),b6); u2=ch_xor3(u2,ch_ll(a,b),ch_hh(a,b)); }
        { uint64x2_t a=veorq_u64(ch_ld(p+816),a6), b=veorq_u64(ch_ld(p+880),b6); u3=ch_xor3(u3,ch_ll(a,b),ch_hh(a,b)); }
        { uint64x2_t a=veorq_u64(ch_ld(p+896),a7), b=veorq_u64(ch_ld(p+960),b7); u0=ch_xor3(u0,ch_ll(a,b),ch_hh(a,b)); }
        { uint64x2_t a=veorq_u64(ch_ld(p+912),a7), b=veorq_u64(ch_ld(p+976),b7); u1=ch_xor3(u1,ch_ll(a,b),ch_hh(a,b)); }
        { uint64x2_t a=veorq_u64(ch_ld(p+928),a7), b=veorq_u64(ch_ld(p+992),b7); u2=ch_xor3(u2,ch_ll(a,b),ch_hh(a,b)); }
        { uint64x2_t a=veorq_u64(ch_ld(p+944),a7), b=veorq_u64(ch_ld(p+1008),b7); u3=ch_xor3(u3,ch_ll(a,b),ch_hh(a,b)); }
        s0=ch_xor3(u0,ch_ll(s0,y),ch_hh(s0,y));
        s1=ch_xor3(u1,ch_ll(s1,y),ch_hh(s1,y));
        s2=ch_xor3(u2,ch_ll(s2,y),ch_hh(s2,y));
        s3=ch_xor3(u3,ch_ll(s3,y),ch_hh(s3,y));
#endif
        p+=1024;
    } while(--regions);
    if(st) { vst1q_u64(&st[0].lo,s0); vst1q_u64(&st[1].lo,s1); vst1q_u64(&st[2].lo,s2); vst1q_u64(&st[3].lo,s3); return 0; }
    uint64x2_t acc=vdupq_n_u64(0),pw;
    pw=vcombine_u64(vcreate_u64(k->yp[3]),vcreate_u64(k->yh[3])); acc=ch_xor3(acc,ch_ll(s0,pw),ch_hh(s0,pw));
    pw=vcombine_u64(vcreate_u64(k->yp[2]),vcreate_u64(k->yh[2])); acc=ch_xor3(acc,ch_ll(s1,pw),ch_hh(s1,pw));
    pw=vcombine_u64(vcreate_u64(k->yp[1]),vcreate_u64(k->yh[1])); acc=ch_xor3(acc,ch_ll(s2,pw),ch_hh(s2,pw));
    pw=vcombine_u64(vcreate_u64(k->yp[0]),vcreate_u64(k->yh[0])); acc=ch_xor3(acc,ch_ll(s3,pw),ch_hh(s3,pw));
    ch_raw r; vst1q_u64(&r.lo,acc); return ch_reduce(r);
}
static uint64_t ch_bulk_neon(const chainhash_key *k,const uint8_t *p,size_t regions,size_t len,ch_raw *st) { return ch_bulk_neon_run(k,p,regions,len,st,CH_PF_OFF,64,0); }
static uint64_t ch_bulk_neon_pf(const chainhash_key *k,const uint8_t *p,size_t regions,size_t len,ch_raw *st,int hint,unsigned step,size_t dist) { CH_PF_SWITCH(ch_bulk_neon,k,p,regions,len,st) }
CH_NEND
#endif
static inline void ch_region(const chainhash_key *k,const uint8_t *p,size_t n,ch_raw out[4],int b) {
    if(n==1024) {
#ifdef CH_X86
        if(b==3) { ch_region512(k,p,out); return; }
        if(b==2) { ch_region256(k,p,out); return; }
        if(b==1) { ch_region128(k,p,out); return; }
#elif defined(CH_ARM)
        if(b==4) { ch_region_neon(k,p,out); return; }
#endif
    }
    ch_region_scalar(k,p,n,out,b);
}
/* Streaming keeps at most one incomplete region. len is supplied at final,
 * so any chunking (including empty updates) has identical semantics.
 * Backend is explicit for testing; use chainhash_backend() in applications.
 * stride must be 1..8; lazy must be 0 or 1. key must outlive the stream.
 */
typedef struct {
    const chainhash_key *key; ch_raw state[8];
    uint64_t len, blocks; unsigned stride; int lazy, backend;
    size_t used; uint8_t buffer[1024];
} chainhash_stream;
static inline void chainhash_init(chainhash_stream *s,const chainhash_key *k,unsigned stride,int lazy,int backend) {
    assert(stride>=1 && stride<=8 && chainhash_has_backend(backend));
    memset(s,0,sizeof(*s)); s->key=k; s->stride=stride; s->lazy=lazy; s->backend=backend;
}
static inline void ch_absorb(chainhash_stream *s,const uint8_t *p,size_t n) {
    ch_raw c[4]; unsigned j,count=ch_lanes(n); const chainhash_key *k=s->key;
    ch_region(k,p,n,c,s->backend);
    for(j=0;j<count;j++) {
        ch_raw *r=&s->state[s->blocks%s->stride];
        if(s->lazy) { ch_raw a=ch_prod(r->lo,k->yp[s->stride],s->backend), b=ch_prod(r->hi,k->yh[s->stride],s->backend); r->lo=a.lo^b.lo^c[j].lo; r->hi=a.hi^b.hi^c[j].hi; }
        else { r->lo=ch_fmul(r->lo,k->yp[s->stride],s->backend)^ch_reduce(c[j]); r->hi=0; }
        ++s->blocks;
    }
}
/* Whole regions of a stride-4 stream run through the bulk kernel, whose lane j
 * holds the blocks congruent to j mod 4 like state j: the kernel starts from the
 * stream's raw states and writes them back (eager states are reduced again). */
static inline int ch_stream_bulk(chainhash_stream *s,const uint8_t *p,size_t regions) {
#if defined(CH_X86) || defined(CH_ARM)
    const chainhash_key *k=s->key; int b=s->backend; unsigned j;
    if(s->stride!=4 || s->blocks%4) return 0;
#ifdef CH_X86
    if(b==3) ch_bulk512(k,p,regions,0,s->state); else if(b==2) ch_bulk256(k,p,regions,0,s->state); else if(b==1) ch_bulk128(k,p,regions,0,s->state); else return 0;
#else
    if(b==4) ch_bulk_neon(k,p,regions,0,s->state); else return 0;
#endif
    if(!s->lazy) for(j=0;j<4;j++) { s->state[j].lo=ch_reduce(s->state[j]); s->state[j].hi=0; }
    s->blocks+=4*(uint64_t)regions; return 1;
#else
    (void)s; (void)p; (void)regions; return 0;
#endif
}
static inline void chainhash_update(chainhash_stream *s,const void *data,size_t n) {
    const uint8_t *p=(const uint8_t *)data; assert(n<=UINT64_MAX-s->len); s->len+=n;
    if(s->used) { size_t take=1024-s->used; if(take>n) take=n; if(take) { memcpy(s->buffer+s->used,p,take); p+=take; } s->used+=take; n-=take; if(s->used==1024) { ch_absorb(s,s->buffer,1024); s->used=0; } }
    if(n>=1024 && ch_stream_bulk(s,p,n/1024)) { p+=n/1024*1024; n%=1024; }
    while(n>=1024) { ch_absorb(s,p,1024); p+=1024; n-=1024; }
    if(n) { memcpy(s->buffer,p,n); s->used=n; }
}
/* Return the message polynomial without its leading length coefficient.
 * This is useful for region-aligned independent thread partitions. */
static inline uint64_t chainhash_partial(chainhash_stream *s) {
    uint64_t v=0; unsigned j;
    if(s->used || !s->blocks) { ch_absorb(s,s->buffer,s->used); s->used=0; }
    for(j=0;j<s->stride && j<s->blocks;j++) { unsigned e=(unsigned)((s->blocks-1-j)%s->stride); v^=ch_fmul(ch_reduce(s->state[j]),s->key->yp[e],s->backend); }
    return v;
}
static inline uint64_t chainhash_final(chainhash_stream *s) {
    uint64_t v=chainhash_partial(s);
    v^=ch_fmul(s->len,ch_pow(s->key->yp[1],s->blocks,s->backend),s->backend);
    return ch_finish(s->key,v,s->backend);
}
/* Concatenate region-aligned chunks. right_blocks counts only actual right
 * blocks; an empty right chunk has zero blocks (not the empty-hash sentinel).
 * Inputs exclude length and finalizer. No inversion: y=0 works as well. */
static inline uint64_t chainhash_join(const chainhash_key *k,uint64_t left,uint64_t right,uint64_t right_blocks,int backend) {
    return ch_fmul(left,ch_pow(k->yp[1],right_blocks,backend),backend)^right;
}
static inline uint64_t chainhash_evaluate(const chainhash_key *k,const void *data,size_t len,unsigned stride,int lazy,int backend) {
    chainhash_stream s; chainhash_init(&s,k,stride,lazy,backend); chainhash_update(&s,data,len); return chainhash_final(&s);
}
/* Portable reference: serial eager Horner, length leading, no round robin. */
static inline uint64_t chainhash_portable(const chainhash_key *k,const void *data,size_t len) {
    const uint8_t *p=(const uint8_t *)data; size_t n=len; uint64_t v=len;
    do { ch_raw c[4]; size_t take=n<1024?n:1024; unsigned j;
        ch_region_scalar(k,p,take,c,0);
        for(j=0;j<ch_lanes(take);j++) v=ch_mul(v,k->yp[1])^ch_reduce(c[j]);
        n-=take; if(!n) break; p+=take;
    } while(1);
    return ch_finish(k,v,0);
}
#if defined(__GNUC__) || defined(__clang__)
#define CH_NOINLINE __attribute__((noinline))
#else
#define CH_NOINLINE
#endif
static CH_NOINLINE uint64_t ch_hash_body(const chainhash_key *k,const void *data,size_t len,int backend,int hint,unsigned step,size_t dist) {
    const uint8_t *p=(const uint8_t *)data; size_t full=len/1024,n=len%1024; uint64_t v=len;
    assert(chainhash_has_backend(backend)); (void)hint; (void)step; (void)dist;
    if(full) {
#ifdef CH_X86
        if(backend==3) v=hint?ch_bulk512_pf(k,p,full,len,0,hint,step,dist):ch_bulk512(k,p,full,len,0);
        else if(backend==2) v=hint?ch_bulk256_pf(k,p,full,len,0,hint,step,dist):ch_bulk256(k,p,full,len,0);
        else if(backend==1) v=hint?ch_bulk128_pf(k,p,full,len,0,hint,step,dist):ch_bulk128(k,p,full,len,0);
        else return chainhash_evaluate(k,data,len,4,1,backend);
#elif defined(CH_ARM)
        if(backend==4) v=hint?ch_bulk_neon_pf(k,p,full,len,0,hint,step,dist):ch_bulk_neon(k,p,full,len,0);
        else return chainhash_evaluate(k,data,len,4,1,backend);
#else
        return chainhash_evaluate(k,data,len,4,1,backend);
#endif
        p+=full*1024;
    }
    #ifdef CH_X86
    /* The latency-bound XMM tail also serves ZMM for short tails: up to 64 bytes where
     * PCLMULQDQ issues no faster than VPCLMULQDQ on ZMM (AMD, where one 512-bit product
     * does the work of four 128-bit ones at the same rate), up to 768 where it issues
     * every cycle (Intel). */
    if(backend==3 && n>(ch_pclmul_fast() ? 768u : 64u)) return ch_finish(k,ch_tail512(k,p,n,v),backend);
    if(backend && (n || !full)) return ch_tail128(k,p,n,v);
#elif defined(CH_ARM)
    if(backend==4 && (n || !full)) return n<=64 ? ch_short_neon(k,p,n,v) : ch_tail_neon(k,p,n,v);
#endif
    if(n || !full) { ch_raw c[4]; unsigned j; ch_region_scalar(k,p,n,c,backend); for(j=0;j<ch_lanes(n);j++) v=ch_fmul(v,k->yp[1],backend)^ch_reduce(c[j]); }
    return ch_finish(k,v,backend);
}
/* Inputs up to 64 bytes take the register-only path without the general body's frame. */
static inline uint64_t ch_hash(const chainhash_key *k,const void *data,size_t len,int backend,int hint,unsigned step,size_t dist) {
#ifdef CH_X86
    if(len<=64 && backend) return ch_short128(k,(const uint8_t *)data,len,len);
#elif defined(CH_ARM)
    if(len<=64 && backend) return ch_short_neon(k,(const uint8_t *)data,len,len);
#endif
    return ch_hash_body(k,data,len,backend,hint,step,dist);
}
static inline uint64_t chainhash_with_backend(const chainhash_key *k,const void *data,size_t len,int backend) { return ch_hash(k,data,len,backend,CH_PF_OFF,64,0); }
static inline uint64_t chainhash(const chainhash_key *k,const void *data,size_t len) { return chainhash_with_backend(k,data,len,chainhash_backend()); }
/* chainhash_with_backend with software prefetch (CH_PF_*, every step = 64 or 128
 * bytes, dist bytes ahead); an unavailable backend falls back to the detected one.
 * The digest is the same for every argument; chainhash_calibrate.h chooses them. */
static inline uint64_t chainhash_with_prefetch(const chainhash_key *k,const void *data,size_t len,int backend,int hint,unsigned step,size_t dist) {
    if(!chainhash_has_backend(backend)) backend=chainhash_backend();
    return ch_hash(k,data,len,backend,hint>=CH_PF_T0 && hint<=CH_PF_NTA ? hint : CH_PF_OFF,step,dist);
}
#ifdef CH_X86
static inline uint64_t chainhash_xmm(const chainhash_key *k,const void *p,size_t n) { return chainhash_with_backend(k,p,n,CH_XMM); }
static inline uint64_t chainhash_ymm(const chainhash_key *k,const void *p,size_t n) { return chainhash_with_backend(k,p,n,CH_YMM); }
static inline uint64_t chainhash_zmm(const chainhash_key *k,const void *p,size_t n) { return chainhash_with_backend(k,p,n,CH_ZMM); }
#endif
#ifdef CH_ARM
static inline uint64_t chainhash_neon(const chainhash_key *k,const void *p,size_t n) { return chainhash_with_backend(k,p,n,CH_NEON); }
#endif
static inline int chainhash_selftest(void) {
    uint8_t m[2049]; chainhash_key k=chainhash_key_from_seed(123); size_t n; unsigned j;
    for(j=0;j<sizeof(m);j++) m[j]=(uint8_t)j;
    /* Constants generated by test/vectors.c's independent memo evaluator. */
    if(chainhash(&k,m,0)!=UINT64_C(0xede120e3ad6ec193) ||
       chainhash(&k,m,17)!=UINT64_C(0x97c346f5999acee9) ||
       chainhash(&k,m,1024)!=UINT64_C(0xf3897c02083c9f82)) return 0;
    for(n=0;n<sizeof(m);n=n<65?n+1:n+127) if(chainhash(&k,m,n)!=chainhash_portable(&k,m,n)) return 0;
    return 1;
}
#endif
