/* ChainHash-128: a keyed 128-bit hash for long inputs with a proven collision bound.
 * Copyright 2026 Thomas Dybdahl Ahle. MIT license. C99/C++11, one header, no allocation.
 *
 * Key: 128 uniformly random bytes (CHAINHASH128_KEY_BYTES), chainhash128_key_from_bytes.
 * chainhash128_key_from_seed expands a 64-bit seed; it is for benchmarks and tests.
 * Hash: chainhash128(&key, data, len) returns a ch128_word (two uint64_t limbs);
 * chainhash128_store writes its 16 canonical little-endian bytes. Streaming:
 * chainhash128_init, chainhash128_update, chainhash128_final. Region-aligned parallel
 * evaluation: chainhash128_partial and chainhash128_join. Portable C, x86 XMM/YMM/ZMM
 * and ARM NEON backends, every stride, reduction schedule, product method and
 * chunking compute the same digest.
 * Bound: for two distinct messages fixed independently of the key, each of at most
 * 8L bytes, Pr[collision] <= (p(L)+d(L))/2^128, where p is the block count and
 * d <= 32; see docs/THEOREM-128.md. The definition and index map: docs/SPEC-128.md.
 * Little-endian bytes; no alignment or padding required. Explicit backend calls
 * require chainhash128_has_backend(). AArch64: -march=armv8-a+crypto
 * (Apple: -march=native+crypto). Define CHAINHASH128_PORTABLE to omit all hardware code.
 */
#ifndef CHAINHASH128_H
#define CHAINHASH128_H
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <assert.h>
#define CHAINHASH128_BLOCK_BYTES 512
#define CH128_W (CHAINHASH128_BLOCK_BYTES/16)
#define CH128_CHUNKS (CH128_W/2)
#define CH128_REGION (8*CHAINHASH128_BLOCK_BYTES)
#define CHAINHASH128_KEY_BYTES 128
typedef struct { uint64_t lo,hi; } ch128_word;
typedef struct { ch128_word lo,hi; } ch128_raw;
/* sk[e] = kappa_1*y^e (e = 0..7): the short path's word weights. */
typedef struct { ch128_word ph[CH128_W],yp[9],yh[9],c[5],tau,sk[8]; } chainhash128_key;
enum { CH128_PORTABLE=0,CH128_XMM=1,CH128_YMM=2,CH128_ZMM=3,CH128_NEON=4 };
static inline ch128_word ch128_make(uint64_t lo, uint64_t hi) { ch128_word r; r.lo=lo; r.hi=hi; return r; }
static inline ch128_word ch128_xor(ch128_word a, ch128_word b) { return ch128_make(a.lo^b.lo,a.hi^b.hi); }
static inline int ch128_equal(ch128_word a, ch128_word b) { return a.lo==b.lo && a.hi==b.hi; }
static inline ch128_word ch128_addint(ch128_word a, ch128_word b) {
    uint64_t lo=a.lo+b.lo; return ch128_make(lo,a.hi+b.hi+(lo<a.lo));
}
static inline uint64_t ch128_load64(const uint8_t *p) {
    uint64_t r=0; unsigned i; for(i=0;i<8;i++) r|=(uint64_t)p[i]<<(8*i); return r;
}
static inline ch128_word ch128_load(const uint8_t *p) { return ch128_make(ch128_load64(p),ch128_load64(p+8)); }
static inline void chainhash128_store(void *out, ch128_word v) {
    uint8_t *p=(uint8_t *)out; unsigned i;
#if defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    /* Two word stores from registers: a later narrower load of the digest forwards from
     * them. (Copying the returned struct lets GCC spill it and move it with one 16-byte
     * load, which the two register stores of the return value cannot forward to.) */
    uint64_t lo=v.lo,hi=v.hi; (void)i;
#if defined(__GNUC__) || defined(__clang__)
    __asm__("" : "+r"(lo), "+r"(hi));
#endif
    memcpy(p,&lo,8); memcpy(p+8,&hi,8);
#else
    for(i=0;i<8;i++) { p[i]=(uint8_t)(v.lo>>(8*i)); p[i+8]=(uint8_t)(v.hi>>(8*i)); }
#endif
}
/* Independent bit-serial oracle, including the UNREDUCED 256-bit product. */
static inline ch128_raw ch128_clmul_ref(ch128_word a, ch128_word b) {
    uint64_t x0=a.lo,x1=a.hi,x2=0,x3=0,r0=0,r1=0,r2=0,r3=0; unsigned i;
    ch128_raw r;
    for(i=0;i<128;i++) {
        uint64_t mask=UINT64_C(0)-(b.lo&1);
        r0^=x0&mask; r1^=x1&mask; r2^=x2&mask; r3^=x3&mask;
        x3=(x3<<1)|(x2>>63); x2=(x2<<1)|(x1>>63);
        x1=(x1<<1)|(x0>>63); x0<<=1;
        b.lo=(b.lo>>1)|(b.hi<<63); b.hi>>=1;
    }
    r.lo=ch128_make(r0,r1); r.hi=ch128_make(r2,r3); return r;
}
/* Bit-serial field multiplication, X^128 = 0x87. */
static inline ch128_word ch128_mul_ref(ch128_word a, ch128_word b) {
    ch128_word r=ch128_make(0,0); unsigned i;
    for(i=0;i<128;i++) {
        uint64_t mask=UINT64_C(0)-(b.lo&1), top=a.hi>>63;
        r.lo^=a.lo&mask; r.hi^=a.hi&mask;
        a.hi=(a.hi<<1)|(a.lo>>63); a.lo=(a.lo<<1)^(UINT64_C(0x87)&(UINT64_C(0)-top));
        b.lo=(b.lo>>1)|(b.hi<<63); b.hi>>=1;
    }
    return r;
}
static inline ch128_word ch128_partial(const uint8_t *p, size_t n, size_t off) {
    uint8_t buf[16]={0};
    if(off<n && n-off>=16) return ch128_load(p+off);
    if(off<n) { size_t count=n-off; if(count>16) count=16; memcpy(buf,p+off,count); }
    return ch128_load(buf);
}

static inline ch128_raw ch128_rxor(ch128_raw a,ch128_raw b) { a.lo=ch128_xor(a.lo,b.lo); a.hi=ch128_xor(a.hi,b.hi); return a; }
/* Fold X^128=0x87 twice, including all 256-bit representatives. */
static inline ch128_word ch128_reduce(ch128_raw a) {
    uint64_t l=a.hi.lo,h=a.hi.hi,t=(h>>63)^(h>>62)^(h>>57);
    return ch128_make(a.lo.lo^l^(l<<1)^(l<<2)^(l<<7)^t^(t<<1)^(t<<2)^(t<<7),
                       a.lo.hi^h^(h<<1)^(h<<2)^(h<<7)^(l>>63)^(l>>62)^(l>>57));
}
#if !defined(CHAINHASH128_PORTABLE) && (defined(__x86_64__) || defined(__i386__)) && (defined(__GNUC__) || defined(__clang__))
#define CH128_X86 1
#include <immintrin.h>
#define CH128_T128 __attribute__((target("avx,pclmul")))
#define CH128_T256 __attribute__((target("avx2,pclmul,vpclmulqdq")))
#define CH128_T512 __attribute__((target("avx2,pclmul,avx512f,vpclmulqdq")))
/* CPUID by inline assembly: GCC 9's <cpuid.h> has no include guard, so a
 * translation unit that included it twice (both ChainHash headers) failed. */
static inline void ch128_cpuid(unsigned leaf,unsigned sub,unsigned r[4]) {
    __asm__ __volatile__("cpuid":"=a"(r[0]),"=b"(r[1]),"=c"(r[2]),"=d"(r[3]):"a"(leaf),"c"(sub));
}
static inline int ch128_detect(void) {
    unsigned r[4],m,l,h;
    ch128_cpuid(0,0,r); m=r[0]; if(m<1) return 0;
    ch128_cpuid(1,0,r); if((r[2]&((1u<<1)|(1u<<27)|(1u<<28)))!=((1u<<1)|(1u<<27)|(1u<<28))) return 0;
    __asm__ volatile("xgetbv":"=a"(l),"=d"(h):"c"(0)); if((l&6)!=6) return 0;
    if(m<7) return 1;
    ch128_cpuid(7,0,r); if(!(r[1]&(1u<<5)) || !(r[2]&(1u<<10))) return 1;
    return (l&0xe6)==0xe6 && (r[1]&(1u<<16)) ? 3:2;
}
#elif !defined(CHAINHASH128_PORTABLE) && defined(__aarch64__) && !defined(__AARCH64EB__) && (defined(__GNUC__) || defined(__clang__))
#define CH128_ARM 1
#include <arm_neon.h>
/* PMULL (FEAT_PMULL) and EOR3 (FEAT_SHA3, optional from ARMv8.2) are optional.
 * Each is used unconditionally when the target guarantees it (CH128_PMULL,
 * CH128_SHA3 1) and otherwise only when the CPU reports it at run time (2).
 * Without compile-time PMULL the NEON code carries a target attribute
 * (CH128_NBEGIN/CH128_NEND) and the backend is NEON only on a CPU with PMULL.
 * Define CHAINHASH128_NO_SHA3 to force the EOR-only kernels. Same digest always. */
#if defined(__ARM_FEATURE_AES) || defined(__ARM_FEATURE_CRYPTO)
#define CH128_PMULL 1
#define CH128_NBEGIN
#define CH128_NEND
#else
#define CH128_PMULL 2
#ifdef __clang__
#define CH128_NBEGIN _Pragma("clang attribute push(__attribute__((target(\"aes\"))),apply_to=function)")
#define CH128_NEND _Pragma("clang attribute pop")
#else
#define CH128_NBEGIN _Pragma("GCC push_options") _Pragma("GCC target(\"+crypto\")")
#define CH128_NEND _Pragma("GCC pop_options")
#endif
#endif
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
#if defined(CHAINHASH128_NO_SHA3)
#define CH128_SHA3 0
#elif defined(__ARM_FEATURE_SHA3)
#define CH128_SHA3 1
#else
#define CH128_SHA3 2
#endif
#if CH128_PMULL == 2 || CH128_SHA3 == 2
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
static inline int ch128_arm_has(unsigned long hwcap,const char *feat,const char *old,int dflt) {
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
static inline int ch128_has_sha3(void) {
#if CH128_SHA3 == 2
    static int cache=-1; int v=__atomic_load_n(&cache,__ATOMIC_RELAXED); if(v<0) { v=ch128_arm_has(1ul<<17,"hw.optional.arm.FEAT_SHA3","hw.optional.armv8_2_sha3",0); __atomic_store_n(&cache,v,__ATOMIC_RELAXED); } return v;
#else
    return CH128_SHA3;
#endif
}
#endif
static inline int chainhash128_backend(void) {
#ifdef CH128_X86
    static int cache=-1; int v=__atomic_load_n(&cache,__ATOMIC_RELAXED); if(v<0) { v=ch128_detect(); __atomic_store_n(&cache,v,__ATOMIC_RELAXED); } return v;
#elif defined(CH128_ARM)
#if CH128_PMULL == 2
    static int cache=-1; int v=__atomic_load_n(&cache,__ATOMIC_RELAXED); if(v<0) { v=ch128_arm_has(1ul<<4,"hw.optional.arm.FEAT_PMULL",NULL,1)?CH128_NEON:0; __atomic_store_n(&cache,v,__ATOMIC_RELAXED); } return v;
#else
    return CH128_NEON;
#endif
#else
    return 0;
#endif
}
static inline int chainhash128_has_backend(int b) { int h=chainhash128_backend(); return b==0 || (h==4 ? b==4 : b>0 && b<=h); }
/* Default product method per backend (both give the same digest). Schoolbook where the
 * PCLMUL port also executes the Karatsuba shuffles and issues a product every cycle:
 * Intel Broadwell and later (detected by ADX) on XMM, ZMM, and NEON. Karatsuba on YMM and
 * on other XMM cores, where PCLMULQDQ issues once per two (AMD, Haswell) to eight (Sandy
 * and Ivy Bridge) cycles while shuffles and XORs use other pipes (uops.info). */
static inline int ch128_pclmul_fast(void) {
#ifdef CH128_X86
    static int cache=-1; int v=__atomic_load_n(&cache,__ATOMIC_RELAXED);
    if(v<0) { unsigned r[4],m; ch128_cpuid(0,0,r); m=r[0];
        v=r[1]==0x756e6547u && r[3]==0x49656e69u && r[2]==0x6c65746eu;
        if(v) { if(m>=7) { ch128_cpuid(7,0,r); v=(r[1]>>19)&1; } else v=0; }
        __atomic_store_n(&cache,v,__ATOMIC_RELAXED); }
    return v;
#else
    return 1;
#endif
}
static inline int ch128_school(int b) { return b==CH128_XMM ? ch128_pclmul_fast() : b==CH128_ZMM || b==CH128_NEON; }
/* Software prefetch in the x86 bulk kernels, as in chainhash.h: hint CH_PF_T0/T1/NTA
 * every step bytes of the region dist bytes ahead, formed only inside the input; no
 * setting changes a digest. The pinned NEON kernels take no hints (compute-bound). */
#ifndef CHAINHASH_PF_HINTS
#define CHAINHASH_PF_HINTS
enum { CH_PF_OFF=0, CH_PF_T0=1, CH_PF_T1=2, CH_PF_NTA=3 };
#endif
#ifdef CH128_X86
#define CH128_INLINE static inline __attribute__((always_inline))
#define CH128_PREFETCH(p,regions,bytes,hint,step,dist) do { if((hint) && (dist)<=((regions)-1)*(size_t)(bytes)) { unsigned o_; \
    for(o_=0;o_<(bytes);o_+=(step)) { const char *q_=(const char *)(p)+(dist)+o_; \
        if((hint)==CH_PF_T0) __builtin_prefetch(q_,0,3); else if((hint)==CH_PF_T1) __builtin_prefetch(q_,0,2); else __builtin_prefetch(q_,0,0); } } } while(0)
#define CH128_PF_SWITCH(NAME,...) switch(hint*2+(step==128)) { \
    case 2: return NAME##_run(__VA_ARGS__,CH_PF_T0,64,dist);  case 3: return NAME##_run(__VA_ARGS__,CH_PF_T0,128,dist); \
    case 4: return NAME##_run(__VA_ARGS__,CH_PF_T1,64,dist);  case 5: return NAME##_run(__VA_ARGS__,CH_PF_T1,128,dist); \
    case 6: return NAME##_run(__VA_ARGS__,CH_PF_NTA,64,dist); case 7: return NAME##_run(__VA_ARGS__,CH_PF_NTA,128,dist); \
    default: return NAME##_run(__VA_ARGS__,CH_PF_OFF,64,0); }
#endif

#ifdef CH128_X86
CH128_T128 static inline __m128i ch128_128_ll(__m128i a,__m128i b) { return _mm_clmulepi64_si128(a,b,0); }
CH128_T128 static inline __m128i ch128_128_hh(__m128i a,__m128i b) { return _mm_clmulepi64_si128(a,b,0x11); }
CH128_T128 static inline __m128i ch128_128_lh(__m128i a,__m128i b) { return _mm_clmulepi64_si128(a,b,0x10); }
CH128_T128 static inline __m128i ch128_128_hl(__m128i a,__m128i b) { return _mm_clmulepi64_si128(a,b,0x01); }
CH128_T128 static inline __m128i ch128_128_zero(void) { return _mm_setzero_si128(); }
CH128_T128 static inline __m128i ch128_128_load(const void *p) { return _mm_loadu_si128((const __m128i_u *)p); }
CH128_T128 static inline __m128i ch128_128_xor(__m128i a,__m128i b) { return _mm_xor_si128(a,b); }
CH128_T128 static inline __m128i ch128_128_swap(__m128i a) { return _mm_shuffle_epi32(a,0x4e); }
CH128_T128 static inline __m128i ch128_128_left(__m128i a) { return _mm_slli_si128(a,8); }
CH128_T128 static inline __m128i ch128_128_right(__m128i a) { return _mm_srli_si128(a,8); }
CH128_T128 static inline void ch128_128_store(void *p,__m128i a) { _mm_storeu_si128((__m128i_u *)p,a); }
CH128_T128 static inline __m128i ch128_128_bc(const void *p) { return _mm_loadu_si128((const __m128i_u *)p); }
typedef struct { __m128i l,h,m; } ch128_128_acc;
typedef struct { __m128i lo,hi; } ch128_128_raw;
CH128_T128 static inline ch128_128_acc ch128_128_azero(void) { ch128_128_acc r; r.l=r.h=r.m=ch128_128_zero(); return r; }
CH128_T128 static inline ch128_128_acc ch128_128_accum(ch128_128_acc r,__m128i a,__m128i b,int school) {
    __m128i l=ch128_128_ll(a,b),h=ch128_128_hh(a,b),m;
    if(school) m=ch128_128_xor(ch128_128_lh(a,b),ch128_128_hl(a,b));
    else m=ch128_128_ll(ch128_128_xor(a,ch128_128_swap(a)),ch128_128_xor(b,ch128_128_swap(b)));
    r.l=ch128_128_xor(r.l,l); r.h=ch128_128_xor(r.h,h); r.m=ch128_128_xor(r.m,m); __asm__("" : "+x"(r.l), "+x"(r.h), "+x"(r.m)); return r;
}
CH128_T128 static inline ch128_128_raw ch128_128_pack(ch128_128_acc a,int school) {
    ch128_128_raw r; if(!school) a.m=ch128_128_xor(a.m,ch128_128_xor(a.l,a.h));
    r.lo=ch128_128_xor(a.l,ch128_128_left(a.m)); r.hi=ch128_128_xor(a.h,ch128_128_right(a.m)); return r;
}
CH128_T128 static inline ch128_128_raw ch128_128_prod(__m128i a,__m128i b,int school) { return ch128_128_pack(ch128_128_accum(ch128_128_azero(),a,b,school),school); }
CH128_T128 static inline ch128_raw ch128_128_wordprod(ch128_word a,ch128_word b,int school) {
    ch128_raw r; ch128_128_raw v=ch128_128_prod(ch128_128_load(&a),ch128_128_load(&b),school); ch128_128_store(&r.lo,v.lo); ch128_128_store(&r.hi,v.hi); return r;
}
#endif

#ifdef CH128_X86
CH128_T256 static inline __m256i ch128_256_ll(__m256i a,__m256i b) { return _mm256_clmulepi64_epi128(a,b,0); }
CH128_T256 static inline __m256i ch128_256_hh(__m256i a,__m256i b) { return _mm256_clmulepi64_epi128(a,b,0x11); }
CH128_T256 static inline __m256i ch128_256_lh(__m256i a,__m256i b) { return _mm256_clmulepi64_epi128(a,b,0x10); }
CH128_T256 static inline __m256i ch128_256_hl(__m256i a,__m256i b) { return _mm256_clmulepi64_epi128(a,b,0x01); }
CH128_T256 static inline __m256i ch128_256_zero(void) { return _mm256_setzero_si256(); }
CH128_T256 static inline __m256i ch128_256_load(const void *p) { return _mm256_loadu_si256((const __m256i_u *)p); }
CH128_T256 static inline __m256i ch128_256_xor(__m256i a,__m256i b) { return _mm256_xor_si256(a,b); }
CH128_T256 static inline __m256i ch128_256_swap(__m256i a) { return _mm256_shuffle_epi32(a,0x4e); }
CH128_T256 static inline __m256i ch128_256_left(__m256i a) { return _mm256_bslli_epi128(a,8); }
CH128_T256 static inline __m256i ch128_256_right(__m256i a) { return _mm256_bsrli_epi128(a,8); }
CH128_T256 static inline void ch128_256_store(void *p,__m256i a) { _mm256_storeu_si256((__m256i_u *)p,a); }
CH128_T256 static inline __m256i ch128_256_bc(const void *p) { return _mm256_broadcastsi128_si256(_mm_loadu_si128((const __m128i_u *)p)); }
typedef struct { __m256i l,h,m; } ch128_256_acc;
typedef struct { __m256i lo,hi; } ch128_256_raw;
CH128_T256 static inline ch128_256_acc ch128_256_azero(void) { ch128_256_acc r; r.l=r.h=r.m=ch128_256_zero(); return r; }
CH128_T256 static inline ch128_256_acc ch128_256_accum(ch128_256_acc r,__m256i a,__m256i b,int school) {
    __m256i l=ch128_256_ll(a,b),h=ch128_256_hh(a,b),m;
    if(school) m=ch128_256_xor(ch128_256_lh(a,b),ch128_256_hl(a,b));
    else m=ch128_256_ll(ch128_256_xor(a,ch128_256_swap(a)),ch128_256_xor(b,ch128_256_swap(b)));
    r.l=ch128_256_xor(r.l,l); r.h=ch128_256_xor(r.h,h); r.m=ch128_256_xor(r.m,m); __asm__("" : "+x"(r.l), "+x"(r.h), "+x"(r.m)); return r;
}
CH128_T256 static inline ch128_256_raw ch128_256_pack(ch128_256_acc a,int school) {
    ch128_256_raw r; if(!school) a.m=ch128_256_xor(a.m,ch128_256_xor(a.l,a.h));
    r.lo=ch128_256_xor(a.l,ch128_256_left(a.m)); r.hi=ch128_256_xor(a.h,ch128_256_right(a.m)); return r;
}
CH128_T256 static inline ch128_256_raw ch128_256_prod(__m256i a,__m256i b,int school) { return ch128_256_pack(ch128_256_accum(ch128_256_azero(),a,b,school),school); }
#endif

