//Author: Wang Yi <godspeed_china@yeah.net>
#ifndef wyhash_included
#define wyhash_included
#ifndef UNLIKELY
#if defined(__GNUC__) || defined(__INTEL_COMPILER)
#define UNLIKELY(x) (__builtin_expect(!!(x), 0))
#else
#define UNLIKELY(x) (x)
#endif
#endif
const	unsigned long long	wyhashp0=0x60bee2bee120fc15ull;
const	unsigned long long	wyhashp1=0xa3b195354a39b70dull;
const	unsigned long long	wyhashp2=0x1b03738712fad5c9ull;
const	unsigned long long	wyhashp3=0xd985068bc5439bd7ull;
const	unsigned long long	wyhashp4=0x897f236fb004a8e7ull;
inline	unsigned long long	wyhashmix(unsigned long long	A,	unsigned long long	B){	
#ifdef __SIZEOF_INT128__
	__uint128_t	r=A;	r*=B^wyhashp0;	return	(r>>64)^r;	
#else
	B^=wyhashp0;
	unsigned long long	ha=A>>32,	hb=B>>32,	la=(unsigned int)A,	lb=(unsigned int)B,	hi, lo;
	unsigned long long	rh=ha*hb,	rm0=ha*lb,	rm1=hb*la,	rl=la*lb,	t=rl+(rm0<<32),	c=t<rl;
	lo=t+(rm1<<32);	c+=lo<t;	hi=rh+(rm0>>32)+(rm1>>32)+c;
	return hi^lo;
#endif
}
inline	unsigned long long	wyhashread64(const	void	*const	ptr){	return	*(unsigned long long*)(ptr);	}
inline	unsigned long long	wyhashread32(const	void	*const	ptr){	return	*(unsigned int*)(ptr);	}
inline	unsigned long long	wyhashread16(const	void	*const	ptr){	return	*(unsigned short*)(ptr);	}
inline	unsigned long long	wyhashread08(const	void	*const	ptr){	return	*(unsigned char*)(ptr);	}
inline	unsigned long long	wyhash(const void* key,	unsigned long long	len, unsigned long long	seed){
	const	unsigned char	*ptr=(const	unsigned char*)key,	*const	end=ptr+len;
	for(;	UNLIKELY(ptr+32<end);	ptr+=32)
		seed=wyhashmix(seed^wyhashp1,wyhashread64(ptr))
		^wyhashmix(seed^wyhashp2,wyhashread64(ptr+8))
		^wyhashmix(seed^wyhashp3,wyhashread64(ptr+16))
		^wyhashmix(seed^wyhashp4,wyhashread64(ptr+24));
	switch(len&31){
	case	0:	return	wyhashmix(
		wyhashmix(seed^wyhashp1,wyhashread64(ptr))
		^wyhashmix(seed^wyhashp2,wyhashread64(ptr+8))
		^wyhashmix(seed^wyhashp3,wyhashread64(ptr+16))
		^wyhashmix(seed^wyhashp4,wyhashread64(ptr+24)),len);
	case	1:	return	wyhashmix(
		wyhashmix(seed^wyhashp1,wyhashread08(ptr)),len);	
	case	2:	return	wyhashmix(
		wyhashmix(seed^wyhashp1,wyhashread16(ptr)),len);
	case	3:	return	wyhashmix(
		wyhashmix(seed^wyhashp1,(wyhashread16(ptr)<<8)|wyhashread08(ptr+2)),len);
	case	4:	return	wyhashmix(
		wyhashmix(seed^wyhashp1,wyhashread32(ptr)),len);
	case	5:	return	wyhashmix(
		wyhashmix(seed^wyhashp1,(wyhashread32(ptr)<<8)|wyhashread08(ptr+4)),len);
	case	6:	return	wyhashmix(
		wyhashmix(seed^wyhashp1,(wyhashread32(ptr)<<16)|wyhashread16(ptr+4)),len);
	case	7:	return	wyhashmix(
		wyhashmix(seed^wyhashp1,(wyhashread32(ptr)<<24)|(wyhashread16(ptr+4)<<8)|wyhashread08(ptr+6)),len);
	case	8:	return	wyhashmix(
		wyhashmix(seed^wyhashp1,wyhashread64(ptr)),len);
	case	9:	return	wyhashmix(
		wyhashmix(seed^wyhashp1,wyhashread64(ptr))
		^wyhashmix(seed^wyhashp2,wyhashread08(ptr+8)),len);	
	case	10:	return	wyhashmix(
		wyhashmix(seed^wyhashp1,wyhashread64(ptr))
		^wyhashmix(seed^wyhashp2,wyhashread16(ptr+8)),len);
	case	11:	return	wyhashmix(
		wyhashmix(seed^wyhashp1,wyhashread64(ptr))
		^wyhashmix(seed^wyhashp2,(wyhashread16(ptr+8)<<8)|wyhashread08(ptr+8+2)),len);
	case	12:	return	wyhashmix(
		wyhashmix(seed^wyhashp1,wyhashread64(ptr))
		^wyhashmix(seed^wyhashp2,wyhashread32(ptr+8)),len);
	case	13:	return	wyhashmix(
		wyhashmix(seed^wyhashp1,wyhashread64(ptr))
		^wyhashmix(seed^wyhashp2,(wyhashread32(ptr+8)<<8)|wyhashread08(ptr+8+4)),len);
	case	14:	return	wyhashmix(
		wyhashmix(seed^wyhashp1,wyhashread64(ptr))
		^wyhashmix(seed^wyhashp2,(wyhashread32(ptr+8)<<16)|wyhashread16(ptr+8+4)),len);
	case	15:	return	wyhashmix(
		wyhashmix(seed^wyhashp1,wyhashread64(ptr))
		^wyhashmix(seed^wyhashp2,(wyhashread32(ptr+8)<<24)|(wyhashread16(ptr+8+4)<<8)|wyhashread08(ptr+8+6)),len);
	case	16:	return	wyhashmix(
		wyhashmix(seed^wyhashp1,wyhashread64(ptr))
		^wyhashmix(seed^wyhashp2,wyhashread64(ptr+8)),len);
	case	17:	return	wyhashmix(
		wyhashmix(seed^wyhashp1,wyhashread64(ptr))
		^wyhashmix(seed^wyhashp2,wyhashread64(ptr+8))
		^wyhashmix(seed^wyhashp3,wyhashread08(ptr+16)),len);	
	case	18:	return	wyhashmix(
		wyhashmix(seed^wyhashp1,wyhashread64(ptr))
		^wyhashmix(seed^wyhashp2,wyhashread64(ptr+8))
		^wyhashmix(seed^wyhashp3,wyhashread16(ptr+16)),len);
	case	19:	return	wyhashmix(
		wyhashmix(seed^wyhashp1,wyhashread64(ptr))
		^wyhashmix(seed^wyhashp2,wyhashread64(ptr+8))
		^wyhashmix(seed^wyhashp3,(wyhashread16(ptr+16)<<8)|wyhashread08(ptr+16+2)),len);
	case	20:	return	wyhashmix(
		wyhashmix(seed^wyhashp1,wyhashread64(ptr))
		^wyhashmix(seed^wyhashp2,wyhashread64(ptr+8))
		^wyhashmix(seed^wyhashp3,wyhashread32(ptr+16)),len);
	case	21:	return	wyhashmix(
		wyhashmix(seed^wyhashp1,wyhashread64(ptr))
		^wyhashmix(seed^wyhashp2,wyhashread64(ptr+8))
		^wyhashmix(seed^wyhashp3,(wyhashread32(ptr+16)<<8)|wyhashread08(ptr+16+4)),len);
	case	22:	return	wyhashmix(
		wyhashmix(seed^wyhashp1,wyhashread64(ptr))
		^wyhashmix(seed^wyhashp2,wyhashread64(ptr+8))
		^wyhashmix(seed^wyhashp3,(wyhashread32(ptr+16)<<16)|wyhashread16(ptr+16+4)),len);
	case	23:	return	wyhashmix(
		wyhashmix(seed^wyhashp1,wyhashread64(ptr))
		^wyhashmix(seed^wyhashp2,wyhashread64(ptr+8))
		^wyhashmix(seed^wyhashp3,(wyhashread32(ptr+16)<<24)|(wyhashread16(ptr+16+4)<<8)|wyhashread08(ptr+16+6)),len);
	case	24:	return	wyhashmix(
		wyhashmix(seed^wyhashp1,wyhashread64(ptr))
		^wyhashmix(seed^wyhashp2,wyhashread64(ptr+8))
		^wyhashmix(seed^wyhashp3,wyhashread64(ptr+16)),len);
	case	25:	return	wyhashmix(
		wyhashmix(seed^wyhashp1,wyhashread64(ptr))
		^wyhashmix(seed^wyhashp2,wyhashread64(ptr+8))
		^wyhashmix(seed^wyhashp3,wyhashread64(ptr+16))
		^wyhashmix(seed^wyhashp4,wyhashread08(ptr+24)),len);	
	case	26:	return	wyhashmix(
		wyhashmix(seed^wyhashp1,wyhashread64(ptr))
		^wyhashmix(seed^wyhashp2,wyhashread64(ptr+8))
		^wyhashmix(seed^wyhashp3,wyhashread64(ptr+16))
		^wyhashmix(seed^wyhashp4,wyhashread16(ptr+24)),len);
	case	27:	return	wyhashmix(
		wyhashmix(seed^wyhashp1,wyhashread64(ptr))
		^wyhashmix(seed^wyhashp2,wyhashread64(ptr+8))
		^wyhashmix(seed^wyhashp3,wyhashread64(ptr+16))
		^wyhashmix(seed^wyhashp4,(wyhashread16(ptr+24)<<8)|wyhashread08(ptr+24+2)),len);
	case	28:	return	wyhashmix(
		wyhashmix(seed^wyhashp1,wyhashread64(ptr))
		^wyhashmix(seed^wyhashp2,wyhashread64(ptr+8))
		^wyhashmix(seed^wyhashp3,wyhashread64(ptr+16))
		^wyhashmix(seed^wyhashp4,wyhashread32(ptr+24)),len);
	case	29:	return	wyhashmix(
		wyhashmix(seed^wyhashp1,wyhashread64(ptr))
		^wyhashmix(seed^wyhashp2,wyhashread64(ptr+8))
		^wyhashmix(seed^wyhashp3,wyhashread64(ptr+16))
		^wyhashmix(seed^wyhashp4,(wyhashread32(ptr+24)<<8)|wyhashread08(ptr+24+4)),len);
	case	30:	return	wyhashmix(
		wyhashmix(seed^wyhashp1,wyhashread64(ptr))
		^wyhashmix(seed^wyhashp2,wyhashread64(ptr+8))
		^wyhashmix(seed^wyhashp3,wyhashread64(ptr+16))
		^wyhashmix(seed^wyhashp4,(wyhashread32(ptr+24)<<16)|wyhashread16(ptr+24+4)),len);
	case	31:	return	wyhashmix(
		wyhashmix(seed^wyhashp1,wyhashread64(ptr))
		^wyhashmix(seed^wyhashp2,wyhashread64(ptr+8))
		^wyhashmix(seed^wyhashp3,wyhashread64(ptr+16))
		^wyhashmix(seed^wyhashp4,(wyhashread32(ptr+24)<<24)|(wyhashread16(ptr+24+4)<<8)|wyhashread08(ptr+24+6)),len);
	}
	return	wyhashmix(seed,	len);
}
inline	unsigned int	wyhashmix32(unsigned int	A,	unsigned int	B){	
	unsigned long long	r=(unsigned long long)A*(unsigned long long)B;	return	(r>>32)^r;	
}
inline	unsigned int	wyhash32(unsigned int	A, unsigned int	B){	
	return	wyhashmix32(wyhashmix32(A^0x7b16763u,	B^0xe4f5a905u),	0x4a9e6939u);	
}
inline	unsigned long long	wyrngmix(unsigned long long	A,	unsigned long long	B){	
#ifdef __SIZEOF_INT128__
	__uint128_t	r=A;	r*=B;	return	(r>>64)^r;	
#else
	unsigned long long	ha=A>>32,	hb=B>>32,	la=(unsigned int)A,	lb=(unsigned int)B,	hi, lo;
	unsigned long long	rh=ha*hb,	rm0=ha*lb,	rm1=hb*la,	rl=la*lb,	t=rl+(rm0<<32),	c=t<rl;
	lo=t+(rm1<<32);	c+=lo<t;	hi=rh+(rm0>>32)+(rm1>>32)+c;
	return hi^lo;
#endif
}	
inline	unsigned long long	wyrng(unsigned long long *seed){	
	*seed+=wyhashp0;	return	wyrngmix(wyrngmix(*seed,	wyhashp1),	wyhashp2);
}
#endif
