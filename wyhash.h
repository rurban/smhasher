/*	Author: Wang Yi <godspeed_china@yeah.net>	*/
#ifndef wyhash_version_3
#define wyhash_version_3
#include	<stdint.h>
#include	<string.h>
#if defined(_MSC_VER) && defined(_M_X64)
#include <intrin.h>
#pragma	intrinsic(_umul128)
#endif
const	uint64_t	_wyp0=0xa0761d6478bd642full,	_wyp1=0xe7037ed1a0b428dbull,	_wyp2=0x8ebc6af09c88c6e3ull,	_wyp3=0x589965cc75374cc3ull,	_wyp4=0x1d8e4e27c47d124full;
static	inline	uint64_t	_wyrotr(uint64_t v, unsigned k) {	return	(v>>k)|(v<<(64-k));	}
static	inline	uint64_t	_wymum(uint64_t	A,	uint64_t	B) {
#ifdef	WYHASH32
	uint64_t	hh=(A>>32)*(B>>32),	hl=(A>>32)*(unsigned)B,	lh=(unsigned)A*(B>>32),	ll=(uint64_t)(unsigned)A*(unsigned)B;
	return	_wyrotr(hl,32)^_wyrotr(lh,32)^hh^ll;
#else
#ifdef __SIZEOF_INT128__
	__uint128_t	r=A;	r*=B;	return	(r>>64)^r;
#elif	defined(_MSC_VER) && defined(_M_X64)
	A=_umul128(A, B, &B);	return	A^B;
#else
	uint64_t	ha=A>>32,	hb=B>>32,	la=(uint32_t)A,	lb=(uint32_t)B,	hi, lo;
	uint64_t	rh=ha*hb,	rm0=ha*lb,	rm1=hb*la,	rl=la*lb,	t=rl+(rm0<<32),	c=t<rl;
	lo=t+(rm1<<32);	c+=lo<t;hi=rh+(rm0>>32)+(rm1>>32)+c;	return hi^lo;
#endif
#endif
}
static	inline	uint64_t	_wyr8(const	uint8_t	*p)	{	uint64_t v;	memcpy(&v,  p,  8);	return  v;	}
static	inline	uint64_t	_wyr4(const	uint8_t	*p) {	uint32_t v;	memcpy(&v,	p,	4);	return	v;	}
static	inline	uint64_t	_wyr3(const	uint8_t	*p,	unsigned	k) {	return	(((uint64_t)p[0])<<16)|(((uint64_t)p[k>>1])<<8)|p[k-1];	}
static	inline	uint64_t	wyhash(const void* key,	uint64_t	len,	uint64_t	seed) {
#if defined(__GNUC__) || defined(__INTEL_COMPILER)
	if(__builtin_expect(!len,0))	return	0;
#else
	if(!len)	return	0;
#endif
	const	uint8_t	*p=(const	uint8_t*)key;
	if(len<4)	return	_wymum(_wymum(_wyr3(p,len)^seed^_wyp0,seed^_wyp1),len^_wyp4);
	else	if(len<=8)	return	_wymum(_wymum(_wyr4(p)^seed^_wyp0,_wyr4(p+len-4)^seed^_wyp1),len^_wyp4);
	else	if(len<=16)	return	_wymum(_wymum(_wyr8(p)^seed^_wyp0,_wyr8(p+len-8)^seed^_wyp1),len^_wyp4);
	else	if(len<=24)	return	_wymum(_wymum(_wyr8(p)^seed^_wyp0,_wyr8(p+8)^seed^_wyp1)^_wymum(_wyr8(p+len-8)^seed^_wyp2,seed^_wyp3),len^_wyp4);
	else	if(len<=32)	return	_wymum(_wymum(_wyr8(p)^seed^_wyp0,_wyr8(p+8)^seed^_wyp1)^_wymum(_wyr8(p+16)^seed^_wyp2,_wyr8(p+len-8)^seed^_wyp3),len^_wyp4);
	uint64_t	see1=seed,	i=len;
	if(i>=256)	for(; i>=256; i-=256,p+=256) {
		seed=_wymum(_wyr8(p)^seed^_wyp0,_wyr8(p+8)^seed^_wyp1)^_wymum(_wyr8(p+16)^seed^_wyp2,_wyr8(p+24)^seed^_wyp3);
		see1=_wymum(_wyr8(p+32)^see1^_wyp1,_wyr8(p+40)^see1^_wyp2)^_wymum(_wyr8(p+48)^see1^_wyp3,_wyr8(p+56)^see1^_wyp0);
		seed=_wymum(_wyr8(p+64)^seed^_wyp0,_wyr8(p+72)^seed^_wyp1)^_wymum(_wyr8(p+80)^seed^_wyp2,_wyr8(p+88)^seed^_wyp3);
		see1=_wymum(_wyr8(p+96)^see1^_wyp1,_wyr8(p+104)^see1^_wyp2)^_wymum(_wyr8(p+112)^see1^_wyp3,_wyr8(p+120)^see1^_wyp0);
		seed=_wymum(_wyr8(p+128)^seed^_wyp0,_wyr8(p+136)^seed^_wyp1)^_wymum(_wyr8(p+144)^seed^_wyp2,_wyr8(p+152)^seed^_wyp3);
		see1=_wymum(_wyr8(p+160)^see1^_wyp1,_wyr8(p+168)^see1^_wyp2)^_wymum(_wyr8(p+176)^see1^_wyp3,_wyr8(p+184)^see1^_wyp0);
		seed=_wymum(_wyr8(p+192)^seed^_wyp0,_wyr8(p+200)^seed^_wyp1)^_wymum(_wyr8(p+208)^seed^_wyp2,_wyr8(p+216)^seed^_wyp3);
		see1=_wymum(_wyr8(p+224)^see1^_wyp1,_wyr8(p+232)^see1^_wyp2)^_wymum(_wyr8(p+240)^see1^_wyp3,_wyr8(p+248)^see1^_wyp0);
	}
	for(; i>=32; i-=32,p+=32) {	seed=_wymum(_wyr8(p)^seed^_wyp0,_wyr8(p+8)^seed^_wyp1);	see1=_wymum(_wyr8(p+16)^see1^_wyp2,_wyr8(p+24)^see1^_wyp3);	}
	if(!i) {}
	else	if(i<4)	seed=_wymum(_wyr3(p,i)^seed^_wyp0,seed^_wyp1);
	else	if(i<=8)	seed=_wymum(_wyr4(p)^seed^_wyp0,_wyr4(p+i-4)^seed^_wyp1);
	else	if(i<=16)	seed=_wymum(_wyr8(p)^seed^_wyp0,_wyr8(p+i-8)^seed^_wyp1);
	else	if(i<=24) {	seed=_wymum(_wyr8(p)^seed^_wyp0,_wyr8(p+8)^seed^_wyp1);	see1=_wymum(_wyr8(p+i-8)^see1^_wyp2,see1^_wyp3);	} 
	else {	seed=_wymum(_wyr8(p)^seed^_wyp0,_wyr8(p+8)^seed^_wyp1);	see1=_wymum(_wyr8(p+16)^see1^_wyp2,_wyr8(p+i-8)^see1^_wyp3);	}
	return	_wymum(seed^see1,len^_wyp4);
}
static	inline	uint64_t	wyhash64(uint64_t	A, uint64_t	B) {	return	_wymum(_wymum(A^_wyp0,	B^_wyp1),	_wyp2);	}
static	inline	double	wy2u01(uint64_t	r) {	const	double	_wynorm=1.0/(1ull<<52);	return	(r>>11)*_wynorm;	}
static	inline	float	wy2gau(uint64_t	r) {	const	float	_wynorm1=1.0f/(1ull<<15);	return	(((r>>16)&0xffff)+((r>>32)&0xffff)+(r>>48))*_wynorm1-3.0f;	}
static	inline	uint64_t	wyrand(uint64_t	*seed) {	*seed+=_wyp0;	return	_wymum(*seed^_wyp1,*seed);	}
static	uint64_t	_wyrand_seed=0;
#define	WYRAND_MAX	0xffffffffffffffffull
static	inline	void	wysrand(uint64_t	seed) {	_wyrand_seed=seed;	}
static	inline	uint64_t	wygrand(void) {
	uint64_t s;
#if defined(_OPENMP)
	#pragma omp atomic capture
#endif
	{
		_wyrand_seed+=_wyp0;	s=_wyrand_seed;
	}
	return	_wymum(s^_wyp1,s);
}
#endif