#ifdef CH128_X86
CH128_T512 static inline __m512i ch128_512_ll(__m512i a,__m512i b) { return _mm512_clmulepi64_epi128(a,b,0); }
CH128_T512 static inline __m512i ch128_512_hh(__m512i a,__m512i b) { return _mm512_clmulepi64_epi128(a,b,0x11); }
CH128_T512 static inline __m512i ch128_512_lh(__m512i a,__m512i b) { return _mm512_clmulepi64_epi128(a,b,0x10); }
CH128_T512 static inline __m512i ch128_512_hl(__m512i a,__m512i b) { return _mm512_clmulepi64_epi128(a,b,0x01); }
CH128_T512 static inline __m512i ch128_512_zero(void) { return _mm512_setzero_si512(); }
CH128_T512 static inline __m512i ch128_512_load(const void *p) { return _mm512_loadu_si512(p); }
CH128_T512 static inline __m512i ch128_512_xor(__m512i a,__m512i b) { return _mm512_xor_si512(a,b); }
CH128_T512 static inline __m512i ch128_512_swap(__m512i a) { return _mm512_shuffle_epi32(a,(_MM_PERM_ENUM)0x4e); }
CH128_T512 static inline __m512i ch128_512_left(__m512i a) { return _mm512_maskz_shuffle_epi32(0xcccc, a, (_MM_PERM_ENUM)0x4e); }
CH128_T512 static inline __m512i ch128_512_right(__m512i a) { return _mm512_maskz_shuffle_epi32(0x3333, a, (_MM_PERM_ENUM)0x4e); }
CH128_T512 static inline void ch128_512_store(void *p,__m512i a) { _mm512_storeu_si512(p,a); }
CH128_T512 static inline __m512i ch128_512_bc(const void *p) { return _mm512_broadcast_i32x4(_mm_loadu_si128((const __m128i_u *)p)); }
typedef struct { __m512i l,h,m; } ch128_512_acc;
typedef struct { __m512i lo,hi; } ch128_512_raw;
CH128_T512 static inline ch128_512_acc ch128_512_azero(void) { ch128_512_acc r; r.l=r.h=r.m=ch128_512_zero(); return r; }
CH128_T512 static inline ch128_512_acc ch128_512_accum(ch128_512_acc r,__m512i a,__m512i b,int school) {
    __m512i l=ch128_512_ll(a,b),h=ch128_512_hh(a,b),m;
    if(school) m=_mm512_ternarylogic_epi64(r.m,ch128_512_lh(a,b),ch128_512_hl(a,b),0x96);
    else m=ch128_512_ll(ch128_512_xor(a,ch128_512_swap(a)),ch128_512_xor(b,ch128_512_swap(b)));
    r.l=ch128_512_xor(r.l,l); r.h=ch128_512_xor(r.h,h); r.m=school?m:ch128_512_xor(r.m,m); __asm__("" : "+v"(r.l), "+v"(r.h), "+v"(r.m)); return r;
}
/* Two products per step, each component folded with one three-input XOR. The volatile
 * barrier on the accumulators keeps the compiler from regrouping the sums into long-lived
 * trees and is a scheduling boundary, so no tuning (e.g. Clang -march=znver4) pulls the
 * next step's loads and products above it and spills them. */
CH128_T512 static inline ch128_512_acc ch128_512_accum2(ch128_512_acc r,__m512i a,__m512i b,__m512i c,__m512i d,int school) {
    r.l=_mm512_ternarylogic_epi64(r.l,ch128_512_ll(a,b),ch128_512_ll(c,d),0x96);
    r.h=_mm512_ternarylogic_epi64(r.h,ch128_512_hh(a,b),ch128_512_hh(c,d),0x96);
    if(school) r.m=_mm512_ternarylogic_epi64(_mm512_ternarylogic_epi64(r.m,ch128_512_lh(a,b),ch128_512_hl(a,b),0x96),ch128_512_lh(c,d),ch128_512_hl(c,d),0x96);
    else r.m=_mm512_ternarylogic_epi64(r.m,ch128_512_ll(ch128_512_xor(a,ch128_512_swap(a)),ch128_512_xor(b,ch128_512_swap(b))),ch128_512_ll(ch128_512_xor(c,ch128_512_swap(c)),ch128_512_xor(d,ch128_512_swap(d))),0x96);
    __asm__ volatile("" : "+v"(r.l), "+v"(r.h), "+v"(r.m)); return r;
}
CH128_T512 static inline ch128_512_raw ch128_512_pack(ch128_512_acc a,int school) {
    ch128_512_raw r; if(!school) a.m=ch128_512_xor(a.m,ch128_512_xor(a.l,a.h));
    r.lo=ch128_512_xor(a.l,ch128_512_left(a.m)); r.hi=ch128_512_xor(a.h,ch128_512_right(a.m)); return r;
}
CH128_T512 static inline ch128_512_raw ch128_512_prod(__m512i a,__m512i b,int school) { return ch128_512_pack(ch128_512_accum(ch128_512_azero(),a,b,school),school); }
/* N consecutive raw lane states (a stream's) as one vector state. */
#define CH128_XIN(P,T,N) T static inline P##_raw P##_in(const ch128_raw *st) { \
    ch128_word lo[N],hi[N]; unsigned t; P##_raw r; for(t=0;t<N;t++) { lo[t]=st[t].lo; hi[t]=st[t].hi; } r.lo=P##_load(lo); r.hi=P##_load(hi); return r; }
CH128_XIN(ch128_128,CH128_T128,1)
CH128_XIN(ch128_256,CH128_T256,2)
CH128_XIN(ch128_512,CH128_T512,4)
#undef CH128_XIN
#endif

#ifdef CH128_ARM
CH128_NBEGIN
static inline uint64x2_t ch128_nld(const void *p) { uint64x2_t a; __asm__("ld1 {%0.2d}, [%1]":"=w"(a):"r"(p),"m"(*(const uint8_t (*)[16])p)); return a; }
static inline uint64x2_t ch128_n_ll(uint64x2_t a,uint64x2_t b) { uint64x2_t r; __asm__("pmull %0.1q, %1.1d, %2.1d":"=w"(r):"w"(a),"w"(b)); return r; }
static inline uint64x2_t ch128_n_hh(uint64x2_t a,uint64x2_t b) { uint64x2_t r; __asm__("pmull2 %0.1q, %1.2d, %2.2d":"=w"(r):"w"(a),"w"(b)); return r; }
static inline uint64x2_t ch128_n_lh(uint64x2_t a,uint64x2_t b) { return ch128_n_ll(a,vextq_u64(b,b,1)); }
static inline uint64x2_t ch128_n_hl(uint64x2_t a,uint64x2_t b) { return ch128_n_ll(vextq_u64(a,a,1),b); }
static inline uint64x2_t ch128_n_zero(void) { return vdupq_n_u64(0); }
static inline uint64x2_t ch128_n_load(const void *p) { return ch128_nld(p); }
static inline uint64x2_t ch128_n_xor(uint64x2_t a,uint64x2_t b) { return veorq_u64(a,b); }
static inline uint64x2_t ch128_n_swap(uint64x2_t a) { return vextq_u64(a,a,1); }
static inline uint64x2_t ch128_n_left(uint64x2_t a) { return vextq_u64(vdupq_n_u64(0),a,1); }
static inline uint64x2_t ch128_n_right(uint64x2_t a) { return vextq_u64(a,vdupq_n_u64(0),1); }
static inline void ch128_n_store(void *p,uint64x2_t a) { vst1q_u8((uint8_t *)p,vreinterpretq_u8_u64(a)); }
static inline uint64x2_t ch128_n_bc(const void *p) { return vld1q_u64((const uint64_t *)p); }
typedef struct { uint64x2_t l,h,m; } ch128_n_acc;
typedef struct { uint64x2_t lo,hi; } ch128_n_raw;
static inline ch128_n_acc ch128_n_azero(void) { ch128_n_acc r; r.l=r.h=r.m=ch128_n_zero(); return r; }
static inline ch128_n_acc ch128_n_accum(ch128_n_acc r,uint64x2_t a,uint64x2_t b,int school) {
    uint64x2_t l=ch128_n_ll(a,b),h=ch128_n_hh(a,b),m;
    if(school) m=ch128_n_xor(ch128_n_lh(a,b),ch128_n_hl(a,b));
    else m=ch128_n_ll(ch128_n_xor(a,ch128_n_swap(a)),ch128_n_xor(b,ch128_n_swap(b)));
    r.l=ch128_n_xor(r.l,l); r.h=ch128_n_xor(r.h,h); r.m=ch128_n_xor(r.m,m); __asm__("" : "+w"(r.l), "+w"(r.h), "+w"(r.m)); return r;
}
static inline ch128_n_raw ch128_n_pack(ch128_n_acc a,int school) {
    ch128_n_raw r; if(!school) a.m=ch128_n_xor(a.m,ch128_n_xor(a.l,a.h));
    r.lo=ch128_n_xor(a.l,ch128_n_left(a.m)); r.hi=ch128_n_xor(a.h,ch128_n_right(a.m)); return r;
}
static inline ch128_n_raw ch128_n_prod(uint64x2_t a,uint64x2_t b,int school) { return ch128_n_pack(ch128_n_accum(ch128_n_azero(),a,b,school),school); }
static inline ch128_raw ch128_n_wordprod(ch128_word a,ch128_word b,int school) {
    ch128_raw r; ch128_n_raw v=ch128_n_prod(ch128_n_load(&a),ch128_n_load(&b),school); ch128_n_store(&r.lo,v.lo); ch128_n_store(&r.hi,v.hi); return r;
}
CH128_NEND
#endif

static inline ch128_raw ch128_prod(ch128_word a,ch128_word b,int backend,int school) {
#ifdef CH128_X86
    if(backend) return ch128_128_wordprod(a,b,school);
#elif defined(CH128_ARM)
    if(backend) return ch128_n_wordprod(a,b,school);
#else
    (void)backend; (void)school;
#endif
    return ch128_clmul_ref(a,b);
}
static inline ch128_word ch128_mul(ch128_word a,ch128_word b,int backend) { return ch128_reduce(ch128_prod(a,b,backend,0)); }
static inline ch128_word ch128_pow(ch128_word a,uint64_t n,int b) {
    ch128_word r=ch128_make(1,0); while(n) { if(n&1) r=ch128_mul(r,a,b); n>>=1; if(n) a=ch128_mul(a,a,b); } return r;
}
static inline chainhash128_key chainhash128_key_from_words(const ch128_word *w) {
    chainhash128_key k; unsigned i; int b=chainhash128_backend(); memcpy(k.ph,w,sizeof(k.ph));
    k.yp[0]=ch128_make(1,0); k.yh[0]=ch128_make(0x87,0);
    for(i=1;i<=8;i++) { k.yp[i]=ch128_mul(k.yp[i-1],w[CH128_W],b); k.yh[i]=ch128_mul(k.yp[i],k.yh[0],b); }
    for(i=0;i<8;i++) k.sk[i]=ch128_mul(w[1],k.yp[i],b);
    for(i=0;i<5;i++) k.c[i]=w[CH128_W+1+i];
    k.tau=w[CH128_W+6]; return k;
}
static inline chainhash128_key chainhash128_key_from_bytes(const uint8_t p[128]) {
    ch128_word w[CH128_W+7],s=ch128_load(p),v=s; unsigned i; int b=chainhash128_backend();
    for(i=0;i<CH128_W;i++) { w[i]=v; v=ch128_mul(v,s,b); }
    for(i=0;i<7;i++) w[CH128_W+i]=ch128_load(p+16*(i+1));
    return chainhash128_key_from_words(w);
}
/* Benchmark convenience: 64 bits of seed entropy, not 128 independent random bytes. */
static inline chainhash128_key chainhash128_key_from_seed(uint64_t seed) {
    uint8_t p[128]; unsigned i,j; for(i=0;i<16;i++) { uint64_t z=(seed+=UINT64_C(0x9e3779b97f4a7c15)); z=(z^(z>>30))*UINT64_C(0xbf58476d1ce4e5b9); z=(z^(z>>27))*UINT64_C(0x94d049bb133111eb); z^=z>>31; for(j=0;j<8;j++) p[8*i+j]=(uint8_t)(z>>(8*j)); } return chainhash128_key_from_bytes(p);
}
#ifdef CH128_X86
/* Latency forms. ch128_128_redh(hi,&e): the part of a reduction contributed by the high
 * half, hi*0x87 by two products (returned) and the carry-out fold (*e, by shifts, formed
 * while the products run); (p mod f) ^ c = (p.lo ^ c ^ e) ^ result, so a constant enters a
 * reduction without a level of its own. */
CH128_T128 static inline __m128i ch128_128_redh(__m128i hi,__m128i *e) {
    const __m128i r=_mm_cvtsi32_si128(0x87);
    __m128i t=ch128_128_right(ch128_128_xor(_mm_srli_epi64(hi,63),ch128_128_xor(_mm_srli_epi64(hi,62),_mm_srli_epi64(hi,57))));
    *e=ch128_128_xor(t,ch128_128_xor(_mm_slli_epi64(t,1),ch128_128_xor(_mm_slli_epi64(t,2),_mm_slli_epi64(t,7))));
    return ch128_128_xor(ch128_128_ll(hi,r),ch128_128_left(ch128_128_hl(hi,r)));
}
/* z*X^128 mod f by shifts (z*0x87 = z ^ z<<1 ^ z<<2 ^ z<<7 as 128-bit shifts, lane carries c,
 * lane 1's carries overflowing and folded again): no product. */
CH128_T128 static inline __m128i ch128_128_x128(__m128i z) {
    __m128i c=ch128_128_xor(_mm_srli_epi64(z,63),ch128_128_xor(_mm_srli_epi64(z,62),_mm_srli_epi64(z,57))),t=ch128_128_right(c);
    return ch128_128_xor(ch128_128_xor(ch128_128_xor(z,_mm_slli_epi64(z,1)),ch128_128_xor(_mm_slli_epi64(z,2),_mm_slli_epi64(z,7))),
                         ch128_128_xor(ch128_128_xor(t,ch128_128_left(c)),ch128_128_xor(_mm_slli_epi64(t,1),ch128_128_xor(_mm_slli_epi64(t,2),_mm_slli_epi64(t,7)))));
}
CH128_T128 static inline __m128i ch128_128_redc(ch128_128_raw p,__m128i c) {
    __m128i e,h=ch128_128_redh(p.hi,&e); return ch128_128_xor(ch128_128_xor(p.lo,ch128_128_xor(c,e)),h);
}
CH128_T128 static inline __m128i ch128_128_reduce(ch128_128_raw p) { return ch128_128_redc(p,ch128_128_zero()); }
CH128_T128 static inline __m128i ch128_128_mul(__m128i a,__m128i b) { return ch128_128_reduce(ch128_128_prod(a,b,0)); }
/* (a + b) mod 2^128 as integers, in a register: lane sums, then the low lane's carry,
 * detected by an unsigned comparison through the sign-flipped signed one. */
CH128_T128 static inline __m128i ch128_128_addint(__m128i a,__m128i b) {
    const __m128i sign=_mm_set1_epi64x((long long)0x8000000000000000ull);
    __m128i s=_mm_add_epi64(a,b),c=_mm_cmpgt_epi64(_mm_xor_si128(a,sign),_mm_xor_si128(s,sign));
    return _mm_sub_epi64(s,_mm_slli_si128(c,8));
}
/* x = v + tau: the sign-flipped sum v + (tau ^ 2^63) comes from its own addition, so the
 * carry test waits on one addition, not two operations. */
CH128_T128 static inline __m128i ch128_128_twist(const chainhash128_key *k,__m128i v) {
    const __m128i sign=_mm_set_epi64x(0,(long long)0x8000000000000000ull);
    __m128i t=ch128_128_load(&k->tau),ts=_mm_xor_si128(t,sign);
    __m128i s=_mm_add_epi64(v,t),c=_mm_cmpgt_epi64(ts,_mm_add_epi64(v,ts));
    return _mm_sub_epi64(s,_mm_slli_si128(c,8));
}
/* The finalizer on v (reduced): x = v + tau, q = x^2, R = (q^c0)(x^q^c1), out = z*(R^c3) ^ c4
 * with z = x^c2. q^c0 and x^q^c1 share the reduction of x^2 with the constants injected; the
 * last product absorbs the reduction of R: z*(R^c3) = z*(R.lo^c3) ^ w*R.hi with w = z*X^128 mod f
 * (by shifts). Three product levels and two reductions after the twist. Where PCLMULQDQ issues
 * every cycle (fast), z*c3 ^ c4 is instead reduced beside R (one XOR off the chain, six more
 * products); AMD issues one product per two cycles and takes the form with fewer products. */
CH128_T128 static inline ch128_word ch128_128_fin(const chainhash128_key *k,__m128i v,int school) {
    __m128i x=ch128_128_twist(k,v),e,H,a,b,z,w,F; ch128_128_raw Q,R; ch128_128_acc A; ch128_word o;
    Q.lo=ch128_128_ll(x,x); Q.hi=ch128_128_hh(x,x); H=ch128_128_redh(Q.hi,&e);
    a=ch128_128_xor(ch128_128_xor(Q.lo,ch128_128_xor(e,ch128_128_load(k->c))),H);
    b=ch128_128_xor(ch128_128_xor(Q.lo,ch128_128_xor(e,ch128_128_xor(x,ch128_128_load(k->c+1)))),H);
    z=ch128_128_xor(x,ch128_128_load(k->c+2)); w=ch128_128_x128(z); R=ch128_128_prod(a,b,school);
    if(ch128_pclmul_fast()) {
        F=ch128_128_redc(ch128_128_prod(z,ch128_128_load(k->c+3),school),ch128_128_load(k->c+4));
        A=ch128_128_accum(ch128_128_accum(ch128_128_azero(),z,R.lo,school),w,R.hi,school);
    } else {
        F=ch128_128_load(k->c+4);
        A=ch128_128_accum(ch128_128_accum(ch128_128_azero(),z,ch128_128_xor(R.lo,ch128_128_load(k->c+3)),school),w,R.hi,school);
    }
    v=ch128_128_redc(ch128_128_pack(A,school),F); o.lo=(uint64_t)_mm_cvtsi128_si64(v); o.hi=(uint64_t)_mm_extract_epi64(v,1); return o;
}
CH128_T128 static inline ch128_word ch128_128_finish(const chainhash128_key *k,__m128i vv) { return ch128_128_fin(k,vv,ch128_pclmul_fast()); }
/* The 1..15 bytes at p, zero-extended, from overlapping in-bounds loads (no stack word). */
CH128_T128 static inline __m128i ch128_128_ldsmall(const uint8_t *p,size_t n) {
    uint64_t lo,hi=0;
    if(n>=8) { memcpy(&lo,p,8); if(n>8) { memcpy(&hi,p+n-8,8); hi>>=8*(16-n); } }
    else if(n>=4) { uint32_t a,b; memcpy(&a,p,4); memcpy(&b,p+n-4,4); lo=a|(uint64_t)b<<(8*(n-4)); }
    else lo=(uint64_t)p[0]|(uint64_t)p[n>>1]<<(8*(n>>1))|(uint64_t)p[n-1]<<(8*(n-1));
    return _mm_set_epi64x((long long)hi,(long long)lo);
}
/* For n<=128 every comb partner is absent, so V = n*y^p + kb*sum_j (w_j+ka)*y^(p-1-j). With the
 * key-side weights sk[e] = kb*y^e every word meets one product: all products (and the length
 * term) accumulate unreduced, one reduction, the finalizer. Full words are loaded in place, the
 * last partial word as the 16 bytes ending at p+n shifted down (no read outside the input). */
CH128_T128 static inline ch128_word ch128_128_short(const chainhash128_key *k,const uint8_t *p,size_t n,int school) {
    static const uint8_t shift_idx[32]={0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,
        0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80};
    unsigned count=(unsigned)((n+15)/16),j; size_t rem;
    __m128i ka,yp,lv,l,m,last; ch128_128_acc a=ch128_128_azero();
    if(!count) return ch128_128_fin(k,ch128_128_zero(),school);
    ka=ch128_128_load(k->ph); rem=n-16*(size_t)(count-1);
    if(rem==16) last=ch128_128_load(p+n-16);
    else if(n>=16) last=_mm_shuffle_epi8(ch128_128_load(p+n-16),ch128_128_load(shift_idx+16-rem));
    else last=ch128_128_ldsmall(p,n);
    for(j=0;j+1<count;j++) a=ch128_128_accum(a,ch128_128_xor(ch128_128_load(p+16*j),ka),ch128_128_load(k->sk+count-1-j),school);
    a=ch128_128_accum(a,ch128_128_xor(last,ka),ch128_128_load(k->sk),school);
    yp=ch128_128_load(k->yp+count); lv=_mm_set1_epi64x((long long)n);   /* {n,n}: hh supplies yp.hi*n */
    l=ch128_128_ll(yp,lv); m=ch128_128_hh(yp,lv);
    a.l=ch128_128_xor(a.l,l); a.m=ch128_128_xor(a.m,school ? m : ch128_128_xor(l,m));
    return ch128_128_fin(k,ch128_128_reduce(ch128_128_pack(a,school)),school);
}
#endif
#ifdef CH128_ARM
CH128_NBEGIN
/* ch128_n_redh returns hi*0x87 mod X^128 (two products) and sets *e to the carry-out of
 * hi.hi*0x87 times 0x87 (a third product, not a shift fold: on NEON a chain of eight
 * dependent shifts and XORs is slower than one more product), so
 * (p mod f) ^ c = (p.lo ^ c ^ e) ^ result. */
static inline uint64x2_t ch128_n_redh(uint64x2_t hi,uint64x2_t *e) {
    const uint64x2_t r=vdupq_n_u64(0x87); uint64x2_t b=ch128_n_hh(hi,r);
    *e=ch128_n_hh(b,r);
    return ch128_n_xor(ch128_n_ll(hi,r),ch128_n_left(b));
}
static inline uint64x2_t ch128_n_redc(ch128_n_raw p,uint64x2_t c) {
    uint64x2_t e,h=ch128_n_redh(p.hi,&e); return ch128_n_xor(ch128_n_xor(p.lo,ch128_n_xor(c,e)),h);
}
static inline uint64x2_t ch128_n_reduce(ch128_n_raw p) { return ch128_n_redc(p,ch128_n_zero()); }
static inline uint64x2_t ch128_n_mul(uint64x2_t a,uint64x2_t b) { return ch128_n_reduce(ch128_n_prod(a,b,0)); }
/* x = v + tau in a register: lane sums, the low lane's carry by an unsigned compare, moved up. */
static inline uint64x2_t ch128_n_twist(const chainhash128_key *k,uint64x2_t v) {
    uint64x2_t t=ch128_n_load(&k->tau),s=vaddq_u64(v,t);
    return vsubq_u64(s,vextq_u64(vdupq_n_u64(0),vcltq_u64(s,t),1));
}
/* The finalizer on v (reduced), as ch128_128_fin in its form with fewer products: z*(R^c3) =
 * z*(R.lo^c3) ^ w*R.hi, w = z*X^128 mod f. */
