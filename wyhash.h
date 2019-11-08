/*	Author: Wang Yi <godspeed_china@yeah.net>	*/
#ifndef wyhash_version_3
#define wyhash_version_3
#include	<stdint.h>
#include	<string.h>
#if defined(_MSC_VER) && defined(_M_X64)
#include <intrin.h>
#pragma	intrinsic(_umul128)
#endif
#ifndef UNLIKELY
	#if defined(__GNUC__) || defined(__INTEL_COMPILER)
		#define UNLIKELY(x) (__builtin_expect(!!(x), 0))
	#else
		#define UNLIKELY(x) (x)
	#endif
#endif
const	uint64_t	_wyp0=0xa0761d6478bd642full,	_wyp1=0xe7037ed1a0b428dbull,	_wyp2=0x8ebc6af09c88c6e3ull,	_wyp3=0x589965cc75374cc3ull,	_wyp4=0x1d8e4e27c47d124full;
static	inline	uint64_t	_wymum(uint64_t	A,	uint64_t	B) {
#ifdef __SIZEOF_INT128__
	__uint128_t	r=A;	r*=B;
	return	(r>>64)^r;
#elif	defined(_MSC_VER) && defined(_M_X64)
	A=_umul128(A, B, &B);
	return	A^B;
#else
	uint64_t	ha=A>>32,	hb=B>>32,	la=(uint32_t)A,	lb=(uint32_t)B,	hi, lo;
	uint64_t	rh=ha*hb,	rm0=ha*lb,	rm1=hb*la,	rl=la*lb,	t=rl+(rm0<<32),	c=t<rl;
	lo=t+(rm1<<32);	c+=lo<t;	hi=rh+(rm0>>32)+(rm1>>32)+c;	return hi^lo;
#endif
}
static	inline	uint64_t	_wymix0(uint64_t	A,	uint64_t	B,	uint64_t	seed) {	return	_wymum(A^seed^_wyp0,	B^seed^_wyp1);}
static	inline	uint64_t	_wymix1(uint64_t	A,	uint64_t	B,	uint64_t	seed) {	return	_wymum(A^seed^_wyp2,	B^seed^_wyp3);}
static	inline	uint64_t	_wymix2(uint64_t	A,	uint64_t	B,	uint64_t	seed) {	return	_wymum(A^seed^_wyp3,	B^seed^_wyp0);}
static	inline	uint64_t	_wymix3(uint64_t	A,	uint64_t	B,	uint64_t	seed) {	return	_wymum(A^seed^_wyp1,	B^seed^_wyp2);}
static	inline	uint64_t	_wyr1(const	uint8_t	*p) {	uint8_t v;	memcpy(&v,	p,	1);	return	v;}
static	inline	uint64_t	_wyr2(const	uint8_t	*p) {	uint16_t v;	memcpy(&v,	p,	2);	return	v;}
static	inline	uint64_t	_wyr3(const	uint8_t	*p) {	return	(_wyr2(p)<<8)|_wyr1(p+2);}
static	inline	uint64_t	_wyr4(const	uint8_t	*p) {	uint32_t v;	memcpy(&v,	p,	4);	return	v;}
static	inline	uint64_t	_wyr5(const	uint8_t	*p) {	return	(_wyr4(p)<<8)|_wyr1(p+4);}
static	inline	uint64_t	_wyr6(const	uint8_t	*p) {	return	(_wyr4(p)<<16)|_wyr2(p+4);}
static	inline	uint64_t	_wyr7(const	uint8_t	*p) {	return	(_wyr4(p)<<24)|(_wyr2(p+4)<<8)|_wyr1(p+6);}
static	inline	uint64_t	_wyr8(const	uint8_t	*p) {	return	(_wyr4(p)<<32)|_wyr4(p+4);}
static	inline	uint64_t	__wyr8(const	uint8_t	*p) {	uint64_t v;	memcpy(&v,	p,	8);	return	v;}
//to avoid attacks, seed should be initialized as a secret.
static	inline	uint64_t	wyhash(const void* key,	uint64_t	len,	uint64_t	seed) {
#ifndef	WYHASH_SAFE
	const   uint64_t    *q=(const   uint64_t*)key;
	if(len<=8)	return	_wymum(len^_wyp4,_wymum((q[0]<<((8-(len&7))<<3))^seed^_wyp0,seed^_wyp1));	
	if(len<=16)	return	_wymum(len^_wyp4,_wymum(q[0]^seed^_wyp0,(q[1]<<((8-(len&7))<<3))^seed^_wyp1));
	if(len<=24)	return	_wymum(len^_wyp4,_wymum(q[0]^seed^_wyp0,q[1]^seed^_wyp1)^_wymum((q[2]<<((8-(len&7))<<3))^seed^_wyp2,seed^_wyp3));
	if(len<=32)	return	_wymum(len^_wyp4,_wymum(q[0]^seed^_wyp0,q[1]^seed^_wyp1)^_wymum(q[2]^seed^_wyp2,(q[3]<<((8-(len&7))<<3))^seed^_wyp3));
#endif
	const	uint8_t	*p=(const	uint8_t*)key;	uint64_t i,	see1=seed,see2=seed,see3=seed;
	for(i=0;	UNLIKELY(i+128<=len);	i+=128,	p+=128) {
		seed=_wymix0(__wyr8(p),__wyr8(p+8),seed);	see1=_wymix1(__wyr8(p+16),__wyr8(p+24),see1);	
		see2=_wymix2(__wyr8(p+32),__wyr8(p+40),see2);	see3=_wymix3(__wyr8(p+48),__wyr8(p+56),see3);
		seed=_wymix0(__wyr8(p+64),__wyr8(p+72),seed);	see1=_wymix1(__wyr8(p+80),__wyr8(p+88),see1);	
		see2=_wymix2(__wyr8(p+96),__wyr8(p+104),see2);	see3=_wymix3(__wyr8(p+112),__wyr8(p+120),see3);
	}
	for(;	UNLIKELY(i+32<=len);	i+=32,	p+=32) {	seed=_wymix0(__wyr8(p),__wyr8(p+8),seed);	see1=_wymix1(__wyr8(p+16),__wyr8(p+24),see1);	}
	switch(len&31) {
	case	1:	seed=_wymix0(_wyr1(p),0,seed);	break;
	case	2:	seed=_wymix0(_wyr2(p),0,seed);	break;
	case	3:	seed=_wymix0(_wyr3(p),0,seed);	break;
	case	4:	seed=_wymix0(_wyr4(p),0,seed);		break;
	case	5:	seed=_wymix0(_wyr5(p),0,seed);	break;
	case	6:	seed=_wymix0(_wyr6(p),0,seed);	break;
	case	7:	seed=_wymix0(_wyr7(p),0,seed);	break;
	case	8:	seed=_wymix0(_wyr8(p),0,seed);	break;
	case	9:	seed=_wymix0(_wyr8(p),_wyr1(p+8),seed);	break;
	case	10:	seed=_wymix0(_wyr8(p),_wyr2(p+8),seed);	break;
	case	11:	seed=_wymix0(_wyr8(p),_wyr3(p+8),seed);	break;
	case	12:	seed=_wymix0(_wyr8(p),_wyr4(p+8),seed);	break;
	case	13:	seed=_wymix0(_wyr8(p),_wyr5(p+8),seed);	break;
	case	14:	seed=_wymix0(_wyr8(p),_wyr6(p+8),seed);	break;
	case	15:	seed=_wymix0(_wyr8(p),_wyr7(p+8),seed);	break;
	case	16:	seed=_wymix0(_wyr8(p),_wyr8(p+8),seed);	break;
	case	17:	seed=_wymix0(_wyr8(p),_wyr8(p+8),seed);	see1=_wymix1(_wyr1(p+16),0,see1);	break;
	case	18:	seed=_wymix0(_wyr8(p),_wyr8(p+8),seed);	see1=_wymix1(_wyr2(p+16),0,see1);	break;
	case	19:	seed=_wymix0(_wyr8(p),_wyr8(p+8),seed);	see1=_wymix1(_wyr3(p+16),0,see1);	break;
	case	20:	seed=_wymix0(_wyr8(p),_wyr8(p+8),seed);	see1=_wymix1(_wyr4(p+16),0,see1);	break;
	case	21:	seed=_wymix0(_wyr8(p),_wyr8(p+8),seed);	see1=_wymix1(_wyr5(p+16),0,see1);	break;
	case	22:	seed=_wymix0(_wyr8(p),_wyr8(p+8),seed);	see1=_wymix1(_wyr6(p+16),0,see1);	break;
	case	23:	seed=_wymix0(_wyr8(p),_wyr8(p+8),seed);	see1=_wymix1(_wyr7(p+16),0,see1);	break;
	case	24:	seed=_wymix0(_wyr8(p),_wyr8(p+8),seed);	see1=_wymix1(_wyr8(p+16),0,see1);	break;
	case	25:	seed=_wymix0(_wyr8(p),_wyr8(p+8),seed);	see1=_wymix1(_wyr8(p+16),_wyr1(p+24),see1);	break;
	case	26:	seed=_wymix0(_wyr8(p),_wyr8(p+8),seed);	see1=_wymix1(_wyr8(p+16),_wyr2(p+24),see1);	break;
	case	27:	seed=_wymix0(_wyr8(p),_wyr8(p+8),seed);	see1=_wymix1(_wyr8(p+16),_wyr3(p+24),see1);	break;
	case	28:	seed=_wymix0(_wyr8(p),_wyr8(p+8),seed);	see1=_wymix1(_wyr8(p+16),_wyr4(p+24),see1);	break;
	case	29:	seed=_wymix0(_wyr8(p),_wyr8(p+8),seed);	see1=_wymix1(_wyr8(p+16),_wyr5(p+24),see1);	break;
	case	30:	seed=_wymix0(_wyr8(p),_wyr8(p+8),seed);	see1=_wymix1(_wyr8(p+16),_wyr6(p+24),see1);	break;
	case	31:	seed=_wymix0(_wyr8(p),_wyr8(p+8),seed);	see1=_wymix1(_wyr8(p+16),_wyr7(p+24),see1);	break;
	}
	return	_wymum(seed^see1^see2,see3^len^_wyp4);
}
static	inline	uint64_t	wyhash64(uint64_t	A, uint64_t	B) {	return	_wymum(_wymum(A^_wyp0,	B^_wyp1),	_wyp2);}
static	inline	double	wy2u01(uint64_t	r) {	const	double	_wynorm=1.0/(1ull<<52);	return	(r&0x000fffffffffffffull)*_wynorm;}
static	inline	float	wy2gau(uint64_t	r) {	const	float	_wynorm1=1.0f/(1ull<<15);	return	(((r>>16)&0xffff)+((r>>32)&0xffff)+(r>>48))*_wynorm1-3.0f;}
static	inline	uint64_t	wyrand(uint64_t	*seed) {	*seed+=_wyp0;	return	_wymum(*seed^_wyp1,*seed);}
static	uint64_t	_wyrand_seed=0;
#define	WYRAND_MAX	0xffffffffffffffffull
static	inline	void	wysrand(uint64_t	seed) {	_wyrand_seed=seed;}
static	inline	uint64_t	wygrand(void) {
	uint64_t s;
#if defined(_OPENMP)
	#pragma omp atomic capture
#endif
	{	_wyrand_seed += _wyp0;	s = _wyrand_seed;	}
	return	_wymum(s^_wyp1,s);
}
#endif