static inline ch128_word ch128_n_fin(const chainhash128_key *k,uint64x2_t v,int school) {
    uint64x2_t x=ch128_n_twist(k,v),e,H,a,b,z,w,F; ch128_n_raw Q,R,zr; ch128_n_acc A; ch128_word o;
    Q.lo=ch128_n_ll(x,x); Q.hi=ch128_n_hh(x,x); H=ch128_n_redh(Q.hi,&e);
    a=ch128_n_xor(ch128_n_xor(Q.lo,ch128_n_xor(e,ch128_n_load(k->c))),H);
    b=ch128_n_xor(ch128_n_xor(Q.lo,ch128_n_xor(e,ch128_n_xor(x,ch128_n_load(k->c+1)))),H);
    z=ch128_n_xor(x,ch128_n_load(k->c+2)); zr.lo=ch128_n_zero(); zr.hi=z; w=ch128_n_reduce(zr);
    R=ch128_n_prod(a,b,school); F=ch128_n_load(k->c+4);
    A=ch128_n_accum(ch128_n_accum(ch128_n_azero(),z,ch128_n_xor(R.lo,ch128_n_load(k->c+3)),school),w,R.hi,school);
    v=ch128_n_redc(ch128_n_pack(A,school),F); o.lo=vgetq_lane_u64(v,0); o.hi=vgetq_lane_u64(v,1); return o;
}
static inline ch128_word ch128_n_finish(const chainhash128_key *k,uint64x2_t vv) { return ch128_n_fin(k,vv,1); }
/* The 16-byte word at q+o with the bytes at or past q+rem zeroed, read in place
 * (the partial word as the 16 bytes ending at q+rem, shifted down by TBL); the
 * caller guarantees q+rem-16 is readable. No buffer, so no store-forwarding stall. */
static inline uint64x2_t ch128_n_word(const uint8_t *q,size_t o,size_t rem) {
    static const uint8_t idx[32]={0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,
        0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80};
    if(o+16<=rem) return ch128_n_load(q+o);
    if(o>=rem) return ch128_n_zero();
    return vreinterpretq_u64_u8(vqtbl1q_u8(vreinterpretq_u8_u64(ch128_n_load(q+rem-16)),vld1q_u8(idx+16-(rem-o))));
}
/* For n<=128 every comb partner is absent: V = n*y^p + sum_j (w_j+ka)*(kb*y^(p-1-j)) with the
 * key-side weights sk, one product level, one reduction; registers only (1..15 bytes by
 * overlapping scalar loads). */
static inline ch128_word ch128_n_short(const chainhash128_key *k,const uint8_t *p,size_t n,int school) {
    unsigned count=(unsigned)((n+15)/16),j; uint64x2_t ka,yp,lv,l,m,last; ch128_n_acc a=ch128_n_azero();
    if(!count) return ch128_n_fin(k,ch128_n_zero(),school);
    ka=ch128_n_load(k->ph);
    if(n>=16) last=ch128_n_word(p,16*(size_t)(count-1),n);
    else { uint64_t lo,hi=0;
        if(n>=8) { memcpy(&lo,p,8); if(n>8) { memcpy(&hi,p+n-8,8); hi>>=8*(16-n); } }
        else if(n>=4) { uint32_t u,v; memcpy(&u,p,4); memcpy(&v,p+n-4,4); lo=u|(uint64_t)v<<(8*(n-4)); }
        else lo=(uint64_t)p[0]|(uint64_t)p[n>>1]<<(8*(n>>1))|(uint64_t)p[n-1]<<(8*(n-1));
        last=vcombine_u64(vcreate_u64(lo),vcreate_u64(hi)); }
    for(j=0;j+1<count;j++) a=ch128_n_accum(a,ch128_n_xor(ch128_n_load(p+16*j),ka),ch128_n_load(k->sk+count-1-j),school);
    a=ch128_n_accum(a,ch128_n_xor(last,ka),ch128_n_load(k->sk),school);
    yp=ch128_n_load(k->yp+count); lv=vdupq_n_u64((uint64_t)n);   /* {n,n}: pmull2 supplies yp.hi*n */
    l=ch128_n_ll(yp,lv); m=ch128_n_hh(yp,lv);
    a.l=ch128_n_xor(a.l,l); a.m=ch128_n_xor(a.m,school ? m : ch128_n_xor(l,m));
    return ch128_n_fin(k,ch128_n_reduce(ch128_n_pack(a,school)),school);
}
CH128_NEND
#endif
#ifdef CH128_X86
CH128_T128 static inline ch128_word ch128_128_finish_word(const chainhash128_key *k,ch128_word v) { return ch128_128_finish(k,ch128_128_load(&v)); }
#endif
static inline ch128_word ch128_finish(const chainhash128_key *k,ch128_word v,int b) {
#ifdef CH128_X86
    if(b) return ch128_128_finish_word(k,v);
#elif defined(CH128_ARM)
    if(b) return ch128_n_finish(k,ch128_n_load(&v));
#endif
    ch128_word q,r; v=ch128_addint(v,k->tau); q=ch128_mul(v,v,b);
    r=ch128_mul(ch128_xor(q,k->c[0]),ch128_xor(ch128_xor(v,q),k->c[1]),b);
    return ch128_xor(ch128_mul(ch128_xor(v,k->c[2]),ch128_xor(r,k->c[3]),b),k->c[4]);
}
static inline unsigned ch128_lanes(size_t n) { return n>112 ? 8 : n ? (unsigned)((n-1)/16+1) : 1; }
static inline void ch128_region_scalar(const chainhash128_key *k,const uint8_t *p,size_t n,ch128_raw out[8],int b,int school) {
    unsigned c,j; memset(out,0,8*sizeof(*out));
    for(c=0;c<CH128_CHUNKS && 256*c<n;c++) for(j=0;j<8 && 256*c+16*j<n;j++) { size_t off=256*c+16*j;
        if(off<n) out[j]=ch128_rxor(out[j],ch128_prod(ch128_xor(ch128_partial(p,n,off),k->ph[2*c]),ch128_xor(ch128_partial(p,n,off+128),k->ph[2*c+1]),b,school));
    }
}

#ifdef CH128_X86
CH128_T128 static void ch128_128_region(const chainhash128_key *k,const uint8_t *p,ch128_raw out[8],int school) {
    unsigned j,c; for(j=0;j<8;j+=1) { ch128_128_acc a=ch128_128_azero(); ch128_128_raw r;
        for(c=0;c<CH128_CHUNKS;c++) a=ch128_128_accum(a,ch128_128_xor(ch128_128_load(p+256*c+16*j),ch128_128_bc(k->ph+2*c)),ch128_128_xor(ch128_128_load(p+256*c+16*j+128),ch128_128_bc(k->ph+2*c+1)),school);
        r=ch128_128_pack(a,school); { ch128_word lo[1],hi[1]; unsigned t; ch128_128_store(lo,r.lo); ch128_128_store(hi,r.hi); for(t=0;t<1;t++) { out[j+t].lo=lo[t]; out[j+t].hi=hi[t]; } }
    }
}
/* XMM bulk kernel, both product methods. Chunk-outer over lane pairs: a chunk's two keys
 * stay in registers for both lanes, so each data word is keyed by one register XOR and
 * at most 15 vector registers are live (6 accumulators, 2 keys, 4 operands, 3 temporaries)
 * on a 16-register ISA. The raw lane states live in memory and are touched once per
 * region. Karatsuba forms the middle operands of a pair as unpacklo(a,b)^unpackhi(a,b)
 * = (a.lo^a.hi, b.lo^b.hi) and multiplies them with one 0x10 product. */
#define CH128_XACC(L,H,M,A,B,SCHOOL) do { __m128i a_=(A),b_=(B); \
    L=_mm_xor_si128(L,_mm_clmulepi64_si128(a_,b_,0x00)); H=_mm_xor_si128(H,_mm_clmulepi64_si128(a_,b_,0x11)); \
    if(SCHOOL) M=_mm_xor_si128(M,_mm_xor_si128(_mm_clmulepi64_si128(a_,b_,0x01),_mm_clmulepi64_si128(a_,b_,0x10))); \
    else { __m128i t_=_mm_xor_si128(_mm_unpacklo_epi64(a_,b_),_mm_unpackhi_epi64(a_,b_)); M=_mm_xor_si128(M,_mm_clmulepi64_si128(t_,t_,0x10)); } \
    __asm__("" : "+x"(L), "+x"(H), "+x"(M)); } while(0)
CH128_T128 static inline void ch128_128_xpack(__m128i l,__m128i h,__m128i m,int school,__m128i *lo,__m128i *hi) {
    if(!school) m=_mm_xor_si128(m,_mm_xor_si128(l,h));
    *lo=_mm_xor_si128(l,_mm_slli_si128(m,8)); *hi=_mm_xor_si128(h,_mm_srli_si128(m,8));
}
CH128_T128 CH128_INLINE ch128_word ch128_128_bulk_run(const chainhash128_key *k,const uint8_t *p,size_t regions,size_t len,ch128_raw *st,const int school,const int hint,const unsigned step,size_t dist) {
    const __m128i z=_mm_setzero_si128();
    __m128i s[16]; unsigned j,c;
    for(j=0;j<16;j++) s[j]=z;
    { ch128_word init={0,0}; init.lo=len; s[14]=ch128_128_load(&init); }
    if(st) for(j=0;j<8;j++) { s[2*j]=ch128_128_load(&st[j].lo); s[2*j+1]=ch128_128_load(&st[j].hi); }
    do {
        CH128_PREFETCH(p,regions,CH128_REGION,hint,step,dist);
        for(j=0;j<8;j+=2) {
            __m128i l0=z,h0=z,m0=z,l1=z,h1=z,m1=z;
            for(c=0;c<CH128_CHUNKS;c++) {
                const __m128i ka=ch128_128_bc(k->ph+2*c),kb=ch128_128_bc(k->ph+2*c+1);
                const uint8_t *q=p+256*c+16*j;
                CH128_XACC(l0,h0,m0,_mm_xor_si128(ch128_128_load(q),ka),_mm_xor_si128(ch128_128_load(q+128),kb),school);
                CH128_XACC(l1,h1,m1,_mm_xor_si128(ch128_128_load(q+16),ka),_mm_xor_si128(ch128_128_load(q+144),kb),school);
            }
            { const __m128i y=ch128_128_bc(k->yp+8),h=ch128_128_bc(k->yh+8);
              CH128_XACC(l0,h0,m0,s[2*j],y,school); CH128_XACC(l0,h0,m0,s[2*j+1],h,school);
              CH128_XACC(l1,h1,m1,s[2*j+2],y,school); CH128_XACC(l1,h1,m1,s[2*j+3],h,school); }
            ch128_128_xpack(l0,h0,m0,school,&s[2*j],&s[2*j+1]);
            ch128_128_xpack(l1,h1,m1,school,&s[2*j+2],&s[2*j+3]);
        }
        p+=CH128_REGION;
    } while(--regions);
    { ch128_raw acc={{0,0},{0,0}}; ch128_word lo[8],hi[8];
      for(j=0;j<8;j++) { ch128_128_store(lo+j,s[2*j]); ch128_128_store(hi+j,s[2*j+1]); }
      if(st) { for(j=0;j<8;j++) { st[j].lo=lo[j]; st[j].hi=hi[j]; } return ch128_make(0,0); }
      for(j=0;j<8;j++) { acc=ch128_rxor(acc,ch128_prod(lo[j],k->yp[7-j],1,school)); acc=ch128_rxor(acc,ch128_prod(hi[j],k->yh[7-j],1,school)); }
      return ch128_reduce(acc); }
}
#undef CH128_XACC
CH128_T128 CH128_INLINE ch128_word ch128_128_bulk0_run(const chainhash128_key *k,const uint8_t *p,size_t regions,size_t len,ch128_raw *st,const int hint,const unsigned step,size_t dist) { return ch128_128_bulk_run(k,p,regions,len,st,0,hint,step,dist); }
CH128_T128 CH128_INLINE ch128_word ch128_128_bulk1_run(const chainhash128_key *k,const uint8_t *p,size_t regions,size_t len,ch128_raw *st,const int hint,const unsigned step,size_t dist) { return ch128_128_bulk_run(k,p,regions,len,st,1,hint,step,dist); }
CH128_T128 static ch128_word ch128_128_bulk0(const chainhash128_key *k,const uint8_t *p,size_t regions,size_t len,ch128_raw *st) { return ch128_128_bulk0_run(k,p,regions,len,st,CH_PF_OFF,64,0); }
CH128_T128 static ch128_word ch128_128_bulk1(const chainhash128_key *k,const uint8_t *p,size_t regions,size_t len,ch128_raw *st) { return ch128_128_bulk1_run(k,p,regions,len,st,CH_PF_OFF,64,0); }
CH128_T128 static ch128_word ch128_128_pf(const chainhash128_key *k,const uint8_t *p,size_t regions,size_t len,ch128_raw *st,int hint,unsigned step,size_t dist) { CH128_PF_SWITCH(ch128_128_bulk1,k,p,regions,len,st) }
CH128_T128 static ch128_word ch128_128_pfk(const chainhash128_key *k,const uint8_t *p,size_t regions,size_t len,ch128_raw *st,int hint,unsigned step,size_t dist) { CH128_PF_SWITCH(ch128_128_bulk0,k,p,regions,len,st) }
#endif

#ifdef CH128_X86
CH128_T256 static void ch128_256_region(const chainhash128_key *k,const uint8_t *p,ch128_raw out[8],int school) {
    unsigned j,c; for(j=0;j<8;j+=2) { ch128_256_acc a=ch128_256_azero(); ch128_256_raw r;
        for(c=0;c<CH128_CHUNKS;c++) a=ch128_256_accum(a,ch128_256_xor(ch128_256_load(p+256*c+16*j),ch128_256_bc(k->ph+2*c)),ch128_256_xor(ch128_256_load(p+256*c+16*j+128),ch128_256_bc(k->ph+2*c+1)),school);
        r=ch128_256_pack(a,school); { ch128_word lo[2],hi[2]; unsigned t; ch128_256_store(lo,r.lo); ch128_256_store(hi,r.hi); for(t=0;t<2;t++) { out[j+t].lo=lo[t]; out[j+t].hi=hi[t]; } }
    }
}
CH128_T256 CH128_INLINE ch128_word ch128_256_bulk0_run(const chainhash128_key *k,const uint8_t *p,size_t regions,size_t len,ch128_raw *st,const int hint,const unsigned step,size_t dist) {
    const __m256i y=ch128_256_bc(k->yp+8),h=ch128_256_bc(k->yh+8);
    ch128_256_raw s0; s0.lo=s0.hi=ch128_256_zero();
    ch128_256_raw s1; s1.lo=s1.hi=ch128_256_zero();
    ch128_256_raw s2; s2.lo=s2.hi=ch128_256_zero();
    ch128_256_raw s3; s3.lo=s3.hi=ch128_256_zero();
    { ch128_word init[2]={ {0,0} }; init[1].lo=len; s3.lo=ch128_256_load(init); }
    if(st) { s0=ch128_256_in(st); s1=ch128_256_in(st+2); s2=ch128_256_in(st+4); s3=ch128_256_in(st+6); }
    do {
        CH128_PREFETCH(p,regions,CH128_REGION,hint,step,dist);
        { ch128_256_acc a=ch128_256_azero();
          a=ch128_256_accum(a,ch128_256_xor(ch128_256_load(p+0),ch128_256_bc(k->ph+0)),ch128_256_xor(ch128_256_load(p+128),ch128_256_bc(k->ph+1)),0);
          a=ch128_256_accum(a,ch128_256_xor(ch128_256_load(p+256),ch128_256_bc(k->ph+2)),ch128_256_xor(ch128_256_load(p+384),ch128_256_bc(k->ph+3)),0);
          a=ch128_256_accum(a,ch128_256_xor(ch128_256_load(p+512),ch128_256_bc(k->ph+4)),ch128_256_xor(ch128_256_load(p+640),ch128_256_bc(k->ph+5)),0);
          a=ch128_256_accum(a,ch128_256_xor(ch128_256_load(p+768),ch128_256_bc(k->ph+6)),ch128_256_xor(ch128_256_load(p+896),ch128_256_bc(k->ph+7)),0);
          a=ch128_256_accum(a,ch128_256_xor(ch128_256_load(p+1024),ch128_256_bc(k->ph+8)),ch128_256_xor(ch128_256_load(p+1152),ch128_256_bc(k->ph+9)),0);
          a=ch128_256_accum(a,ch128_256_xor(ch128_256_load(p+1280),ch128_256_bc(k->ph+10)),ch128_256_xor(ch128_256_load(p+1408),ch128_256_bc(k->ph+11)),0);
          a=ch128_256_accum(a,ch128_256_xor(ch128_256_load(p+1536),ch128_256_bc(k->ph+12)),ch128_256_xor(ch128_256_load(p+1664),ch128_256_bc(k->ph+13)),0);
          a=ch128_256_accum(a,ch128_256_xor(ch128_256_load(p+1792),ch128_256_bc(k->ph+14)),ch128_256_xor(ch128_256_load(p+1920),ch128_256_bc(k->ph+15)),0);
#if CHAINHASH128_BLOCK_BYTES == 512
          a=ch128_256_accum(a,ch128_256_xor(ch128_256_load(p+2048),ch128_256_bc(k->ph+16)),ch128_256_xor(ch128_256_load(p+2176),ch128_256_bc(k->ph+17)),0);
          a=ch128_256_accum(a,ch128_256_xor(ch128_256_load(p+2304),ch128_256_bc(k->ph+18)),ch128_256_xor(ch128_256_load(p+2432),ch128_256_bc(k->ph+19)),0);
          a=ch128_256_accum(a,ch128_256_xor(ch128_256_load(p+2560),ch128_256_bc(k->ph+20)),ch128_256_xor(ch128_256_load(p+2688),ch128_256_bc(k->ph+21)),0);
          a=ch128_256_accum(a,ch128_256_xor(ch128_256_load(p+2816),ch128_256_bc(k->ph+22)),ch128_256_xor(ch128_256_load(p+2944),ch128_256_bc(k->ph+23)),0);
          a=ch128_256_accum(a,ch128_256_xor(ch128_256_load(p+3072),ch128_256_bc(k->ph+24)),ch128_256_xor(ch128_256_load(p+3200),ch128_256_bc(k->ph+25)),0);
          a=ch128_256_accum(a,ch128_256_xor(ch128_256_load(p+3328),ch128_256_bc(k->ph+26)),ch128_256_xor(ch128_256_load(p+3456),ch128_256_bc(k->ph+27)),0);
          a=ch128_256_accum(a,ch128_256_xor(ch128_256_load(p+3584),ch128_256_bc(k->ph+28)),ch128_256_xor(ch128_256_load(p+3712),ch128_256_bc(k->ph+29)),0);
          a=ch128_256_accum(a,ch128_256_xor(ch128_256_load(p+3840),ch128_256_bc(k->ph+30)),ch128_256_xor(ch128_256_load(p+3968),ch128_256_bc(k->ph+31)),0);
#endif
          a=ch128_256_accum(a,s0.lo,y,0); a=ch128_256_accum(a,s0.hi,h,0); s0=ch128_256_pack(a,0); }
        { ch128_256_acc a=ch128_256_azero();
          a=ch128_256_accum(a,ch128_256_xor(ch128_256_load(p+32),ch128_256_bc(k->ph+0)),ch128_256_xor(ch128_256_load(p+160),ch128_256_bc(k->ph+1)),0);
          a=ch128_256_accum(a,ch128_256_xor(ch128_256_load(p+288),ch128_256_bc(k->ph+2)),ch128_256_xor(ch128_256_load(p+416),ch128_256_bc(k->ph+3)),0);
          a=ch128_256_accum(a,ch128_256_xor(ch128_256_load(p+544),ch128_256_bc(k->ph+4)),ch128_256_xor(ch128_256_load(p+672),ch128_256_bc(k->ph+5)),0);
          a=ch128_256_accum(a,ch128_256_xor(ch128_256_load(p+800),ch128_256_bc(k->ph+6)),ch128_256_xor(ch128_256_load(p+928),ch128_256_bc(k->ph+7)),0);
          a=ch128_256_accum(a,ch128_256_xor(ch128_256_load(p+1056),ch128_256_bc(k->ph+8)),ch128_256_xor(ch128_256_load(p+1184),ch128_256_bc(k->ph+9)),0);
          a=ch128_256_accum(a,ch128_256_xor(ch128_256_load(p+1312),ch128_256_bc(k->ph+10)),ch128_256_xor(ch128_256_load(p+1440),ch128_256_bc(k->ph+11)),0);
          a=ch128_256_accum(a,ch128_256_xor(ch128_256_load(p+1568),ch128_256_bc(k->ph+12)),ch128_256_xor(ch128_256_load(p+1696),ch128_256_bc(k->ph+13)),0);
          a=ch128_256_accum(a,ch128_256_xor(ch128_256_load(p+1824),ch128_256_bc(k->ph+14)),ch128_256_xor(ch128_256_load(p+1952),ch128_256_bc(k->ph+15)),0);
#if CHAINHASH128_BLOCK_BYTES == 512
          a=ch128_256_accum(a,ch128_256_xor(ch128_256_load(p+2080),ch128_256_bc(k->ph+16)),ch128_256_xor(ch128_256_load(p+2208),ch128_256_bc(k->ph+17)),0);
          a=ch128_256_accum(a,ch128_256_xor(ch128_256_load(p+2336),ch128_256_bc(k->ph+18)),ch128_256_xor(ch128_256_load(p+2464),ch128_256_bc(k->ph+19)),0);
          a=ch128_256_accum(a,ch128_256_xor(ch128_256_load(p+2592),ch128_256_bc(k->ph+20)),ch128_256_xor(ch128_256_load(p+2720),ch128_256_bc(k->ph+21)),0);
          a=ch128_256_accum(a,ch128_256_xor(ch128_256_load(p+2848),ch128_256_bc(k->ph+22)),ch128_256_xor(ch128_256_load(p+2976),ch128_256_bc(k->ph+23)),0);
          a=ch128_256_accum(a,ch128_256_xor(ch128_256_load(p+3104),ch128_256_bc(k->ph+24)),ch128_256_xor(ch128_256_load(p+3232),ch128_256_bc(k->ph+25)),0);
          a=ch128_256_accum(a,ch128_256_xor(ch128_256_load(p+3360),ch128_256_bc(k->ph+26)),ch128_256_xor(ch128_256_load(p+3488),ch128_256_bc(k->ph+27)),0);
          a=ch128_256_accum(a,ch128_256_xor(ch128_256_load(p+3616),ch128_256_bc(k->ph+28)),ch128_256_xor(ch128_256_load(p+3744),ch128_256_bc(k->ph+29)),0);
          a=ch128_256_accum(a,ch128_256_xor(ch128_256_load(p+3872),ch128_256_bc(k->ph+30)),ch128_256_xor(ch128_256_load(p+4000),ch128_256_bc(k->ph+31)),0);
#endif
          a=ch128_256_accum(a,s1.lo,y,0); a=ch128_256_accum(a,s1.hi,h,0); s1=ch128_256_pack(a,0); }
        { ch128_256_acc a=ch128_256_azero();
          a=ch128_256_accum(a,ch128_256_xor(ch128_256_load(p+64),ch128_256_bc(k->ph+0)),ch128_256_xor(ch128_256_load(p+192),ch128_256_bc(k->ph+1)),0);
          a=ch128_256_accum(a,ch128_256_xor(ch128_256_load(p+320),ch128_256_bc(k->ph+2)),ch128_256_xor(ch128_256_load(p+448),ch128_256_bc(k->ph+3)),0);
          a=ch128_256_accum(a,ch128_256_xor(ch128_256_load(p+576),ch128_256_bc(k->ph+4)),ch128_256_xor(ch128_256_load(p+704),ch128_256_bc(k->ph+5)),0);
          a=ch128_256_accum(a,ch128_256_xor(ch128_256_load(p+832),ch128_256_bc(k->ph+6)),ch128_256_xor(ch128_256_load(p+960),ch128_256_bc(k->ph+7)),0);
          a=ch128_256_accum(a,ch128_256_xor(ch128_256_load(p+1088),ch128_256_bc(k->ph+8)),ch128_256_xor(ch128_256_load(p+1216),ch128_256_bc(k->ph+9)),0);
          a=ch128_256_accum(a,ch128_256_xor(ch128_256_load(p+1344),ch128_256_bc(k->ph+10)),ch128_256_xor(ch128_256_load(p+1472),ch128_256_bc(k->ph+11)),0);
          a=ch128_256_accum(a,ch128_256_xor(ch128_256_load(p+1600),ch128_256_bc(k->ph+12)),ch128_256_xor(ch128_256_load(p+1728),ch128_256_bc(k->ph+13)),0);
          a=ch128_256_accum(a,ch128_256_xor(ch128_256_load(p+1856),ch128_256_bc(k->ph+14)),ch128_256_xor(ch128_256_load(p+1984),ch128_256_bc(k->ph+15)),0);
#if CHAINHASH128_BLOCK_BYTES == 512
          a=ch128_256_accum(a,ch128_256_xor(ch128_256_load(p+2112),ch128_256_bc(k->ph+16)),ch128_256_xor(ch128_256_load(p+2240),ch128_256_bc(k->ph+17)),0);
          a=ch128_256_accum(a,ch128_256_xor(ch128_256_load(p+2368),ch128_256_bc(k->ph+18)),ch128_256_xor(ch128_256_load(p+2496),ch128_256_bc(k->ph+19)),0);
          a=ch128_256_accum(a,ch128_256_xor(ch128_256_load(p+2624),ch128_256_bc(k->ph+20)),ch128_256_xor(ch128_256_load(p+2752),ch128_256_bc(k->ph+21)),0);
          a=ch128_256_accum(a,ch128_256_xor(ch128_256_load(p+2880),ch128_256_bc(k->ph+22)),ch128_256_xor(ch128_256_load(p+3008),ch128_256_bc(k->ph+23)),0);
          a=ch128_256_accum(a,ch128_256_xor(ch128_256_load(p+3136),ch128_256_bc(k->ph+24)),ch128_256_xor(ch128_256_load(p+3264),ch128_256_bc(k->ph+25)),0);
          a=ch128_256_accum(a,ch128_256_xor(ch128_256_load(p+3392),ch128_256_bc(k->ph+26)),ch128_256_xor(ch128_256_load(p+3520),ch128_256_bc(k->ph+27)),0);
          a=ch128_256_accum(a,ch128_256_xor(ch128_256_load(p+3648),ch128_256_bc(k->ph+28)),ch128_256_xor(ch128_256_load(p+3776),ch128_256_bc(k->ph+29)),0);
          a=ch128_256_accum(a,ch128_256_xor(ch128_256_load(p+3904),ch128_256_bc(k->ph+30)),ch128_256_xor(ch128_256_load(p+4032),ch128_256_bc(k->ph+31)),0);
#endif
          a=ch128_256_accum(a,s2.lo,y,0); a=ch128_256_accum(a,s2.hi,h,0); s2=ch128_256_pack(a,0); }
        { ch128_256_acc a=ch128_256_azero();
          a=ch128_256_accum(a,ch128_256_xor(ch128_256_load(p+96),ch128_256_bc(k->ph+0)),ch128_256_xor(ch128_256_load(p+224),ch128_256_bc(k->ph+1)),0);
          a=ch128_256_accum(a,ch128_256_xor(ch128_256_load(p+352),ch128_256_bc(k->ph+2)),ch128_256_xor(ch128_256_load(p+480),ch128_256_bc(k->ph+3)),0);
          a=ch128_256_accum(a,ch128_256_xor(ch128_256_load(p+608),ch128_256_bc(k->ph+4)),ch128_256_xor(ch128_256_load(p+736),ch128_256_bc(k->ph+5)),0);
          a=ch128_256_accum(a,ch128_256_xor(ch128_256_load(p+864),ch128_256_bc(k->ph+6)),ch128_256_xor(ch128_256_load(p+992),ch128_256_bc(k->ph+7)),0);
          a=ch128_256_accum(a,ch128_256_xor(ch128_256_load(p+1120),ch128_256_bc(k->ph+8)),ch128_256_xor(ch128_256_load(p+1248),ch128_256_bc(k->ph+9)),0);
          a=ch128_256_accum(a,ch128_256_xor(ch128_256_load(p+1376),ch128_256_bc(k->ph+10)),ch128_256_xor(ch128_256_load(p+1504),ch128_256_bc(k->ph+11)),0);
          a=ch128_256_accum(a,ch128_256_xor(ch128_256_load(p+1632),ch128_256_bc(k->ph+12)),ch128_256_xor(ch128_256_load(p+1760),ch128_256_bc(k->ph+13)),0);
          a=ch128_256_accum(a,ch128_256_xor(ch128_256_load(p+1888),ch128_256_bc(k->ph+14)),ch128_256_xor(ch128_256_load(p+2016),ch128_256_bc(k->ph+15)),0);
#if CHAINHASH128_BLOCK_BYTES == 512
          a=ch128_256_accum(a,ch128_256_xor(ch128_256_load(p+2144),ch128_256_bc(k->ph+16)),ch128_256_xor(ch128_256_load(p+2272),ch128_256_bc(k->ph+17)),0);
          a=ch128_256_accum(a,ch128_256_xor(ch128_256_load(p+2400),ch128_256_bc(k->ph+18)),ch128_256_xor(ch128_256_load(p+2528),ch128_256_bc(k->ph+19)),0);
          a=ch128_256_accum(a,ch128_256_xor(ch128_256_load(p+2656),ch128_256_bc(k->ph+20)),ch128_256_xor(ch128_256_load(p+2784),ch128_256_bc(k->ph+21)),0);
          a=ch128_256_accum(a,ch128_256_xor(ch128_256_load(p+2912),ch128_256_bc(k->ph+22)),ch128_256_xor(ch128_256_load(p+3040),ch128_256_bc(k->ph+23)),0);
          a=ch128_256_accum(a,ch128_256_xor(ch128_256_load(p+3168),ch128_256_bc(k->ph+24)),ch128_256_xor(ch128_256_load(p+3296),ch128_256_bc(k->ph+25)),0);
          a=ch128_256_accum(a,ch128_256_xor(ch128_256_load(p+3424),ch128_256_bc(k->ph+26)),ch128_256_xor(ch128_256_load(p+3552),ch128_256_bc(k->ph+27)),0);
          a=ch128_256_accum(a,ch128_256_xor(ch128_256_load(p+3680),ch128_256_bc(k->ph+28)),ch128_256_xor(ch128_256_load(p+3808),ch128_256_bc(k->ph+29)),0);
          a=ch128_256_accum(a,ch128_256_xor(ch128_256_load(p+3936),ch128_256_bc(k->ph+30)),ch128_256_xor(ch128_256_load(p+4064),ch128_256_bc(k->ph+31)),0);
#endif
          a=ch128_256_accum(a,s3.lo,y,0); a=ch128_256_accum(a,s3.hi,h,0); s3=ch128_256_pack(a,0); }
        p+=CH128_REGION;
    } while(--regions);
    { ch128_raw acc={{0,0},{0,0}}; ch128_word lo[8],hi[8]; unsigned j;
      ch128_256_store(lo+0,s0.lo); ch128_256_store(hi+0,s0.hi);
      ch128_256_store(lo+2,s1.lo); ch128_256_store(hi+2,s1.hi);
      ch128_256_store(lo+4,s2.lo); ch128_256_store(hi+4,s2.hi);
      ch128_256_store(lo+6,s3.lo); ch128_256_store(hi+6,s3.hi);
      if(st) { for(j=0;j<8;j++) { st[j].lo=lo[j]; st[j].hi=hi[j]; } return ch128_make(0,0); }
      for(j=0;j<8;j++) { acc=ch128_rxor(acc,ch128_prod(lo[j],k->yp[7-j],1,0)); acc=ch128_rxor(acc,ch128_prod(hi[j],k->yh[7-j],1,0)); }
      return ch128_reduce(acc); }
}
CH128_T256 CH128_INLINE ch128_word ch128_256_bulk1_run(const chainhash128_key *k,const uint8_t *p,size_t regions,size_t len,ch128_raw *st,const int hint,const unsigned step,size_t dist) {
    const __m256i y=ch128_256_bc(k->yp+8),h=ch128_256_bc(k->yh+8);
    ch128_256_raw s0; s0.lo=s0.hi=ch128_256_zero();
    ch128_256_raw s1; s1.lo=s1.hi=ch128_256_zero();
    ch128_256_raw s2; s2.lo=s2.hi=ch128_256_zero();
    ch128_256_raw s3; s3.lo=s3.hi=ch128_256_zero();
    { ch128_word init[2]={ {0,0} }; init[1].lo=len; s3.lo=ch128_256_load(init); }
    if(st) { s0=ch128_256_in(st); s1=ch128_256_in(st+2); s2=ch128_256_in(st+4); s3=ch128_256_in(st+6); }
    do {
        CH128_PREFETCH(p,regions,CH128_REGION,hint,step,dist);
        { ch128_256_acc a=ch128_256_azero();
          a=ch128_256_accum(a,ch128_256_xor(ch128_256_load(p+0),ch128_256_bc(k->ph+0)),ch128_256_xor(ch128_256_load(p+128),ch128_256_bc(k->ph+1)),1);
          a=ch128_256_accum(a,ch128_256_xor(ch128_256_load(p+256),ch128_256_bc(k->ph+2)),ch128_256_xor(ch128_256_load(p+384),ch128_256_bc(k->ph+3)),1);
          a=ch128_256_accum(a,ch128_256_xor(ch128_256_load(p+512),ch128_256_bc(k->ph+4)),ch128_256_xor(ch128_256_load(p+640),ch128_256_bc(k->ph+5)),1);
          a=ch128_256_accum(a,ch128_256_xor(ch128_256_load(p+768),ch128_256_bc(k->ph+6)),ch128_256_xor(ch128_256_load(p+896),ch128_256_bc(k->ph+7)),1);
          a=ch128_256_accum(a,ch128_256_xor(ch128_256_load(p+1024),ch128_256_bc(k->ph+8)),ch128_256_xor(ch128_256_load(p+1152),ch128_256_bc(k->ph+9)),1);
          a=ch128_256_accum(a,ch128_256_xor(ch128_256_load(p+1280),ch128_256_bc(k->ph+10)),ch128_256_xor(ch128_256_load(p+1408),ch128_256_bc(k->ph+11)),1);
          a=ch128_256_accum(a,ch128_256_xor(ch128_256_load(p+1536),ch128_256_bc(k->ph+12)),ch128_256_xor(ch128_256_load(p+1664),ch128_256_bc(k->ph+13)),1);
          a=ch128_256_accum(a,ch128_256_xor(ch128_256_load(p+1792),ch128_256_bc(k->ph+14)),ch128_256_xor(ch128_256_load(p+1920),ch128_256_bc(k->ph+15)),1);
#if CHAINHASH128_BLOCK_BYTES == 512
          a=ch128_256_accum(a,ch128_256_xor(ch128_256_load(p+2048),ch128_256_bc(k->ph+16)),ch128_256_xor(ch128_256_load(p+2176),ch128_256_bc(k->ph+17)),1);
          a=ch128_256_accum(a,ch128_256_xor(ch128_256_load(p+2304),ch128_256_bc(k->ph+18)),ch128_256_xor(ch128_256_load(p+2432),ch128_256_bc(k->ph+19)),1);
          a=ch128_256_accum(a,ch128_256_xor(ch128_256_load(p+2560),ch128_256_bc(k->ph+20)),ch128_256_xor(ch128_256_load(p+2688),ch128_256_bc(k->ph+21)),1);
          a=ch128_256_accum(a,ch128_256_xor(ch128_256_load(p+2816),ch128_256_bc(k->ph+22)),ch128_256_xor(ch128_256_load(p+2944),ch128_256_bc(k->ph+23)),1);
          a=ch128_256_accum(a,ch128_256_xor(ch128_256_load(p+3072),ch128_256_bc(k->ph+24)),ch128_256_xor(ch128_256_load(p+3200),ch128_256_bc(k->ph+25)),1);
          a=ch128_256_accum(a,ch128_256_xor(ch128_256_load(p+3328),ch128_256_bc(k->ph+26)),ch128_256_xor(ch128_256_load(p+3456),ch128_256_bc(k->ph+27)),1);
          a=ch128_256_accum(a,ch128_256_xor(ch128_256_load(p+3584),ch128_256_bc(k->ph+28)),ch128_256_xor(ch128_256_load(p+3712),ch128_256_bc(k->ph+29)),1);
          a=ch128_256_accum(a,ch128_256_xor(ch128_256_load(p+3840),ch128_256_bc(k->ph+30)),ch128_256_xor(ch128_256_load(p+3968),ch128_256_bc(k->ph+31)),1);
#endif
          a=ch128_256_accum(a,s0.lo,y,1); a=ch128_256_accum(a,s0.hi,h,1); s0=ch128_256_pack(a,1); }
        { ch128_256_acc a=ch128_256_azero();
          a=ch128_256_accum(a,ch128_256_xor(ch128_256_load(p+32),ch128_256_bc(k->ph+0)),ch128_256_xor(ch128_256_load(p+160),ch128_256_bc(k->ph+1)),1);
          a=ch128_256_accum(a,ch128_256_xor(ch128_256_load(p+288),ch128_256_bc(k->ph+2)),ch128_256_xor(ch128_256_load(p+416),ch128_256_bc(k->ph+3)),1);
          a=ch128_256_accum(a,ch128_256_xor(ch128_256_load(p+544),ch128_256_bc(k->ph+4)),ch128_256_xor(ch128_256_load(p+672),ch128_256_bc(k->ph+5)),1);
          a=ch128_256_accum(a,ch128_256_xor(ch128_256_load(p+800),ch128_256_bc(k->ph+6)),ch128_256_xor(ch128_256_load(p+928),ch128_256_bc(k->ph+7)),1);
          a=ch128_256_accum(a,ch128_256_xor(ch128_256_load(p+1056),ch128_256_bc(k->ph+8)),ch128_256_xor(ch128_256_load(p+1184),ch128_256_bc(k->ph+9)),1);
          a=ch128_256_accum(a,ch128_256_xor(ch128_256_load(p+1312),ch128_256_bc(k->ph+10)),ch128_256_xor(ch128_256_load(p+1440),ch128_256_bc(k->ph+11)),1);
          a=ch128_256_accum(a,ch128_256_xor(ch128_256_load(p+1568),ch128_256_bc(k->ph+12)),ch128_256_xor(ch128_256_load(p+1696),ch128_256_bc(k->ph+13)),1);
          a=ch128_256_accum(a,ch128_256_xor(ch128_256_load(p+1824),ch128_256_bc(k->ph+14)),ch128_256_xor(ch128_256_load(p+1952),ch128_256_bc(k->ph+15)),1);
#if CHAINHASH128_BLOCK_BYTES == 512
          a=ch128_256_accum(a,ch128_256_xor(ch128_256_load(p+2080),ch128_256_bc(k->ph+16)),ch128_256_xor(ch128_256_load(p+2208),ch128_256_bc(k->ph+17)),1);
          a=ch128_256_accum(a,ch128_256_xor(ch128_256_load(p+2336),ch128_256_bc(k->ph+18)),ch128_256_xor(ch128_256_load(p+2464),ch128_256_bc(k->ph+19)),1);
          a=ch128_256_accum(a,ch128_256_xor(ch128_256_load(p+2592),ch128_256_bc(k->ph+20)),ch128_256_xor(ch128_256_load(p+2720),ch128_256_bc(k->ph+21)),1);
          a=ch128_256_accum(a,ch128_256_xor(ch128_256_load(p+2848),ch128_256_bc(k->ph+22)),ch128_256_xor(ch128_256_load(p+2976),ch128_256_bc(k->ph+23)),1);
          a=ch128_256_accum(a,ch128_256_xor(ch128_256_load(p+3104),ch128_256_bc(k->ph+24)),ch128_256_xor(ch128_256_load(p+3232),ch128_256_bc(k->ph+25)),1);
          a=ch128_256_accum(a,ch128_256_xor(ch128_256_load(p+3360),ch128_256_bc(k->ph+26)),ch128_256_xor(ch128_256_load(p+3488),ch128_256_bc(k->ph+27)),1);
          a=ch128_256_accum(a,ch128_256_xor(ch128_256_load(p+3616),ch128_256_bc(k->ph+28)),ch128_256_xor(ch128_256_load(p+3744),ch128_256_bc(k->ph+29)),1);
          a=ch128_256_accum(a,ch128_256_xor(ch128_256_load(p+3872),ch128_256_bc(k->ph+30)),ch128_256_xor(ch128_256_load(p+4000),ch128_256_bc(k->ph+31)),1);
#endif
          a=ch128_256_accum(a,s1.lo,y,1); a=ch128_256_accum(a,s1.hi,h,1); s1=ch128_256_pack(a,1); }
        { ch128_256_acc a=ch128_256_azero();
          a=ch128_256_accum(a,ch128_256_xor(ch128_256_load(p+64),ch128_256_bc(k->ph+0)),ch128_256_xor(ch128_256_load(p+192),ch128_256_bc(k->ph+1)),1);
          a=ch128_256_accum(a,ch128_256_xor(ch128_256_load(p+320),ch128_256_bc(k->ph+2)),ch128_256_xor(ch128_256_load(p+448),ch128_256_bc(k->ph+3)),1);
          a=ch128_256_accum(a,ch128_256_xor(ch128_256_load(p+576),ch128_256_bc(k->ph+4)),ch128_256_xor(ch128_256_load(p+704),ch128_256_bc(k->ph+5)),1);
          a=ch128_256_accum(a,ch128_256_xor(ch128_256_load(p+832),ch128_256_bc(k->ph+6)),ch128_256_xor(ch128_256_load(p+960),ch128_256_bc(k->ph+7)),1);
          a=ch128_256_accum(a,ch128_256_xor(ch128_256_load(p+1088),ch128_256_bc(k->ph+8)),ch128_256_xor(ch128_256_load(p+1216),ch128_256_bc(k->ph+9)),1);
          a=ch128_256_accum(a,ch128_256_xor(ch128_256_load(p+1344),ch128_256_bc(k->ph+10)),ch128_256_xor(ch128_256_load(p+1472),ch128_256_bc(k->ph+11)),1);
          a=ch128_256_accum(a,ch128_256_xor(ch128_256_load(p+1600),ch128_256_bc(k->ph+12)),ch128_256_xor(ch128_256_load(p+1728),ch128_256_bc(k->ph+13)),1);
          a=ch128_256_accum(a,ch128_256_xor(ch128_256_load(p+1856),ch128_256_bc(k->ph+14)),ch128_256_xor(ch128_256_load(p+1984),ch128_256_bc(k->ph+15)),1);
#if CHAINHASH128_BLOCK_BYTES == 512
          a=ch128_256_accum(a,ch128_256_xor(ch128_256_load(p+2112),ch128_256_bc(k->ph+16)),ch128_256_xor(ch128_256_load(p+2240),ch128_256_bc(k->ph+17)),1);
          a=ch128_256_accum(a,ch128_256_xor(ch128_256_load(p+2368),ch128_256_bc(k->ph+18)),ch128_256_xor(ch128_256_load(p+2496),ch128_256_bc(k->ph+19)),1);
          a=ch128_256_accum(a,ch128_256_xor(ch128_256_load(p+2624),ch128_256_bc(k->ph+20)),ch128_256_xor(ch128_256_load(p+2752),ch128_256_bc(k->ph+21)),1);
          a=ch128_256_accum(a,ch128_256_xor(ch128_256_load(p+2880),ch128_256_bc(k->ph+22)),ch128_256_xor(ch128_256_load(p+3008),ch128_256_bc(k->ph+23)),1);
          a=ch128_256_accum(a,ch128_256_xor(ch128_256_load(p+3136),ch128_256_bc(k->ph+24)),ch128_256_xor(ch128_256_load(p+3264),ch128_256_bc(k->ph+25)),1);
          a=ch128_256_accum(a,ch128_256_xor(ch128_256_load(p+3392),ch128_256_bc(k->ph+26)),ch128_256_xor(ch128_256_load(p+3520),ch128_256_bc(k->ph+27)),1);
          a=ch128_256_accum(a,ch128_256_xor(ch128_256_load(p+3648),ch128_256_bc(k->ph+28)),ch128_256_xor(ch128_256_load(p+3776),ch128_256_bc(k->ph+29)),1);
          a=ch128_256_accum(a,ch128_256_xor(ch128_256_load(p+3904),ch128_256_bc(k->ph+30)),ch128_256_xor(ch128_256_load(p+4032),ch128_256_bc(k->ph+31)),1);
#endif
          a=ch128_256_accum(a,s2.lo,y,1); a=ch128_256_accum(a,s2.hi,h,1); s2=ch128_256_pack(a,1); }
        { ch128_256_acc a=ch128_256_azero();
          a=ch128_256_accum(a,ch128_256_xor(ch128_256_load(p+96),ch128_256_bc(k->ph+0)),ch128_256_xor(ch128_256_load(p+224),ch128_256_bc(k->ph+1)),1);
          a=ch128_256_accum(a,ch128_256_xor(ch128_256_load(p+352),ch128_256_bc(k->ph+2)),ch128_256_xor(ch128_256_load(p+480),ch128_256_bc(k->ph+3)),1);
          a=ch128_256_accum(a,ch128_256_xor(ch128_256_load(p+608),ch128_256_bc(k->ph+4)),ch128_256_xor(ch128_256_load(p+736),ch128_256_bc(k->ph+5)),1);
          a=ch128_256_accum(a,ch128_256_xor(ch128_256_load(p+864),ch128_256_bc(k->ph+6)),ch128_256_xor(ch128_256_load(p+992),ch128_256_bc(k->ph+7)),1);
          a=ch128_256_accum(a,ch128_256_xor(ch128_256_load(p+1120),ch128_256_bc(k->ph+8)),ch128_256_xor(ch128_256_load(p+1248),ch128_256_bc(k->ph+9)),1);
          a=ch128_256_accum(a,ch128_256_xor(ch128_256_load(p+1376),ch128_256_bc(k->ph+10)),ch128_256_xor(ch128_256_load(p+1504),ch128_256_bc(k->ph+11)),1);
          a=ch128_256_accum(a,ch128_256_xor(ch128_256_load(p+1632),ch128_256_bc(k->ph+12)),ch128_256_xor(ch128_256_load(p+1760),ch128_256_bc(k->ph+13)),1);
          a=ch128_256_accum(a,ch128_256_xor(ch128_256_load(p+1888),ch128_256_bc(k->ph+14)),ch128_256_xor(ch128_256_load(p+2016),ch128_256_bc(k->ph+15)),1);
#if CHAINHASH128_BLOCK_BYTES == 512
          a=ch128_256_accum(a,ch128_256_xor(ch128_256_load(p+2144),ch128_256_bc(k->ph+16)),ch128_256_xor(ch128_256_load(p+2272),ch128_256_bc(k->ph+17)),1);
          a=ch128_256_accum(a,ch128_256_xor(ch128_256_load(p+2400),ch128_256_bc(k->ph+18)),ch128_256_xor(ch128_256_load(p+2528),ch128_256_bc(k->ph+19)),1);
          a=ch128_256_accum(a,ch128_256_xor(ch128_256_load(p+2656),ch128_256_bc(k->ph+20)),ch128_256_xor(ch128_256_load(p+2784),ch128_256_bc(k->ph+21)),1);
          a=ch128_256_accum(a,ch128_256_xor(ch128_256_load(p+2912),ch128_256_bc(k->ph+22)),ch128_256_xor(ch128_256_load(p+3040),ch128_256_bc(k->ph+23)),1);
          a=ch128_256_accum(a,ch128_256_xor(ch128_256_load(p+3168),ch128_256_bc(k->ph+24)),ch128_256_xor(ch128_256_load(p+3296),ch128_256_bc(k->ph+25)),1);
          a=ch128_256_accum(a,ch128_256_xor(ch128_256_load(p+3424),ch128_256_bc(k->ph+26)),ch128_256_xor(ch128_256_load(p+3552),ch128_256_bc(k->ph+27)),1);
          a=ch128_256_accum(a,ch128_256_xor(ch128_256_load(p+3680),ch128_256_bc(k->ph+28)),ch128_256_xor(ch128_256_load(p+3808),ch128_256_bc(k->ph+29)),1);
          a=ch128_256_accum(a,ch128_256_xor(ch128_256_load(p+3936),ch128_256_bc(k->ph+30)),ch128_256_xor(ch128_256_load(p+4064),ch128_256_bc(k->ph+31)),1);
#endif
          a=ch128_256_accum(a,s3.lo,y,1); a=ch128_256_accum(a,s3.hi,h,1); s3=ch128_256_pack(a,1); }
        p+=CH128_REGION;
    } while(--regions);
    { ch128_raw acc={{0,0},{0,0}}; ch128_word lo[8],hi[8]; unsigned j;
      ch128_256_store(lo+0,s0.lo); ch128_256_store(hi+0,s0.hi);
      ch128_256_store(lo+2,s1.lo); ch128_256_store(hi+2,s1.hi);
      ch128_256_store(lo+4,s2.lo); ch128_256_store(hi+4,s2.hi);
      ch128_256_store(lo+6,s3.lo); ch128_256_store(hi+6,s3.hi);
      if(st) { for(j=0;j<8;j++) { st[j].lo=lo[j]; st[j].hi=hi[j]; } return ch128_make(0,0); }
      for(j=0;j<8;j++) { acc=ch128_rxor(acc,ch128_prod(lo[j],k->yp[7-j],1,1)); acc=ch128_rxor(acc,ch128_prod(hi[j],k->yh[7-j],1,1)); }
      return ch128_reduce(acc); }
}
CH128_T256 static ch128_word ch128_256_bulk0(const chainhash128_key *k,const uint8_t *p,size_t regions,size_t len,ch128_raw *st) { return ch128_256_bulk0_run(k,p,regions,len,st,CH_PF_OFF,64,0); }
CH128_T256 static ch128_word ch128_256_bulk1(const chainhash128_key *k,const uint8_t *p,size_t regions,size_t len,ch128_raw *st) { return ch128_256_bulk1_run(k,p,regions,len,st,CH_PF_OFF,64,0); }
CH128_T256 static ch128_word ch128_256_pf(const chainhash128_key *k,const uint8_t *p,size_t regions,size_t len,ch128_raw *st,int hint,unsigned step,size_t dist) { CH128_PF_SWITCH(ch128_256_bulk0,k,p,regions,len,st) }
#endif

#ifdef CH128_X86
CH128_T512 static void ch128_512_region(const chainhash128_key *k,const uint8_t *p,ch128_raw out[8],int school) {
    unsigned j,c; for(j=0;j<8;j+=4) { ch128_512_acc a=ch128_512_azero(); ch128_512_raw r;
        for(c=0;c<CH128_CHUNKS;c++) a=ch128_512_accum(a,ch128_512_xor(ch128_512_load(p+256*c+16*j),ch128_512_bc(k->ph+2*c)),ch128_512_xor(ch128_512_load(p+256*c+16*j+128),ch128_512_bc(k->ph+2*c+1)),school);
        r=ch128_512_pack(a,school); { ch128_word lo[4],hi[4]; unsigned t; ch128_512_store(lo,r.lo); ch128_512_store(hi,r.hi); for(t=0;t<4;t++) { out[j+t].lo=lo[t]; out[j+t].hi=hi[t]; } }
    }
}
CH128_T512 CH128_INLINE ch128_word ch128_512_bulk0_run(const chainhash128_key *k,const uint8_t *p,size_t regions,size_t len,ch128_raw *st,const int hint,const unsigned step,size_t dist) {
    const __m512i y=ch128_512_bc(k->yp+4),h=ch128_512_bc(k->yh+4);
    ch128_512_raw state; ch128_word init[4]={{0,0}}; init[3].lo=len; state.lo=ch128_512_load(init);state.hi=ch128_512_zero();
    if(st) state=ch128_512_in(st);
    /* The 32 chunk keys, broadcast once per call and read back with plain loads (on Zen 4 a
     * ZMM broadcast from memory also takes a shuffle slot). The clobber keeps the compiler
     * from folding the table back into broadcasts; each pass launders the table pointer so
     * the keys are not hoisted out of the loop and spilled a second time. */
    __m512i kz[CH128_W]; unsigned c;
    for(c=0;c<CH128_W;c++) kz[c]=ch128_512_bc(k->ph+c);
    __asm__ volatile("" : : "r"(kz) : "memory");
    do {
        CH128_PREFETCH(p,regions,CH128_REGION,hint,step,dist);
      { ch128_512_acc a=ch128_512_azero(); const __m512i *kt=kz; __asm__ volatile("" : "+r"(kt));
        a=ch128_512_accum2(a,ch128_512_xor(ch128_512_load(p+0),kt[0]),ch128_512_xor(ch128_512_load(p+128),kt[1]),ch128_512_xor(ch128_512_load(p+256),kt[2]),ch128_512_xor(ch128_512_load(p+384),kt[3]),0);
        a=ch128_512_accum2(a,ch128_512_xor(ch128_512_load(p+512),kt[4]),ch128_512_xor(ch128_512_load(p+640),kt[5]),ch128_512_xor(ch128_512_load(p+768),kt[6]),ch128_512_xor(ch128_512_load(p+896),kt[7]),0);
        a=ch128_512_accum2(a,ch128_512_xor(ch128_512_load(p+1024),kt[8]),ch128_512_xor(ch128_512_load(p+1152),kt[9]),ch128_512_xor(ch128_512_load(p+1280),kt[10]),ch128_512_xor(ch128_512_load(p+1408),kt[11]),0);
        a=ch128_512_accum2(a,ch128_512_xor(ch128_512_load(p+1536),kt[12]),ch128_512_xor(ch128_512_load(p+1664),kt[13]),ch128_512_xor(ch128_512_load(p+1792),kt[14]),ch128_512_xor(ch128_512_load(p+1920),kt[15]),0);
#if CHAINHASH128_BLOCK_BYTES == 512
        a=ch128_512_accum2(a,ch128_512_xor(ch128_512_load(p+2048),kt[16]),ch128_512_xor(ch128_512_load(p+2176),kt[17]),ch128_512_xor(ch128_512_load(p+2304),kt[18]),ch128_512_xor(ch128_512_load(p+2432),kt[19]),0);
        a=ch128_512_accum2(a,ch128_512_xor(ch128_512_load(p+2560),kt[20]),ch128_512_xor(ch128_512_load(p+2688),kt[21]),ch128_512_xor(ch128_512_load(p+2816),kt[22]),ch128_512_xor(ch128_512_load(p+2944),kt[23]),0);
        a=ch128_512_accum2(a,ch128_512_xor(ch128_512_load(p+3072),kt[24]),ch128_512_xor(ch128_512_load(p+3200),kt[25]),ch128_512_xor(ch128_512_load(p+3328),kt[26]),ch128_512_xor(ch128_512_load(p+3456),kt[27]),0);
        a=ch128_512_accum2(a,ch128_512_xor(ch128_512_load(p+3584),kt[28]),ch128_512_xor(ch128_512_load(p+3712),kt[29]),ch128_512_xor(ch128_512_load(p+3840),kt[30]),ch128_512_xor(ch128_512_load(p+3968),kt[31]),0);
#endif
        a=ch128_512_accum2(a,state.lo,y,state.hi,h,0);state=ch128_512_pack(a,0); }
      { ch128_512_acc a=ch128_512_azero(); const __m512i *kt=kz; __asm__ volatile("" : "+r"(kt));
        a=ch128_512_accum2(a,ch128_512_xor(ch128_512_load(p+64),kt[0]),ch128_512_xor(ch128_512_load(p+192),kt[1]),ch128_512_xor(ch128_512_load(p+320),kt[2]),ch128_512_xor(ch128_512_load(p+448),kt[3]),0);
        a=ch128_512_accum2(a,ch128_512_xor(ch128_512_load(p+576),kt[4]),ch128_512_xor(ch128_512_load(p+704),kt[5]),ch128_512_xor(ch128_512_load(p+832),kt[6]),ch128_512_xor(ch128_512_load(p+960),kt[7]),0);
        a=ch128_512_accum2(a,ch128_512_xor(ch128_512_load(p+1088),kt[8]),ch128_512_xor(ch128_512_load(p+1216),kt[9]),ch128_512_xor(ch128_512_load(p+1344),kt[10]),ch128_512_xor(ch128_512_load(p+1472),kt[11]),0);
        a=ch128_512_accum2(a,ch128_512_xor(ch128_512_load(p+1600),kt[12]),ch128_512_xor(ch128_512_load(p+1728),kt[13]),ch128_512_xor(ch128_512_load(p+1856),kt[14]),ch128_512_xor(ch128_512_load(p+1984),kt[15]),0);
#if CHAINHASH128_BLOCK_BYTES == 512
        a=ch128_512_accum2(a,ch128_512_xor(ch128_512_load(p+2112),kt[16]),ch128_512_xor(ch128_512_load(p+2240),kt[17]),ch128_512_xor(ch128_512_load(p+2368),kt[18]),ch128_512_xor(ch128_512_load(p+2496),kt[19]),0);
        a=ch128_512_accum2(a,ch128_512_xor(ch128_512_load(p+2624),kt[20]),ch128_512_xor(ch128_512_load(p+2752),kt[21]),ch128_512_xor(ch128_512_load(p+2880),kt[22]),ch128_512_xor(ch128_512_load(p+3008),kt[23]),0);
        a=ch128_512_accum2(a,ch128_512_xor(ch128_512_load(p+3136),kt[24]),ch128_512_xor(ch128_512_load(p+3264),kt[25]),ch128_512_xor(ch128_512_load(p+3392),kt[26]),ch128_512_xor(ch128_512_load(p+3520),kt[27]),0);
        a=ch128_512_accum2(a,ch128_512_xor(ch128_512_load(p+3648),kt[28]),ch128_512_xor(ch128_512_load(p+3776),kt[29]),ch128_512_xor(ch128_512_load(p+3904),kt[30]),ch128_512_xor(ch128_512_load(p+4032),kt[31]),0);
#endif
        a=ch128_512_accum2(a,state.lo,y,state.hi,h,0);state=ch128_512_pack(a,0); }
      p+=CH128_REGION;
    } while(--regions);
    { ch128_raw acc={{0,0},{0,0}}; ch128_word lo[4],hi[4]; unsigned j;
      ch128_512_store(lo,state.lo);ch128_512_store(hi,state.hi);
      if(st) { for(j=0;j<4;j++) { st[j].lo=lo[j]; st[j].hi=hi[j]; } return ch128_make(0,0); }
      for(j=0;j<4;j++) {acc=ch128_rxor(acc,ch128_prod(lo[j],k->yp[3-j],1,0));acc=ch128_rxor(acc,ch128_prod(hi[j],k->yh[3-j],1,0));}return ch128_reduce(acc);
    }
}
CH128_T512 CH128_INLINE ch128_word ch128_512_bulk1_run(const chainhash128_key *k,const uint8_t *p,size_t regions,size_t len,ch128_raw *st,const int hint,const unsigned step,size_t dist) {
    const __m512i y=ch128_512_bc(k->yp+4),h=ch128_512_bc(k->yh+4);
    ch128_512_raw state; ch128_word init[4]={{0,0}}; init[3].lo=len; state.lo=ch128_512_load(init);state.hi=ch128_512_zero();
    if(st) state=ch128_512_in(st);
    /* The 32 chunk keys, broadcast once per call and read back with plain loads (on Zen 4 a
     * ZMM broadcast from memory also takes a shuffle slot). The clobber keeps the compiler
     * from folding the table back into broadcasts; each pass launders the table pointer so
     * the keys are not hoisted out of the loop and spilled a second time. */
    __m512i kz[CH128_W]; unsigned c;
    for(c=0;c<CH128_W;c++) kz[c]=ch128_512_bc(k->ph+c);
    __asm__ volatile("" : : "r"(kz) : "memory");
    do {
        CH128_PREFETCH(p,regions,CH128_REGION,hint,step,dist);
      { ch128_512_acc a=ch128_512_azero(); const __m512i *kt=kz; __asm__ volatile("" : "+r"(kt));
        a=ch128_512_accum2(a,ch128_512_xor(ch128_512_load(p+0),kt[0]),ch128_512_xor(ch128_512_load(p+128),kt[1]),ch128_512_xor(ch128_512_load(p+256),kt[2]),ch128_512_xor(ch128_512_load(p+384),kt[3]),1);
        a=ch128_512_accum2(a,ch128_512_xor(ch128_512_load(p+512),kt[4]),ch128_512_xor(ch128_512_load(p+640),kt[5]),ch128_512_xor(ch128_512_load(p+768),kt[6]),ch128_512_xor(ch128_512_load(p+896),kt[7]),1);
        a=ch128_512_accum2(a,ch128_512_xor(ch128_512_load(p+1024),kt[8]),ch128_512_xor(ch128_512_load(p+1152),kt[9]),ch128_512_xor(ch128_512_load(p+1280),kt[10]),ch128_512_xor(ch128_512_load(p+1408),kt[11]),1);
        a=ch128_512_accum2(a,ch128_512_xor(ch128_512_load(p+1536),kt[12]),ch128_512_xor(ch128_512_load(p+1664),kt[13]),ch128_512_xor(ch128_512_load(p+1792),kt[14]),ch128_512_xor(ch128_512_load(p+1920),kt[15]),1);
#if CHAINHASH128_BLOCK_BYTES == 512
        a=ch128_512_accum2(a,ch128_512_xor(ch128_512_load(p+2048),kt[16]),ch128_512_xor(ch128_512_load(p+2176),kt[17]),ch128_512_xor(ch128_512_load(p+2304),kt[18]),ch128_512_xor(ch128_512_load(p+2432),kt[19]),1);
        a=ch128_512_accum2(a,ch128_512_xor(ch128_512_load(p+2560),kt[20]),ch128_512_xor(ch128_512_load(p+2688),kt[21]),ch128_512_xor(ch128_512_load(p+2816),kt[22]),ch128_512_xor(ch128_512_load(p+2944),kt[23]),1);
        a=ch128_512_accum2(a,ch128_512_xor(ch128_512_load(p+3072),kt[24]),ch128_512_xor(ch128_512_load(p+3200),kt[25]),ch128_512_xor(ch128_512_load(p+3328),kt[26]),ch128_512_xor(ch128_512_load(p+3456),kt[27]),1);
        a=ch128_512_accum2(a,ch128_512_xor(ch128_512_load(p+3584),kt[28]),ch128_512_xor(ch128_512_load(p+3712),kt[29]),ch128_512_xor(ch128_512_load(p+3840),kt[30]),ch128_512_xor(ch128_512_load(p+3968),kt[31]),1);
#endif
        a=ch128_512_accum2(a,state.lo,y,state.hi,h,1);state=ch128_512_pack(a,1); }
      { ch128_512_acc a=ch128_512_azero(); const __m512i *kt=kz; __asm__ volatile("" : "+r"(kt));
        a=ch128_512_accum2(a,ch128_512_xor(ch128_512_load(p+64),kt[0]),ch128_512_xor(ch128_512_load(p+192),kt[1]),ch128_512_xor(ch128_512_load(p+320),kt[2]),ch128_512_xor(ch128_512_load(p+448),kt[3]),1);
        a=ch128_512_accum2(a,ch128_512_xor(ch128_512_load(p+576),kt[4]),ch128_512_xor(ch128_512_load(p+704),kt[5]),ch128_512_xor(ch128_512_load(p+832),kt[6]),ch128_512_xor(ch128_512_load(p+960),kt[7]),1);
        a=ch128_512_accum2(a,ch128_512_xor(ch128_512_load(p+1088),kt[8]),ch128_512_xor(ch128_512_load(p+1216),kt[9]),ch128_512_xor(ch128_512_load(p+1344),kt[10]),ch128_512_xor(ch128_512_load(p+1472),kt[11]),1);
        a=ch128_512_accum2(a,ch128_512_xor(ch128_512_load(p+1600),kt[12]),ch128_512_xor(ch128_512_load(p+1728),kt[13]),ch128_512_xor(ch128_512_load(p+1856),kt[14]),ch128_512_xor(ch128_512_load(p+1984),kt[15]),1);
#if CHAINHASH128_BLOCK_BYTES == 512
        a=ch128_512_accum2(a,ch128_512_xor(ch128_512_load(p+2112),kt[16]),ch128_512_xor(ch128_512_load(p+2240),kt[17]),ch128_512_xor(ch128_512_load(p+2368),kt[18]),ch128_512_xor(ch128_512_load(p+2496),kt[19]),1);
        a=ch128_512_accum2(a,ch128_512_xor(ch128_512_load(p+2624),kt[20]),ch128_512_xor(ch128_512_load(p+2752),kt[21]),ch128_512_xor(ch128_512_load(p+2880),kt[22]),ch128_512_xor(ch128_512_load(p+3008),kt[23]),1);
        a=ch128_512_accum2(a,ch128_512_xor(ch128_512_load(p+3136),kt[24]),ch128_512_xor(ch128_512_load(p+3264),kt[25]),ch128_512_xor(ch128_512_load(p+3392),kt[26]),ch128_512_xor(ch128_512_load(p+3520),kt[27]),1);
        a=ch128_512_accum2(a,ch128_512_xor(ch128_512_load(p+3648),kt[28]),ch128_512_xor(ch128_512_load(p+3776),kt[29]),ch128_512_xor(ch128_512_load(p+3904),kt[30]),ch128_512_xor(ch128_512_load(p+4032),kt[31]),1);
#endif
        a=ch128_512_accum2(a,state.lo,y,state.hi,h,1);state=ch128_512_pack(a,1); }
      p+=CH128_REGION;
    } while(--regions);
    { ch128_raw acc={{0,0},{0,0}}; ch128_word lo[4],hi[4]; unsigned j;
      ch128_512_store(lo,state.lo);ch128_512_store(hi,state.hi);
      if(st) { for(j=0;j<4;j++) { st[j].lo=lo[j]; st[j].hi=hi[j]; } return ch128_make(0,0); }
      for(j=0;j<4;j++) {acc=ch128_rxor(acc,ch128_prod(lo[j],k->yp[3-j],1,1));acc=ch128_rxor(acc,ch128_prod(hi[j],k->yh[3-j],1,1));}return ch128_reduce(acc);
    }
}
CH128_T512 static ch128_word ch128_512_bulk0(const chainhash128_key *k,const uint8_t *p,size_t regions,size_t len,ch128_raw *st) { return ch128_512_bulk0_run(k,p,regions,len,st,CH_PF_OFF,64,0); }
CH128_T512 static ch128_word ch128_512_bulk1(const chainhash128_key *k,const uint8_t *p,size_t regions,size_t len,ch128_raw *st) { return ch128_512_bulk1_run(k,p,regions,len,st,CH_PF_OFF,64,0); }
CH128_T512 static ch128_word ch128_512_pf(const chainhash128_key *k,const uint8_t *p,size_t regions,size_t len,ch128_raw *st,int hint,unsigned step,size_t dist) { CH128_PF_SWITCH(ch128_512_bulk1,k,p,regions,len,st) }
#endif

#ifdef CH128_ARM
CH128_NBEGIN
static void ch128_n_region(const chainhash128_key *k,const uint8_t *p,ch128_raw out[8],int school) {
    unsigned j,c; for(j=0;j<8;j+=1) { ch128_n_acc a=ch128_n_azero(); ch128_n_raw r;
        for(c=0;c<CH128_CHUNKS;c++) a=ch128_n_accum(a,ch128_n_xor(ch128_n_load(p+256*c+16*j),ch128_n_bc(k->ph+2*c)),ch128_n_xor(ch128_n_load(p+256*c+16*j+128),ch128_n_bc(k->ph+2*c+1)),school);
        r=ch128_n_pack(a,school); { ch128_word lo[1],hi[1]; unsigned t; ch128_n_store(lo,r.lo); ch128_n_store(hi,r.hi); for(t=0;t<1;t++) { out[j+t].lo=lo[t]; out[j+t].hi=hi[t]; } }
    }
}
/* Fixed-register PMULL instruction templates. STEP consumes four comb
 * pairs and advances the two half pointers by one 256-byte chunk. FOLD
 * advances the four raw states by y^4. Expanded instructions are audited
 * in audit/; these macros introduce no calls or runtime choices. */
#if CH128_SHA3 == 1
#define CH128_NX3(D,A,B,C) "eor3 v" #D ".16b,v" #A ".16b,v" #B ".16b,v" #C ".16b\n\t"
#else
#define CH128_NX3(D,A,B,C) "eor v" #D ".16b,v" #A ".16b,v" #B ".16b\n\t" \
                              "eor v" #D ".16b,v" #D ".16b,v" #C ".16b\n\t"
#endif
#define CH128_NPH0 \
      "ld1 {v28.2d,v29.2d},[x11],#32\n\t" \
      "ld1 {v20.2d,v21.2d,v22.2d,v23.2d},[x9]\n\t" \
      "ld1 {v24.2d,v25.2d,v26.2d,v27.2d},[x10]\n\t" \
      "eor v20.16b,v20.16b,v28.16b\n\t" \
      "eor v24.16b,v24.16b,v29.16b\n\t" \
      "pmull v30.1q,v20.1d,v24.1d\n\t" \
      "eor v8.16b,v8.16b,v30.16b\n\t" \
      "pmull2 v30.1q,v20.2d,v24.2d\n\t" \
      "eor v9.16b,v9.16b,v30.16b\n\t" \
      "ext v30.16b,v20.16b,v20.16b,#8\n\t" \
      "ext v31.16b,v24.16b,v24.16b,#8\n\t" \
      "eor v30.16b,v30.16b,v20.16b\n\t" \
      "eor v31.16b,v31.16b,v24.16b\n\t" \
      "pmull v30.1q,v30.1d,v31.1d\n\t" \
      "eor v10.16b,v10.16b,v30.16b\n\t" \
      "eor v21.16b,v21.16b,v28.16b\n\t" \
      "eor v25.16b,v25.16b,v29.16b\n\t" \
      "pmull v30.1q,v21.1d,v25.1d\n\t" \
      "eor v11.16b,v11.16b,v30.16b\n\t" \
      "pmull2 v30.1q,v21.2d,v25.2d\n\t" \
      "eor v12.16b,v12.16b,v30.16b\n\t" \
      "ext v30.16b,v21.16b,v21.16b,#8\n\t" \
      "ext v31.16b,v25.16b,v25.16b,#8\n\t" \
      "eor v30.16b,v30.16b,v21.16b\n\t" \
      "eor v31.16b,v31.16b,v25.16b\n\t" \
      "pmull v30.1q,v30.1d,v31.1d\n\t" \
      "eor v13.16b,v13.16b,v30.16b\n\t" \
      "eor v22.16b,v22.16b,v28.16b\n\t" \
      "eor v26.16b,v26.16b,v29.16b\n\t" \
      "pmull v30.1q,v22.1d,v26.1d\n\t" \
      "eor v14.16b,v14.16b,v30.16b\n\t" \
      "pmull2 v30.1q,v22.2d,v26.2d\n\t" \
      "eor v15.16b,v15.16b,v30.16b\n\t" \
      "ext v30.16b,v22.16b,v22.16b,#8\n\t" \
      "ext v31.16b,v26.16b,v26.16b,#8\n\t" \
      "eor v30.16b,v30.16b,v22.16b\n\t" \
      "eor v31.16b,v31.16b,v26.16b\n\t" \
      "pmull v30.1q,v30.1d,v31.1d\n\t" \
      "eor v16.16b,v16.16b,v30.16b\n\t" \
      "eor v23.16b,v23.16b,v28.16b\n\t" \
      "eor v27.16b,v27.16b,v29.16b\n\t" \
      "pmull v30.1q,v23.1d,v27.1d\n\t" \
      "eor v17.16b,v17.16b,v30.16b\n\t" \
      "pmull2 v30.1q,v23.2d,v27.2d\n\t" \
      "eor v18.16b,v18.16b,v30.16b\n\t" \
      "ext v30.16b,v23.16b,v23.16b,#8\n\t" \
      "ext v31.16b,v27.16b,v27.16b,#8\n\t" \
      "eor v30.16b,v30.16b,v23.16b\n\t" \
      "eor v31.16b,v31.16b,v27.16b\n\t" \
      "pmull v30.1q,v30.1d,v31.1d\n\t" \
      "eor v19.16b,v19.16b,v30.16b\n\t"
#define CH128_NSTEP0 \
      CH128_NPH0 \
      "add x9,x9,#256\n\t" \
      "add x10,x10,#256\n\t"
#define CH128_NFOLD0 \
      "ld1 {v28.2d},[%[yp]]\n\t" \
      "ld1 {v29.2d},[%[yh]]\n\t" \
      "pmull v30.1q,v0.1d,v28.1d\n\t" \
      "eor v8.16b,v8.16b,v30.16b\n\t" \
      "pmull2 v30.1q,v0.2d,v28.2d\n\t" \
      "eor v9.16b,v9.16b,v30.16b\n\t" \
      "ext v30.16b,v0.16b,v0.16b,#8\n\t" \
      "ext v31.16b,v28.16b,v28.16b,#8\n\t" \
      "eor v30.16b,v30.16b,v0.16b\n\t" \
      "eor v31.16b,v31.16b,v28.16b\n\t" \
      "pmull v30.1q,v30.1d,v31.1d\n\t" \
      "eor v10.16b,v10.16b,v30.16b\n\t" \
      "pmull v30.1q,v1.1d,v29.1d\n\t" \
      "eor v8.16b,v8.16b,v30.16b\n\t" \
      "pmull2 v30.1q,v1.2d,v29.2d\n\t" \
      "eor v9.16b,v9.16b,v30.16b\n\t" \
      "ext v30.16b,v1.16b,v1.16b,#8\n\t" \
      "ext v31.16b,v29.16b,v29.16b,#8\n\t" \
      "eor v30.16b,v30.16b,v1.16b\n\t" \
      "eor v31.16b,v31.16b,v29.16b\n\t" \
      "pmull v30.1q,v30.1d,v31.1d\n\t" \
      "eor v10.16b,v10.16b,v30.16b\n\t" \
      CH128_NX3(10,10,8,9) \
      "movi v31.2d,#0\n\t" \
      "ext v30.16b,v31.16b,v10.16b,#8\n\t" \
      "eor v0.16b,v8.16b,v30.16b\n\t" \
      "ext v30.16b,v10.16b,v31.16b,#8\n\t" \
      "eor v1.16b,v9.16b,v30.16b\n\t" \
      "pmull v30.1q,v2.1d,v28.1d\n\t" \
      "eor v11.16b,v11.16b,v30.16b\n\t" \
      "pmull2 v30.1q,v2.2d,v28.2d\n\t" \
      "eor v12.16b,v12.16b,v30.16b\n\t" \
      "ext v30.16b,v2.16b,v2.16b,#8\n\t" \
      "ext v31.16b,v28.16b,v28.16b,#8\n\t" \
      "eor v30.16b,v30.16b,v2.16b\n\t" \
      "eor v31.16b,v31.16b,v28.16b\n\t" \
      "pmull v30.1q,v30.1d,v31.1d\n\t" \
      "eor v13.16b,v13.16b,v30.16b\n\t" \
      "pmull v30.1q,v3.1d,v29.1d\n\t" \
      "eor v11.16b,v11.16b,v30.16b\n\t" \
      "pmull2 v30.1q,v3.2d,v29.2d\n\t" \
      "eor v12.16b,v12.16b,v30.16b\n\t" \
      "ext v30.16b,v3.16b,v3.16b,#8\n\t" \
      "ext v31.16b,v29.16b,v29.16b,#8\n\t" \
      "eor v30.16b,v30.16b,v3.16b\n\t" \
      "eor v31.16b,v31.16b,v29.16b\n\t" \
      "pmull v30.1q,v30.1d,v31.1d\n\t" \
      "eor v13.16b,v13.16b,v30.16b\n\t" \
      CH128_NX3(13,13,11,12) \
      "movi v31.2d,#0\n\t" \
      "ext v30.16b,v31.16b,v13.16b,#8\n\t" \
      "eor v2.16b,v11.16b,v30.16b\n\t" \
      "ext v30.16b,v13.16b,v31.16b,#8\n\t" \
      "eor v3.16b,v12.16b,v30.16b\n\t" \
      "pmull v30.1q,v4.1d,v28.1d\n\t" \
      "eor v14.16b,v14.16b,v30.16b\n\t" \
      "pmull2 v30.1q,v4.2d,v28.2d\n\t" \
      "eor v15.16b,v15.16b,v30.16b\n\t" \
      "ext v30.16b,v4.16b,v4.16b,#8\n\t" \
      "ext v31.16b,v28.16b,v28.16b,#8\n\t" \
      "eor v30.16b,v30.16b,v4.16b\n\t" \
      "eor v31.16b,v31.16b,v28.16b\n\t" \
      "pmull v30.1q,v30.1d,v31.1d\n\t" \
      "eor v16.16b,v16.16b,v30.16b\n\t" \
      "pmull v30.1q,v5.1d,v29.1d\n\t" \
      "eor v14.16b,v14.16b,v30.16b\n\t" \
      "pmull2 v30.1q,v5.2d,v29.2d\n\t" \
      "eor v15.16b,v15.16b,v30.16b\n\t" \
      "ext v30.16b,v5.16b,v5.16b,#8\n\t" \
      "ext v31.16b,v29.16b,v29.16b,#8\n\t" \
      "eor v30.16b,v30.16b,v5.16b\n\t" \
      "eor v31.16b,v31.16b,v29.16b\n\t" \
      "pmull v30.1q,v30.1d,v31.1d\n\t" \
      "eor v16.16b,v16.16b,v30.16b\n\t" \
      CH128_NX3(16,16,14,15) \
      "movi v31.2d,#0\n\t" \
      "ext v30.16b,v31.16b,v16.16b,#8\n\t" \
      "eor v4.16b,v14.16b,v30.16b\n\t" \
      "ext v30.16b,v16.16b,v31.16b,#8\n\t" \
      "eor v5.16b,v15.16b,v30.16b\n\t" \
      "pmull v30.1q,v6.1d,v28.1d\n\t" \
      "eor v17.16b,v17.16b,v30.16b\n\t" \
      "pmull2 v30.1q,v6.2d,v28.2d\n\t" \
      "eor v18.16b,v18.16b,v30.16b\n\t" \
      "ext v30.16b,v6.16b,v6.16b,#8\n\t" \
      "ext v31.16b,v28.16b,v28.16b,#8\n\t" \
      "eor v30.16b,v30.16b,v6.16b\n\t" \
      "eor v31.16b,v31.16b,v28.16b\n\t" \
      "pmull v30.1q,v30.1d,v31.1d\n\t" \
      "eor v19.16b,v19.16b,v30.16b\n\t" \
      "pmull v30.1q,v7.1d,v29.1d\n\t" \
      "eor v17.16b,v17.16b,v30.16b\n\t" \
      "pmull2 v30.1q,v7.2d,v29.2d\n\t" \
      "eor v18.16b,v18.16b,v30.16b\n\t" \
      "ext v30.16b,v7.16b,v7.16b,#8\n\t" \
      "ext v31.16b,v29.16b,v29.16b,#8\n\t" \
      "eor v30.16b,v30.16b,v7.16b\n\t" \
      "eor v31.16b,v31.16b,v29.16b\n\t" \
      "pmull v30.1q,v30.1d,v31.1d\n\t" \
      "eor v19.16b,v19.16b,v30.16b\n\t" \
      CH128_NX3(19,19,17,18) \
      "movi v31.2d,#0\n\t" \
      "ext v30.16b,v31.16b,v19.16b,#8\n\t" \
      "eor v6.16b,v17.16b,v30.16b\n\t" \
      "ext v30.16b,v19.16b,v31.16b,#8\n\t" \
      "eor v7.16b,v18.16b,v30.16b\n\t"
#define CH128_NPH1 \
      "ld1 {v28.2d,v29.2d},[x11],#32\n\t" \
      "ld1 {v20.2d,v21.2d,v22.2d,v23.2d},[x9]\n\t" \
      "ld1 {v24.2d,v25.2d,v26.2d,v27.2d},[x10]\n\t" \
      "eor v20.16b,v20.16b,v28.16b\n\t" \
      "eor v24.16b,v24.16b,v29.16b\n\t" \
      "pmull v30.1q,v20.1d,v24.1d\n\t" \
      "eor v8.16b,v8.16b,v30.16b\n\t" \
      "pmull2 v30.1q,v20.2d,v24.2d\n\t" \
      "eor v9.16b,v9.16b,v30.16b\n\t" \
      "ext v31.16b,v24.16b,v24.16b,#8\n\t" \
      "pmull v30.1q,v20.1d,v31.1d\n\t" \
      "pmull2 v31.1q,v20.2d,v31.2d\n\t" \
      CH128_NX3(10,10,30,31) \
      "eor v21.16b,v21.16b,v28.16b\n\t" \
      "eor v25.16b,v25.16b,v29.16b\n\t" \
      "pmull v30.1q,v21.1d,v25.1d\n\t" \
      "eor v11.16b,v11.16b,v30.16b\n\t" \
      "pmull2 v30.1q,v21.2d,v25.2d\n\t" \
      "eor v12.16b,v12.16b,v30.16b\n\t" \
      "ext v31.16b,v25.16b,v25.16b,#8\n\t" \
      "pmull v30.1q,v21.1d,v31.1d\n\t" \
      "pmull2 v31.1q,v21.2d,v31.2d\n\t" \
      CH128_NX3(13,13,30,31) \
      "eor v22.16b,v22.16b,v28.16b\n\t" \
      "eor v26.16b,v26.16b,v29.16b\n\t" \
      "pmull v30.1q,v22.1d,v26.1d\n\t" \
      "eor v14.16b,v14.16b,v30.16b\n\t" \
      "pmull2 v30.1q,v22.2d,v26.2d\n\t" \
      "eor v15.16b,v15.16b,v30.16b\n\t" \
      "ext v31.16b,v26.16b,v26.16b,#8\n\t" \
      "pmull v30.1q,v22.1d,v31.1d\n\t" \
      "pmull2 v31.1q,v22.2d,v31.2d\n\t" \
      CH128_NX3(16,16,30,31) \
      "eor v23.16b,v23.16b,v28.16b\n\t" \
      "eor v27.16b,v27.16b,v29.16b\n\t" \
      "pmull v30.1q,v23.1d,v27.1d\n\t" \
      "eor v17.16b,v17.16b,v30.16b\n\t" \
      "pmull2 v30.1q,v23.2d,v27.2d\n\t" \
      "eor v18.16b,v18.16b,v30.16b\n\t" \
      "ext v31.16b,v27.16b,v27.16b,#8\n\t" \
      "pmull v30.1q,v23.1d,v31.1d\n\t" \
      "pmull2 v31.1q,v23.2d,v31.2d\n\t" \
      CH128_NX3(19,19,30,31)
#define CH128_NSTEP1 \
      CH128_NPH1 \
      "add x9,x9,#256\n\t" \
      "add x10,x10,#256\n\t"
#define CH128_NFOLD1 \
      "ld1 {v28.2d},[%[yp]]\n\t" \
      "ld1 {v29.2d},[%[yh]]\n\t" \
      "pmull v30.1q,v0.1d,v28.1d\n\t" \
      "eor v8.16b,v8.16b,v30.16b\n\t" \
      "pmull2 v30.1q,v0.2d,v28.2d\n\t" \
      "eor v9.16b,v9.16b,v30.16b\n\t" \
      "ext v31.16b,v28.16b,v28.16b,#8\n\t" \
      "pmull v30.1q,v0.1d,v31.1d\n\t" \
      "pmull2 v31.1q,v0.2d,v31.2d\n\t" \
      CH128_NX3(10,10,30,31) \
      "pmull v30.1q,v1.1d,v29.1d\n\t" \
      "eor v8.16b,v8.16b,v30.16b\n\t" \
      "pmull2 v30.1q,v1.2d,v29.2d\n\t" \
      "eor v9.16b,v9.16b,v30.16b\n\t" \
      "ext v31.16b,v29.16b,v29.16b,#8\n\t" \
      "pmull v30.1q,v1.1d,v31.1d\n\t" \
      "pmull2 v31.1q,v1.2d,v31.2d\n\t" \
      CH128_NX3(10,10,30,31) \
      "movi v31.2d,#0\n\t" \
      "ext v30.16b,v31.16b,v10.16b,#8\n\t" \
      "eor v0.16b,v8.16b,v30.16b\n\t" \
      "ext v30.16b,v10.16b,v31.16b,#8\n\t" \
      "eor v1.16b,v9.16b,v30.16b\n\t" \
      "pmull v30.1q,v2.1d,v28.1d\n\t" \
      "eor v11.16b,v11.16b,v30.16b\n\t" \
      "pmull2 v30.1q,v2.2d,v28.2d\n\t" \
      "eor v12.16b,v12.16b,v30.16b\n\t" \
      "ext v31.16b,v28.16b,v28.16b,#8\n\t" \
      "pmull v30.1q,v2.1d,v31.1d\n\t" \
      "pmull2 v31.1q,v2.2d,v31.2d\n\t" \
      CH128_NX3(13,13,30,31) \
      "pmull v30.1q,v3.1d,v29.1d\n\t" \
      "eor v11.16b,v11.16b,v30.16b\n\t" \
      "pmull2 v30.1q,v3.2d,v29.2d\n\t" \
      "eor v12.16b,v12.16b,v30.16b\n\t" \
      "ext v31.16b,v29.16b,v29.16b,#8\n\t" \
      "pmull v30.1q,v3.1d,v31.1d\n\t" \
      "pmull2 v31.1q,v3.2d,v31.2d\n\t" \
      CH128_NX3(13,13,30,31) \
      "movi v31.2d,#0\n\t" \
      "ext v30.16b,v31.16b,v13.16b,#8\n\t" \
      "eor v2.16b,v11.16b,v30.16b\n\t" \
      "ext v30.16b,v13.16b,v31.16b,#8\n\t" \
      "eor v3.16b,v12.16b,v30.16b\n\t" \
      "pmull v30.1q,v4.1d,v28.1d\n\t" \
      "eor v14.16b,v14.16b,v30.16b\n\t" \
      "pmull2 v30.1q,v4.2d,v28.2d\n\t" \
      "eor v15.16b,v15.16b,v30.16b\n\t" \
      "ext v31.16b,v28.16b,v28.16b,#8\n\t" \
      "pmull v30.1q,v4.1d,v31.1d\n\t" \
      "pmull2 v31.1q,v4.2d,v31.2d\n\t" \
      CH128_NX3(16,16,30,31) \
      "pmull v30.1q,v5.1d,v29.1d\n\t" \
      "eor v14.16b,v14.16b,v30.16b\n\t" \
      "pmull2 v30.1q,v5.2d,v29.2d\n\t" \
      "eor v15.16b,v15.16b,v30.16b\n\t" \
      "ext v31.16b,v29.16b,v29.16b,#8\n\t" \
      "pmull v30.1q,v5.1d,v31.1d\n\t" \
      "pmull2 v31.1q,v5.2d,v31.2d\n\t" \
      CH128_NX3(16,16,30,31) \
      "movi v31.2d,#0\n\t" \
      "ext v30.16b,v31.16b,v16.16b,#8\n\t" \
      "eor v4.16b,v14.16b,v30.16b\n\t" \
      "ext v30.16b,v16.16b,v31.16b,#8\n\t" \
      "eor v5.16b,v15.16b,v30.16b\n\t" \
      "pmull v30.1q,v6.1d,v28.1d\n\t" \
      "eor v17.16b,v17.16b,v30.16b\n\t" \
      "pmull2 v30.1q,v6.2d,v28.2d\n\t" \
      "eor v18.16b,v18.16b,v30.16b\n\t" \
      "ext v31.16b,v28.16b,v28.16b,#8\n\t" \
      "pmull v30.1q,v6.1d,v31.1d\n\t" \
      "pmull2 v31.1q,v6.2d,v31.2d\n\t" \
      CH128_NX3(19,19,30,31) \
      "pmull v30.1q,v7.1d,v29.1d\n\t" \
      "eor v17.16b,v17.16b,v30.16b\n\t" \
      "pmull2 v30.1q,v7.2d,v29.2d\n\t" \
      "eor v18.16b,v18.16b,v30.16b\n\t" \
      "ext v31.16b,v29.16b,v29.16b,#8\n\t" \
      "pmull v30.1q,v7.1d,v31.1d\n\t" \
      "pmull2 v31.1q,v7.2d,v31.2d\n\t" \
      CH128_NX3(19,19,30,31) \
      "movi v31.2d,#0\n\t" \
      "ext v30.16b,v31.16b,v19.16b,#8\n\t" \
      "eor v6.16b,v17.16b,v30.16b\n\t" \
      "ext v30.16b,v19.16b,v31.16b,#8\n\t" \
      "eor v7.16b,v18.16b,v30.16b\n\t"
/* Four fixed lazy chains. LD1 lists consume each comb half directly.
 * v0..v7: raw states; v8..v19: three-component block accumulators;
 * v20..v27: four word pairs; v28/v29: keys; v30/v31: scratch.
 * No hot-loop stack spills or vector-to-integer transfers are possible. */
static __attribute__((noinline)) ch128_word ch128_n_bulk0(const chainhash128_key *k,const uint8_t *p,size_t regions,size_t len,ch128_raw *st) {
    ch128_raw state[4],acc={{0,0},{0,0}};uint8_t *out=(uint8_t *)state;unsigned j;
    if(st) memcpy(state,st,sizeof(state)); else { memset(state,0,sizeof(state)); state[3].lo.lo=len; }
    __asm__ volatile(
      "add x9,%[out],#64\n\t"
      "ld1 {v0.2d,v1.2d,v2.2d,v3.2d},[%[out]]\n\t"
      "ld1 {v4.2d,v5.2d,v6.2d,v7.2d},[x9]\n\t"
      "1:\n\t"
      "movi v8.2d,#0\n\t"
      "movi v9.2d,#0\n\t"
      "movi v10.2d,#0\n\t"
      "movi v11.2d,#0\n\t"
      "movi v12.2d,#0\n\t"
      "movi v13.2d,#0\n\t"
      "movi v14.2d,#0\n\t"
      "movi v15.2d,#0\n\t"
      "movi v16.2d,#0\n\t"
      "movi v17.2d,#0\n\t"
      "movi v18.2d,#0\n\t"
      "movi v19.2d,#0\n\t"
      "add x9,%[p],#0\n\t"
      "add x10,x9,#128\n\t"
      "mov x11,%[key]\n\t"
      CH128_NSTEP0
      CH128_NSTEP0
      CH128_NSTEP0
      CH128_NSTEP0
      CH128_NSTEP0
      CH128_NSTEP0
      CH128_NSTEP0
      CH128_NSTEP0
#if CHAINHASH128_BLOCK_BYTES == 512
      CH128_NSTEP0
      CH128_NSTEP0
      CH128_NSTEP0
      CH128_NSTEP0
      CH128_NSTEP0
      CH128_NSTEP0
      CH128_NSTEP0
      CH128_NPH0
#endif
      CH128_NFOLD0
      "movi v8.2d,#0\n\t"
      "movi v9.2d,#0\n\t"
      "movi v10.2d,#0\n\t"
      "movi v11.2d,#0\n\t"
      "movi v12.2d,#0\n\t"
      "movi v13.2d,#0\n\t"
      "movi v14.2d,#0\n\t"
      "movi v15.2d,#0\n\t"
      "movi v16.2d,#0\n\t"
      "movi v17.2d,#0\n\t"
      "movi v18.2d,#0\n\t"
      "movi v19.2d,#0\n\t"
      "add x9,%[p],#64\n\t"
      "add x10,x9,#128\n\t"
      "mov x11,%[key]\n\t"
      CH128_NSTEP0
      CH128_NSTEP0
      CH128_NSTEP0
      CH128_NSTEP0
      CH128_NSTEP0
      CH128_NSTEP0
      CH128_NSTEP0
      CH128_NSTEP0
#if CHAINHASH128_BLOCK_BYTES == 512
      CH128_NSTEP0
      CH128_NSTEP0
      CH128_NSTEP0
      CH128_NSTEP0
      CH128_NSTEP0
      CH128_NSTEP0
      CH128_NSTEP0
      CH128_NPH0
#endif
      CH128_NFOLD0
      "add %[p],%[p],%[rb]\n\t"
      "subs %[count],%[count],#1\n\t"
      "b.ne 1b\n\t"
      "st1 {v0.2d,v1.2d,v2.2d,v3.2d},[%[out]],#64\n\t"
      "st1 {v4.2d,v5.2d,v6.2d,v7.2d},[%[out]]\n\t"
      : [p] "+&r"(p),[count] "+&r"(regions),[out] "+&r"(out)
      : [key] "r"(k->ph),[yp] "r"(k->yp+4),[yh] "r"(k->yh+4),[rb] "I"(CH128_REGION)
      : "x9","x10","x11","cc","memory","v0","v1","v2","v3","v4","v5","v6","v7","v8","v9","v10","v11","v12","v13","v14","v15","v16","v17","v18","v19","v20","v21","v22","v23","v24","v25","v26","v27","v28","v29","v30","v31");
    if(st) { memcpy(st,state,sizeof(state)); return ch128_make(0,0); }
    for(j=0;j<4;j++) {acc=ch128_rxor(acc,ch128_prod(state[j].lo,k->yp[3-j],4,0));acc=ch128_rxor(acc,ch128_prod(state[j].hi,k->yh[3-j],4,0));}return ch128_reduce(acc);
}
#if CHAINHASH_NEON_FUSE
/* Four fixed lazy chains, schoolbook, every product accumulated by a fused pair:
 * "pmull vS; eor vS,vS,vACC" (the EOR writes its PMULL's destination right after it,
 * which Apple cores issue as one operation; it needs no EOR3). Each accumulator
 * takes its products two at a time through the spare v30 (pmull v30; eor v30,v30,ACC;
 * pmull ACC; eor ACC,ACC,v30), so every macro instance ends with the same register
 * layout. The first chunk pair of a pass writes its accumulators (no zeroing), and
 * the lane's raw state times y^4 is accumulated last, so the state is needed only at
 * the end of a pass. XOR is associative and commutative: the raw states, and so the
 * digest, are bit-identical to the unfused kernels. Registers: v0-v7 states,
 * v8-v19 lane accumulators (ll,hh,mid), v20-v23 words, v24/v25 y^4 and X^128*y^4,
 * v26-v29 the two chunk key pairs, v30 spare, v31 zero. Only x9 (data) and x11 (key)
 * are advanced; loads use immediate offsets. */
#define CH128_NFL(D,A,B,C) "pmull v" #D ".1q,v" #A ".1d,v" #B ".1d\n\teor v" #D ".16b,v" #D ".16b,v" #C ".16b\n\t"
#define CH128_NFH(D,A,B,C) "pmull2 v" #D ".1q,v" #A ".2d,v" #B ".2d\n\teor v" #D ".16b,v" #D ".16b,v" #C ".16b\n\t"
#define CH128_NPL(D,A,B,C) "pmull v" #D ".1q,v" #A ".1d,v" #B ".1d\n\t"
#define CH128_NPH(D,A,B,C) "pmull2 v" #D ".1q,v" #A ".2d,v" #B ".2d\n\t"
#define CH128_NKEYS2 \
      "ldr q28,[x11,#0]\n\t" \
      "ldr q29,[x11,#16]\n\t" \
      "ldr q26,[x11,#32]\n\t" \
      "ldr q27,[x11,#48]\n\t"
/* Lane words of two chunks (A at x9+OaA/ObA, B at x9+OaB/ObB); FL/FH are CH128_NFL/NFH,
 * or CH128_NPL/NPH to start the accumulators on the first chunk pair of a pass. */
#define CH128_NPAIR2(FL,FH,OaA,ObA,OaB,ObB,LL,HH,MID) \
      "ldr q20,[x9,#" #OaA "]\n\t" \
      "ldr q21,[x9,#" #ObA "]\n\t" \
      "ldr q22,[x9,#" #OaB "]\n\t" \
      "ldr q23,[x9,#" #ObB "]\n\t" \
      "eor v20.16b,v20.16b,v28.16b\n\t" \
      "eor v21.16b,v21.16b,v29.16b\n\t" \
      "eor v22.16b,v22.16b,v26.16b\n\t" \
      "eor v23.16b,v23.16b,v27.16b\n\t" \
      FL(30,20,21,LL) CH128_NFL(LL,22,23,30) \
      FH(30,20,21,HH) CH128_NFH(HH,22,23,30) \
      "ext v21.16b,v21.16b,v21.16b,#8\n\t" \
      "ext v23.16b,v23.16b,v23.16b,#8\n\t" \
      FL(30,20,21,MID) CH128_NFH(MID,20,21,30) \
      CH128_NFL(30,22,23,MID) CH128_NFH(MID,22,23,30)
#define CH128_NPH1X2(FL,FH) \
      CH128_NKEYS2 \
      CH128_NPAIR2(FL,FH,0,128,256,384,8,9,10) \
      CH128_NPAIR2(FL,FH,16,144,272,400,11,12,13) \
      CH128_NPAIR2(FL,FH,32,160,288,416,14,15,16) \
      CH128_NPAIR2(FL,FH,48,176,304,432,17,18,19) \
      "add x9,x9,#512\n\t" \
      "add x11,x11,#64\n\t"
#define CH128_NPH1X2F CH128_NPH1X2(CH128_NFL,CH128_NFH)
#if CHAINHASH128_BLOCK_BYTES == 512
#define CH128_NPH1X2HI CH128_NPH1X2F CH128_NPH1X2F CH128_NPH1X2F CH128_NPH1X2F
#else
#define CH128_NPH1X2HI
#endif
/* Accumulate raw state (SL,SH) times y^4 into the lane, then pack the lane into the state. */
#define CH128_NFOLDL(SL,SH,LL,HH,MID) \
      CH128_NFL(30,SL,24,LL) CH128_NFL(LL,SH,25,30) \
      CH128_NFH(30,SL,24,HH) CH128_NFH(HH,SH,25,30) \
      CH128_NFL(30,SL,20,MID) CH128_NFH(MID,SL,20,30) \
      CH128_NFL(30,SH,21,MID) CH128_NFH(MID,SH,21,30) \
      "ext v30.16b,v31.16b,v" #MID ".16b,#8\n\t" \
      "eor v" #SL ".16b,v" #LL ".16b,v30.16b\n\t" \
      "ext v30.16b,v" #MID ".16b,v31.16b,#8\n\t" \
      "eor v" #SH ".16b,v" #HH ".16b,v30.16b\n\t"
#define CH128_NFOLDF \
      "ext v20.16b,v24.16b,v24.16b,#8\n\t" \
      "ext v21.16b,v25.16b,v25.16b,#8\n\t" \
      CH128_NFOLDL(0,1,8,9,10) \
      CH128_NFOLDL(2,3,11,12,13) \
      CH128_NFOLDL(4,5,14,15,16) \
      CH128_NFOLDL(6,7,17,18,19)
#define CH128_NPASS(OFF) \
      "add x9,%[p],#" #OFF "\n\t" \
      "mov x11,%[key]\n\t" \
      CH128_NPH1X2(CH128_NPL,CH128_NPH) \
      CH128_NPH1X2F \
      CH128_NPH1X2F \
      CH128_NPH1X2F \
      CH128_NPH1X2HI \
      CH128_NFOLDF
static __attribute__((noinline)) ch128_word ch128_n_bulk1(const chainhash128_key *k,const uint8_t *p,size_t regions,size_t len,ch128_raw *st) {
    ch128_raw state[4],acc={{0,0},{0,0}};uint8_t *out=(uint8_t *)state;unsigned j;
    if(st) memcpy(state,st,sizeof(state)); else { memset(state,0,sizeof(state)); state[3].lo.lo=len; }
    __asm__ volatile(
      "add x9,%[out],#64\n\t"
      "ld1 {v0.2d,v1.2d,v2.2d,v3.2d},[%[out]]\n\t"
      "ld1 {v4.2d,v5.2d,v6.2d,v7.2d},[x9]\n\t"
      "ld1 {v24.2d},[%[yp]]\n\t"
      "ld1 {v25.2d},[%[yh]]\n\t"
      "movi v31.2d,#0\n\t"
      "1:\n\t"
      CH128_NPASS(0)
      CH128_NPASS(64)
      "add %[p],%[p],%[rb]\n\t"
      "subs %[count],%[count],#1\n\t"
      "b.ne 1b\n\t"
      "st1 {v0.2d,v1.2d,v2.2d,v3.2d},[%[out]],#64\n\t"
      "st1 {v4.2d,v5.2d,v6.2d,v7.2d},[%[out]]\n\t"
      : [p] "+&r"(p),[count] "+&r"(regions),[out] "+&r"(out)
      : [key] "r"(k->ph),[yp] "r"(k->yp+4),[yh] "r"(k->yh+4),[rb] "I"(CH128_REGION)
      : "x9","x11","cc","memory","v0","v1","v2","v3","v4","v5","v6","v7","v8","v9","v10","v11","v12","v13","v14","v15","v16","v17","v18","v19","v20","v21","v22","v23","v24","v25","v26","v27","v28","v29","v30","v31");
    if(st) { memcpy(st,state,sizeof(state)); return ch128_make(0,0); }
    for(j=0;j<4;j++) {acc=ch128_rxor(acc,ch128_prod(state[j].lo,k->yp[3-j],4,1));acc=ch128_rxor(acc,ch128_prod(state[j].hi,k->yh[3-j],4,1));}return ch128_reduce(acc);
}
#else
/* Four fixed lazy chains. LD1 lists consume each comb half directly.
 * v0..v7: raw states; v8..v19: three-component block accumulators;
 * v20..v27: four word pairs; v28/v29: keys; v30/v31: scratch.
 * No hot-loop stack spills or vector-to-integer transfers are possible. */
/* Round-2: two-step batched schoolbook comb.
 * Processes two adjacent 256-byte chunks per invocation and folds each
 * pair's two low/high partial products into the lane accumulator with
 * E3(D,A,B): D^=A^B. CH128_NE3S is a single eor3 (FEAT_SHA3); CH128_NE3B
 * is two eor that combine the dead scratch A^=B first, so the accumulator
 * still takes one XOR per fold. XOR is associative/commutative so the
 * accumulated value, the reduction and the digest are bit-identical to
 * CH128_NSTEP1. Uses only x9 (data) + x11 (key) with ldr-immediate offsets;
 * keys are per-chunk and shared across the four lanes. Registers: v0-v7 states, v8-v19 lane
 * accumulators, v20-v23 words, v24/v25 chunk-A ll/hh, v26-v29 the two key
 * pairs, v30/v31 scratch. */
#define CH128_NE3S(D,A,B) "eor3 v" #D ".16b,v" #D ".16b,v" #A ".16b,v" #B ".16b\n\t"
#define CH128_NE3B(D,A,B) "eor v" #A ".16b,v" #A ".16b,v" #B ".16b\n\t" \
                          "eor v" #D ".16b,v" #D ".16b,v" #A ".16b\n\t"
#define CH128_NKEYS2 \
      "ldr q28,[x11,#0]\n\t" \
      "ldr q29,[x11,#16]\n\t" \
      "ldr q26,[x11,#32]\n\t" \
      "ldr q27,[x11,#48]\n\t"
#define CH128_NPAIR2(E3,OaA,ObA,OaB,ObB,LL,HH,MID) \
      "ldr q20,[x9,#" #OaA "]\n\t" \
      "ldr q21,[x9,#" #ObA "]\n\t" \
      "ldr q22,[x9,#" #OaB "]\n\t" \
      "ldr q23,[x9,#" #ObB "]\n\t" \
      "eor v20.16b,v20.16b,v28.16b\n\t" \
      "eor v21.16b,v21.16b,v29.16b\n\t" \
      "eor v22.16b,v22.16b,v26.16b\n\t" \
      "eor v23.16b,v23.16b,v27.16b\n\t" \
      "pmull v24.1q,v20.1d,v21.1d\n\t" \
      "pmull2 v25.1q,v20.2d,v21.2d\n\t" \
      "ext v30.16b,v21.16b,v21.16b,#8\n\t" \
      "pmull v31.1q,v20.1d,v30.1d\n\t" \
      "pmull2 v30.1q,v20.2d,v30.2d\n\t" \
      E3(MID,31,30) \
      "pmull v31.1q,v22.1d,v23.1d\n\t" \
      E3(LL,24,31) \
      "pmull2 v31.1q,v22.2d,v23.2d\n\t" \
      E3(HH,25,31) \
      "ext v30.16b,v23.16b,v23.16b,#8\n\t" \
      "pmull v31.1q,v22.1d,v30.1d\n\t" \
      "pmull2 v30.1q,v22.2d,v30.2d\n\t" \
      E3(MID,31,30)
#define CH128_NPH1X2(E3) \
      CH128_NKEYS2 \
      CH128_NPAIR2(E3,0,128,256,384,8,9,10) \
      CH128_NPAIR2(E3,16,144,272,400,11,12,13) \
      CH128_NPAIR2(E3,32,160,288,416,14,15,16) \
      CH128_NPAIR2(E3,48,176,304,432,17,18,19) \
      "add x9,x9,#512\n\t" \
      "add x11,x11,#64\n\t"
#if CHAINHASH128_BLOCK_BYTES == 512
#define CH128_NPH1X2HI(E3) CH128_NPH1X2(E3) CH128_NPH1X2(E3) CH128_NPH1X2(E3) CH128_NPH1X2(E3)
#else
#define CH128_NPH1X2HI(E3)
#endif
/* One template, two instantiations: ch128_n_bulk1s (eor3) and ch128_n_bulk1b
 * (eor only); PRE lets the SHA3 kernel assemble on a non-SHA3 target. */
#define CH128_NBULK1(NAME,E3,PRE) \
static __attribute__((noinline)) ch128_word NAME(const chainhash128_key *k,const uint8_t *p,size_t regions,size_t len,ch128_raw *st) { \
    ch128_raw state[4],acc={{0,0},{0,0}};uint8_t *out=(uint8_t *)state;unsigned j; \
    if(st) memcpy(state,st,sizeof(state)); else { memset(state,0,sizeof(state)); state[3].lo.lo=len; } \
    __asm__ volatile( \
      PRE \
      "add x9,%[out],#64\n\t" \
      "ld1 {v0.2d,v1.2d,v2.2d,v3.2d},[%[out]]\n\t" \
      "ld1 {v4.2d,v5.2d,v6.2d,v7.2d},[x9]\n\t" \
      "1:\n\t" \
      "movi v8.2d,#0\n\t" \
      "movi v9.2d,#0\n\t" \
      "movi v10.2d,#0\n\t" \
      "movi v11.2d,#0\n\t" \
      "movi v12.2d,#0\n\t" \
      "movi v13.2d,#0\n\t" \
      "movi v14.2d,#0\n\t" \
      "movi v15.2d,#0\n\t" \
      "movi v16.2d,#0\n\t" \
      "movi v17.2d,#0\n\t" \
      "movi v18.2d,#0\n\t" \
      "movi v19.2d,#0\n\t" \
      "add x9,%[p],#0\n\t" \
      "mov x11,%[key]\n\t" \
      CH128_NPH1X2(E3) \
      CH128_NPH1X2(E3) \
      CH128_NPH1X2(E3) \
      CH128_NPH1X2(E3) \
      CH128_NPH1X2HI(E3) \
      CH128_NFOLD1 \
      "movi v8.2d,#0\n\t" \
      "movi v9.2d,#0\n\t" \
      "movi v10.2d,#0\n\t" \
      "movi v11.2d,#0\n\t" \
      "movi v12.2d,#0\n\t" \
      "movi v13.2d,#0\n\t" \
      "movi v14.2d,#0\n\t" \
      "movi v15.2d,#0\n\t" \
      "movi v16.2d,#0\n\t" \
      "movi v17.2d,#0\n\t" \
      "movi v18.2d,#0\n\t" \
      "movi v19.2d,#0\n\t" \
      "add x9,%[p],#64\n\t" \
      "mov x11,%[key]\n\t" \
      CH128_NPH1X2(E3) \
      CH128_NPH1X2(E3) \
      CH128_NPH1X2(E3) \
      CH128_NPH1X2(E3) \
      CH128_NPH1X2HI(E3) \
      CH128_NFOLD1 \
      "add %[p],%[p],%[rb]\n\t" \
      "subs %[count],%[count],#1\n\t" \
      "b.ne 1b\n\t" \
      "st1 {v0.2d,v1.2d,v2.2d,v3.2d},[%[out]],#64\n\t" \
      "st1 {v4.2d,v5.2d,v6.2d,v7.2d},[%[out]]\n\t" \
      : [p] "+&r"(p),[count] "+&r"(regions),[out] "+&r"(out) \
      : [key] "r"(k->ph),[yp] "r"(k->yp+4),[yh] "r"(k->yh+4),[rb] "I"(CH128_REGION) \
      : "x9","x10","x11","cc","memory","v0","v1","v2","v3","v4","v5","v6","v7","v8","v9","v10","v11","v12","v13","v14","v15","v16","v17","v18","v19","v20","v21","v22","v23","v24","v25","v26","v27","v28","v29","v30","v31"); \
    if(st) { memcpy(st,state,sizeof(state)); return ch128_make(0,0); } \
    for(j=0;j<4;j++) {acc=ch128_rxor(acc,ch128_prod(state[j].lo,k->yp[3-j],4,1));acc=ch128_rxor(acc,ch128_prod(state[j].hi,k->yh[3-j],4,1));}return ch128_reduce(acc); \
}

#if CH128_SHA3 == 2
CH128_NBULK1(ch128_n_bulk1s,CH128_NE3S,".arch_extension sha3\n\t")
#elif CH128_SHA3 == 1
CH128_NBULK1(ch128_n_bulk1s,CH128_NE3S,"")
#endif
#if CH128_SHA3 != 1
CH128_NBULK1(ch128_n_bulk1b,CH128_NE3B,"")
#endif
static inline ch128_word ch128_n_bulk1(const chainhash128_key *k,const uint8_t *p,size_t regions,size_t len,ch128_raw *st) {
#if CH128_SHA3 == 2
    return ch128_has_sha3()?ch128_n_bulk1s(k,p,regions,len,st):ch128_n_bulk1b(k,p,regions,len,st);
#elif CH128_SHA3 == 1
    return ch128_n_bulk1s(k,p,regions,len,st);
#else
    return ch128_n_bulk1b(k,p,regions,len,st);
#endif
}
#endif
#undef CH128_NX3
#undef CH128_NPH0
#undef CH128_NSTEP0
#undef CH128_NFOLD0
#undef CH128_NPH1
#undef CH128_NSTEP1
#undef CH128_NFOLD1
#undef CH128_NFL
#undef CH128_NFH
#undef CH128_NPL
#undef CH128_NPH
#undef CH128_NKEYS2
#undef CH128_NPAIR2
#undef CH128_NPH1X2
#undef CH128_NPH1X2F
#undef CH128_NPH1X2HI
#undef CH128_NFOLDL
#undef CH128_NFOLDF
#undef CH128_NPASS
#undef CH128_NE3S
#undef CH128_NE3B
#undef CH128_NBULK1

CH128_NEND
#endif

static inline void ch128_region(const chainhash128_key *k,const uint8_t *p,size_t n,ch128_raw out[8],int b,int school) {
    if(n==CH128_REGION) {
#ifdef CH128_X86
        if(b==3) { ch128_512_region(k,p,out,school); return; }
        if(b==2) { ch128_256_region(k,p,out,school); return; }
        if(b==1) { ch128_128_region(k,p,out,school); return; }
#elif defined(CH128_ARM)
        if(b==4) { ch128_n_region(k,p,out,school); return; }
#endif
    }
    ch128_region_scalar(k,p,n,out,b,school);
}
typedef struct {
    const chainhash128_key *key; ch128_raw state[8]; uint64_t len,blocks;
    unsigned stride; int lazy,backend,school; size_t used; uint8_t buffer[CH128_REGION];
} chainhash128_stream;
static inline void chainhash128_init(chainhash128_stream *s,const chainhash128_key *k,unsigned stride,int lazy,int b,int school) {
    assert(stride>=1 && stride<=8 && chainhash128_has_backend(b)); assert((lazy==0 || lazy==1) && (school==0 || school==1));
    memset(s,0,sizeof(*s)); s->key=k; s->stride=stride; s->lazy=lazy; s->backend=b; s->school=school;
}
static inline void ch128_absorb(chainhash128_stream *s,const uint8_t *p,size_t n) {
    ch128_raw c[8]; unsigned j,count=ch128_lanes(n); const chainhash128_key *k=s->key;
    ch128_region(k,p,n,c,s->backend,s->school);
    for(j=0;j<count;j++) {
        ch128_raw *r=&s->state[s->blocks%s->stride];
        if(s->blocks<s->stride) { *r=c[j]; if(!s->lazy) { r->lo=ch128_reduce(*r); r->hi=ch128_make(0,0); } }
        else if(s->lazy) *r=ch128_rxor(c[j],ch128_rxor(ch128_prod(r->lo,k->yp[s->stride],s->backend,s->school),ch128_prod(r->hi,k->yh[s->stride],s->backend,s->school)));
        else r->lo=ch128_xor(ch128_mul(r->lo,k->yp[s->stride],s->backend),ch128_reduce(c[j]));
        ++s->blocks;
    }
}
/* Whole regions of a stream whose stride is the bulk kernel's chain count (8 on
 * XMM/YMM, 4 on ZMM/NEON) run through the kernel, which starts from the stream's
 * raw states and writes them back (eager states are reduced again). */
static inline int ch128_stream_bulk(chainhash128_stream *s,const uint8_t *p,size_t regions) {
#if defined(CH128_X86) || defined(CH128_ARM)
    const chainhash128_key *k=s->key; int b=s->backend,m=s->school; ch128_raw *st=s->state; unsigned j,lanes=b==CH128_XMM || b==CH128_YMM ? 8 : 4;
    if(!b || s->stride!=lanes || s->blocks%lanes) return 0;
#ifdef CH128_X86
    if(b==3) { if(m) ch128_512_bulk1(k,p,regions,0,st); else ch128_512_bulk0(k,p,regions,0,st); }
    else if(b==2) { if(m) ch128_256_bulk1(k,p,regions,0,st); else ch128_256_bulk0(k,p,regions,0,st); }
    else { if(m) ch128_128_bulk1(k,p,regions,0,st); else ch128_128_bulk0(k,p,regions,0,st); }
#else
    if(m) ch128_n_bulk1(k,p,regions,0,st); else ch128_n_bulk0(k,p,regions,0,st);
#endif
    if(!s->lazy) for(j=0;j<lanes;j++) { st[j].lo=ch128_reduce(st[j]); st[j].hi=ch128_make(0,0); }
    s->blocks+=8*(uint64_t)regions; return 1;
#else
    (void)s; (void)p; (void)regions; return 0;
#endif
}
static inline void chainhash128_update(chainhash128_stream *s,const void *data,size_t n) {
    const uint8_t *p=(const uint8_t *)data; assert(n<=UINT64_MAX-s->len); s->len+=n;
    if(s->used) { size_t take=CH128_REGION-s->used; if(take>n) take=n; if(take) { memcpy(s->buffer+s->used,p,take); p+=take; } s->used+=take; n-=take; if(s->used==CH128_REGION) { ch128_absorb(s,s->buffer,CH128_REGION); s->used=0; } }
    if(n>=CH128_REGION && ch128_stream_bulk(s,p,n/CH128_REGION)) { p+=n/CH128_REGION*CH128_REGION; n%=CH128_REGION; }
    while(n>=CH128_REGION) { ch128_absorb(s,p,CH128_REGION); p+=CH128_REGION; n-=CH128_REGION; }
    if(n) { memcpy(s->buffer,p,n); s->used=n; }
}
/* Consumes stream. Empty partitions use (value,blocks)=(0,0) instead. */
static inline ch128_word chainhash128_partial(chainhash128_stream *s) {
    ch128_raw v={{0,0},{0,0}}; unsigned j;
    if(s->used || !s->blocks) { ch128_absorb(s,s->buffer,s->used); s->used=0; }
    for(j=0;j<s->stride && j<s->blocks;j++) { unsigned e=(unsigned)((s->blocks-1-j)%s->stride);
        if(!e) v=ch128_rxor(v,s->state[j]);
        else v=ch128_rxor(v,ch128_rxor(ch128_prod(s->state[j].lo,s->key->yp[e],s->backend,s->school),ch128_prod(s->state[j].hi,s->key->yh[e],s->backend,s->school)));
    } return ch128_reduce(v);
}
static inline ch128_word chainhash128_final(chainhash128_stream *s) {
    ch128_word v=chainhash128_partial(s);
    v=ch128_xor(v,ch128_mul(ch128_make(s->len,0),ch128_pow(s->key->yp[1],s->blocks,s->backend),s->backend));
    return ch128_finish(s->key,v,s->backend);
}
static inline ch128_word chainhash128_join(const chainhash128_key *k,ch128_word left,ch128_word right,uint64_t right_blocks,int b) {
    return ch128_xor(ch128_mul(left,ch128_pow(k->yp[1],right_blocks,b),b),right);
}
static inline ch128_word chainhash128_evaluate(const chainhash128_key *k,const void *p,size_t n,unsigned stride,int lazy,int b,int school) {
    chainhash128_stream s; chainhash128_init(&s,k,stride,lazy,b,school); chainhash128_update(&s,p,n); return chainhash128_final(&s);
}
static inline ch128_word chainhash128_portable(const chainhash128_key *k,const void *data,size_t len) {
    const uint8_t *p=(const uint8_t *)data; size_t n=len; ch128_word v=ch128_make(len,0);
    do { ch128_raw c[8]; unsigned j; size_t take=n<CH128_REGION?n:CH128_REGION;
        ch128_region_scalar(k,p,take,c,0,0); for(j=0;j<ch128_lanes(take);j++) v=ch128_xor(ch128_mul_ref(v,k->yp[1]),ch128_reduce(c[j]));
        n-=take; if(!n) break; p+=take;
    } while(1); return ch128_finish(k,v,0);
}
#ifdef CH128_X86
/* One tail kernel for all x86 widths. Pad only the last chunk and key only
 * present pairs. Fold raw lanes with y^e and X^128*y^e, then reduce once.
 * Words are read in place: whole vectors straight from the input, the last
 * partial word as the 16 bytes ending at p+n shifted down (tails follow at least
 * 128 input bytes), and the reversed y powers are formed in registers, so no
 * vector load reads back narrower stores (store-forwarding stalls). */
CH128_T128 static inline __m128i ch128_128_word(const uint8_t *q,size_t o,size_t rem) {
    static const uint8_t idx[32]={0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,
        0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80};
    if(o+16<=rem) return ch128_128_load(q+o);
    if(o>=rem) return ch128_128_zero();
    return _mm_shuffle_epi8(ch128_128_load(q+rem-16),ch128_128_load(idx+16-(rem-o)));
}
CH128_T128 static inline __m128i ch128_128_mk(__m128i a,__m128i b,__m128i c,__m128i d) { (void)b; (void)c; (void)d; return a; }
CH128_T256 static inline __m256i ch128_256_mk(__m128i a,__m128i b,__m128i c,__m128i d) { (void)c; (void)d; return _mm256_inserti128_si256(_mm256_castsi128_si256(a),b,1); }
CH128_T512 static inline __m512i ch128_512_mk(__m128i a,__m128i b,__m128i c,__m128i d) {
    return _mm512_inserti32x4(_mm512_inserti32x4(_mm512_inserti32x4(_mm512_castsi128_si512(a),b,1),c,2),d,3);
}
#define CH128_XTAIL(P,T,N,V) \
/* Words j..j+N-1 of the half at off of the last chunk, keyed; a pair whose first \
 * word is absent is zero. */ \
T static inline V P##_tw(const uint8_t *q,unsigned j,size_t rem,const ch128_word *key,size_t off) { \
    __m128i w[4],kk=ch128_128_load(key); unsigned t; \
    if(off+16*(j+N)<=rem) return P##_xor(P##_load(q+off+16*j),P##_bc(key)); \
    for(t=0;t<4;t++) w[t]=t<N && 16*(j+t)<rem ? ch128_128_xor(ch128_128_word(q,off+16*(j+t),rem),kk) : ch128_128_zero(); \
    return P##_mk(w[0],w[1],w[2],w[3]); \
} \
T static inline V P##_yw(const ch128_word *y,size_t count,unsigned j) { \
    __m128i w[4]; unsigned t; \
    for(t=0;t<4;t++) w[t]=t<N && j+t<count ? ch128_128_load(y+count-1-j-t) : ch128_128_zero(); \
    return P##_mk(w[0],w[1],w[2],w[3]); \
} \
T static inline ch128_word P##_tail(const chainhash128_key *k,const uint8_t *p,size_t n,ch128_word v,int school) { \
    ch128_word lo[N],hi[N]; \
    size_t chunks=n/256,rem=n%256,count=ch128_lanes(n); unsigned j,c; \
    const uint8_t *q=p+256*chunks; P##_acc sum=P##_azero(); P##_raw r; \
    ch128_128_raw total=ch128_128_prod(_mm_set_epi64x((long long)v.hi,(long long)v.lo),ch128_128_load(k->yp+count),0); \
    for(j=0;j<8;j+=N) { \
        P##_acc a=P##_azero(); \
        for(c=0;c<chunks;c++) \
            a=P##_accum(a,P##_xor(P##_load(p+256*c+16*j),P##_bc(k->ph+2*c)), \
                           P##_xor(P##_load(p+256*c+16*j+128),P##_bc(k->ph+2*c+1)),school); \
        if(16*j<rem) a=P##_accum(a,P##_tw(q,j,rem,k->ph+2*chunks,0),P##_tw(q,j,rem,k->ph+2*chunks+1,128),school); \
        r=P##_pack(a,school); \
        sum=P##_accum(sum,r.lo,P##_yw(k->yp,count,j),0); \
        sum=P##_accum(sum,r.hi,P##_yw(k->yh,count,j),0); \
    } \
    r=P##_pack(sum,0); P##_store(lo,r.lo); P##_store(hi,r.hi); \
    for(j=0;j<N;j++) { total.lo=ch128_128_xor(total.lo,ch128_128_load(lo+j)); total.hi=ch128_128_xor(total.hi,ch128_128_load(hi+j)); } \
    ch128_128_store(&v,ch128_128_reduce(total)); return v; \
}
CH128_XTAIL(ch128_128,CH128_T128,1,__m128i)
CH128_XTAIL(ch128_256,CH128_T256,2,__m256i)
CH128_XTAIL(ch128_512,CH128_T512,4,__m512i)
#undef CH128_XTAIL
#endif
#ifdef CH128_ARM
CH128_NBEGIN
/* Evaluate a partial comb region in vector registers. Only the final 256-byte
 * chunk needs padding (words read in place by ch128_n_word; tails follow at least
 * 128 input bytes); skip absent first words so no key-only pairs are added.
 * The cached powers combine the independent lanes without a serial Horner chain. */
static inline ch128_word ch128_n_tail(const chainhash128_key *k,const uint8_t *p,size_t n,ch128_word v,int school) {
    size_t chunks=n/256,rem=n%256,count=ch128_lanes(n); unsigned j,c; const uint8_t *q=p+256*chunks;
    ch128_n_acc sum=ch128_n_accum(ch128_n_azero(),vcombine_u64(vcreate_u64(v.lo),vcreate_u64(v.hi)),ch128_n_load(k->yp+count),0);
    ch128_n_raw last;
    last.lo=last.hi=ch128_n_zero();
    for(j=0;j<count;j++) {
        ch128_n_acc a=ch128_n_azero(); ch128_n_raw r;
        for(c=0;c<chunks;c++)
            a=ch128_n_accum(a,ch128_n_xor(ch128_n_load(p+256*c+16*j),ch128_n_load(k->ph+2*c)),
                              ch128_n_xor(ch128_n_load(p+256*c+16*j+128),ch128_n_load(k->ph+2*c+1)),school);
        if(16*j<rem)
            a=ch128_n_accum(a,ch128_n_xor(ch128_n_word(q,16*j,rem),ch128_n_load(k->ph+2*chunks)),
                              ch128_n_xor(ch128_n_word(q,16*j+128,rem),ch128_n_load(k->ph+2*chunks+1)),school);
        r=ch128_n_pack(a,school);
        if(j+1<count) sum=ch128_n_accum(sum,ch128_n_reduce(r),ch128_n_load(k->yp+count-1-j),0);
        else last=r;
    }
    { ch128_n_raw r=ch128_n_pack(sum,0);
      r.lo=ch128_n_xor(r.lo,last.lo); r.hi=ch128_n_xor(r.hi,last.hi);
      ch128_n_store(&v,ch128_n_reduce(r)); }
    return v;
}
CH128_NEND
#endif
#if defined(__GNUC__) || defined(__clang__)
#define CH128_NOINLINE __attribute__((noinline))
#else
#define CH128_NOINLINE
#endif
static CH128_NOINLINE ch128_word ch128_hash_body(const chainhash128_key *k,const void *data,size_t len,int b,int school,int hint,unsigned step,size_t dist) {
    const uint8_t *p=(const uint8_t *)data; size_t full=len/CH128_REGION,n=len%CH128_REGION; ch128_word v=ch128_make(len,0);
    (void)hint; (void)step; (void)dist;
    if(full) {
#ifdef CH128_X86
        if(b==3) v=hint?ch128_512_pf(k,p,full,len,0,hint,step,dist):school?ch128_512_bulk1(k,p,full,len,0):ch128_512_bulk0(k,p,full,len,0);
        else if(b==2) v=hint?ch128_256_pf(k,p,full,len,0,hint,step,dist):school?ch128_256_bulk1(k,p,full,len,0):ch128_256_bulk0(k,p,full,len,0);
        else if(b==1) v=hint?(school?ch128_128_pf(k,p,full,len,0,hint,step,dist):ch128_128_pfk(k,p,full,len,0,hint,step,dist)):school?ch128_128_bulk1(k,p,full,len,0):ch128_128_bulk0(k,p,full,len,0);
        else return chainhash128_evaluate(k,data,len,8,1,b,school);
#elif defined(CH128_ARM)
        if(b==4) v=school?ch128_n_bulk1(k,p,full,len,0):ch128_n_bulk0(k,p,full,len,0);
        else return chainhash128_evaluate(k,data,len,8,1,b,school);
#else
        return chainhash128_evaluate(k,data,len,8,1,b,school);
#endif
        p+=full*CH128_REGION;
    }
#ifdef CH128_X86
    if(n && b) {
        if(b==3) v=ch128_512_tail(k,p,n,v,school);
        else if(b==2) v=ch128_256_tail(k,p,n,v,school);
        else v=ch128_128_tail(k,p,n,v,school);
        return ch128_finish(k,v,b);
    }
#elif defined(CH128_ARM)
    if(b==4 && n) return ch128_finish(k,ch128_n_tail(k,p,n,v,school),b);
#endif
    if(n || !full) { ch128_raw c[8]; unsigned j; ch128_region_scalar(k,p,n,c,b,school); for(j=0;j<ch128_lanes(n);j++) v=ch128_xor(ch128_mul(v,k->yp[1],b),ch128_reduce(c[j])); }
    return ch128_finish(k,v,b);
}
/* Inputs up to 128 bytes take the register-only path without the general body's frame. */
static inline ch128_word ch128_hash(const chainhash128_key *k,const void *data,size_t len,int b,int school,int hint,unsigned step,size_t dist) {
    assert(chainhash128_has_backend(b)); assert(school==0 || school==1);
#ifdef CH128_X86
    if(b && len<=128) return ch128_128_short(k,(const uint8_t *)data,len,school);
#elif defined(CH128_ARM)
    if(b && len<=128) return ch128_n_short(k,(const uint8_t *)data,len,school);
#endif
    return ch128_hash_body(k,data,len,b,school,hint,step,dist);
}
static inline ch128_word chainhash128_with_backend(const chainhash128_key *k,const void *data,size_t len,int b,int school) { return ch128_hash(k,data,len,b,school,CH_PF_OFF,64,0); }
static inline ch128_word chainhash128(const chainhash128_key *k,const void *p,size_t n) { int b=chainhash128_backend(); return chainhash128_with_backend(k,p,n,b,ch128_school(b)); }
/* chainhash128() with an explicit backend and software prefetch (CH_PF_*, every
 * step = 64 or 128 bytes, dist bytes ahead; x86 only), with the backend's default
 * product method. An unavailable backend falls back to the detected one. The digest
 * is the same for every argument; chainhash_calibrate.h chooses them. */
static inline ch128_word chainhash128_with_prefetch(const chainhash128_key *k,const void *data,size_t len,int b,int hint,unsigned step,size_t dist) {
    if(!chainhash128_has_backend(b)) b=chainhash128_backend();
#ifndef CH128_X86
    hint=CH_PF_OFF;
#endif
    return ch128_hash(k,data,len,b,ch128_school(b),hint>=CH_PF_T0 && hint<=CH_PF_NTA ? hint : CH_PF_OFF,step,dist);
}
static inline int chainhash128_selftest(void) {
    uint8_t m[CH128_REGION+1]; size_t n; chainhash128_key k=chainhash128_key_from_seed(123);
    for(n=0;n<sizeof(m);n++) m[n]=(uint8_t)(n*137+29);
    /* Independent formal evaluator vectors, test/128/vectors.c. */
    if(!ch128_equal(chainhash128(&k,m,0),ch128_make(UINT64_C(0x66be5c470e2ee79f),UINT64_C(0xcb2a7994d9c3a248))) ||
       !ch128_equal(chainhash128(&k,m,17),ch128_make(UINT64_C(0x2b2f07c62c4ea752),UINT64_C(0xfd15779069b8d199)))) return 0;
#if CHAINHASH128_BLOCK_BYTES == 512
    if(!ch128_equal(chainhash128(&k,m,2049),ch128_make(UINT64_C(0xa298054ce47e3d7d),UINT64_C(0x756d75e371f7f634)))) return 0;
#else
    if(!ch128_equal(chainhash128(&k,m,2049),ch128_make(UINT64_C(0x0d09cd594310045d),UINT64_C(0xaa6d98bd3e2ed773)))) return 0;
#endif
    for(n=0;n<sizeof(m);n=n<65?n+1:n+127) if(!ch128_equal(chainhash128(&k,m,n),chainhash128_portable(&k,m,n))) return 0;
    return ch128_equal(ch128_mul_ref(ch128_make(0,UINT64_C(1)<<63),ch128_make(2,0)),ch128_make(0x87,0));
}
#endif
