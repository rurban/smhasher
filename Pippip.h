// FNV1A_Pippip_Yurii_OOO_128bit_AES_TriXZi_Mikayla: the 100% FREE lookuper, last update: 2025-Jul-17, Kaze (sanmayce@sanmayce.com). [
// This latest revision was written when Mikayla "saveafox" left this world.

// "There it now stands for ever. Black on white.
// I can't get away from it. Ahoy, Yorikke, ahoy, hoy, ho!
// Go to hell now if you wish. What do I care? It's all the same now to me.
// I am part of you now. Where you go I go, where you leave I leave, when you go to the devil I go. Married.
// Vanished from the living. Damned and doomed. Of me there is not left a breath in all the vast world.
// Ahoy, Yorikke! Ahoy, hoy, ho!
// I am not buried in the sea,
// The death ship is now part of me
// So far from sunny New Orleans
// So far from lovely Louisiana."
// /An excerpt from 'THE DEATH SHIP - THE STORY OF AN AMERICAN SAILOR' by B.TRAVEN/
// 
// "Walking home to our good old Yorikke, I could not help thinking of this beautiful ship, with a crew on board that had faces as if they were seeing ghosts by day and by night.
// Compared to that gilded Empress, the Yorikke was an honorable old lady with lavender sachets in her drawers.
// Yorikke did not pretend to anything she was not. She lived up to her looks. Honest to her lowest ribs and to the leaks in her bilge.
// Now, what is this? I find myself falling in love with that old jane.
// All right, I cannot pass by you, Yorikke; I have to tell you I love you. Honest, baby, I love you.
// I have six black finger-nails, and four black and green-blue nails on my toes, which you, honey, gave me when necking you.
// Grate-bars have crushed some of my toes. And each finger-nail has its own painful story to tell.
// My chest, my back, my arms, my legs are covered with scars of burns and scorchings.
// Each scar, when it was being created, caused me pains which I shall surely never forget.
// But every outcry of pain was a love-cry for you, honey.
// You are no hypocrite. Your heart does not bleed tears when you do not feel heart-aches deeply and truly.
// You do not dance on the water if you do not feel like being jolly and kicking chasers in the pants.
// Your heart never lies. It is fine and clean like polished gold. Never mind the rags, honey dear.
// When you laugh, your whole soul and all your body is laughing.
// And when you weep, sweety, then you weep so that even the reefs you pass feel like weeping with you.
// I never want to leave you again, honey. I mean it. Not for all the rich and elegant buckets in the world.
// I love you, my gypsy of the sea!"
// /An excerpt from 'THE DEATH SHIP - THE STORY OF AN AMERICAN SAILOR' by B.TRAVEN/
//
// Dedicated to Pippip, the main character in the 'Das Totenschiff' roman, actually the B.Traven himself, his real name was Hermann Albert Otto Maksymilian Feige.
// CAUTION: Add 8 more bytes to the buffer being hashed, usually malloc(...+8) - to prevent out of boundary reads!
// Many thanks go to Yurii 'Hordi' Hordiienko, he lessened with 3 instructions the original 'Pippip', thus:
// 

#include <stdlib.h>
#include <stdint.h>
#include <immintrin.h>
#define HashInBITS 32 // default 26 i.e. 2^26 i.e. 64MS(Mega Slots); slots contain 8bytes pointers or 512MB, because many netbooks have 512MB free (1GB in total)!
#define MASK ((1ULL << HashInBITS) - 1)
#define _PADr_KAZE(x, n) ( ((x) << (n))>>(n) )
#define _PAD_KAZE(x, n) ( ((x) << (n)) )

typedef struct {
	uint64_t low;
	uint64_t high;
} hashPippip128_t;

static inline uint64_t fold64(uint64_t A, uint64_t B) {
	#if defined(__GNUC__) || defined(__clang__)
		__uint128_t r = (__uint128_t)A * B;
		return (uint64_t)r ^ (uint64_t)(r >> 64);
	#else
		uint64_t hash64 = A ^ B;
		hash64 *= 1099511628211; //591798841;
		return hash64;
	#endif
}

static inline uint32_t fold32(uint32_t A, uint32_t B) {
	#if defined(__GNUC__) || defined(__clang__)
		uint64_t r = (uint64_t)A * (uint64_t)B;
		return (uint32_t)r ^ (uint32_t)(r >> 32);
	#else
		uint32_t hash32 = A ^ B;
		hash32 *= 591798841;
		return hash32;
	#endif
}

void FNV1A_Pippip_Yurii_OOO_128bit_AES_TriXZi_Mikayla (const char *str, size_t wrdlen, uint32_t seed, void *output) {
	__m128i chunkA;
	__m128i chunkB;
	__m128i stateMIX;
	uint64_t hashLH;
	uint64_t hashRH;
	stateMIX = _mm_set1_epi32( (uint32_t)wrdlen ^ seed );
	if (wrdlen > 8) {
		__m128i stateA = _mm_set_epi64x(0x6c62272e07bb0142, 0x9e3779b97f4a7c15);
		__m128i stateB = _mm_set_epi64x(0x6c62272e07bb0142, 0x9e3779b97f4a7c15);
		__m128i stateC = _mm_set_epi64x(0x6c62272e07bb0142, 0x9e3779b97f4a7c15);
		size_t Cycles, NDhead;
		if (wrdlen > 16) {
			Cycles = ((wrdlen - 1)>>5) + 1;
			NDhead = wrdlen - (Cycles<<4);
			if (Cycles & 1) {
				#pragma nounroll
				for(; Cycles--; str += 16) {
					//_mm_prefetch(str+512, _MM_HINT_T0);
					//_mm_prefetch(str+NDhead+512, _MM_HINT_T0);
					chunkA = _mm_loadu_si128((__m128i *)(str));
					stateA = _mm_aesenc_si128(stateA, chunkA);
					chunkB = _mm_loadu_si128((__m128i *)(str+NDhead));
					stateC = _mm_aesenc_si128(stateC, chunkA);
					stateB = _mm_aesenc_si128(stateB, chunkB);
					stateC = _mm_aesenc_si128(stateC, chunkB);
				}
			} else {
				Cycles = Cycles>>1;
				__m128i stateA2 = _mm_set_epi64x(0x6c62272e07bb0142, 0x9e3779b97f4a7c15);
				__m128i stateB2 = _mm_set_epi64x(0x6c62272e07bb0142, 0x9e3779b97f4a7c15);
				__m128i stateC2 = _mm_set_epi64x(0x6c62272e07bb0142, 0x9e3779b97f4a7c15);
				#pragma nounroll
				for(; Cycles--; str += 32) {
					_mm_prefetch(str+512, _MM_HINT_T0);
					_mm_prefetch(str+NDhead+512, _MM_HINT_T0);
					chunkA = _mm_loadu_si128((__m128i *)(str));
					__m128i chunkA2 = _mm_loadu_si128((__m128i *)(str+16));
					stateA = _mm_aesenc_si128(stateA, chunkA);
					stateA2 = _mm_aesenc_si128(stateA2, chunkA2);
					chunkB = _mm_loadu_si128((__m128i *)(str+NDhead));
					__m128i chunkB2 = _mm_loadu_si128((__m128i *)(str+NDhead+16));
					stateC = _mm_aesenc_si128(stateC, chunkA);
					stateB = _mm_aesenc_si128(stateB, chunkB);
					stateC = _mm_aesenc_si128(stateC, chunkB);
					stateC2 = _mm_aesenc_si128(stateC2, chunkA2);
					stateB2 = _mm_aesenc_si128(stateB2, chunkB2);
					stateC2 = _mm_aesenc_si128(stateC2, chunkB2);
				}
				stateMIX = _mm_aesenc_si128(stateMIX, stateA2);
				stateMIX = _mm_aesenc_si128(stateMIX, stateB2);
				stateMIX = _mm_aesenc_si128(stateMIX, stateC2);
			}
		} else { // 9..16
			NDhead = wrdlen - (1<<3);
			hashLH = (*(uint64_t *)(str));
			hashRH = (*(uint64_t *)(str+NDhead));
			chunkA = _mm_set_epi64x(hashLH, hashLH);
			stateA = _mm_aesenc_si128(stateA, chunkA);
			chunkB = _mm_set_epi64x(hashRH, hashRH);
			stateC = _mm_aesenc_si128(stateC, chunkA);
			stateB = _mm_aesenc_si128(stateB, chunkB);
			stateC = _mm_aesenc_si128(stateC, chunkB);
		}
		stateMIX = _mm_aesenc_si128(stateMIX, stateA);
		stateMIX = _mm_aesenc_si128(stateMIX, stateB);
		stateMIX = _mm_aesenc_si128(stateMIX, stateC);
	} else {
		hashLH = _PADr_KAZE(*(uint64_t *)(str+0), (8-wrdlen)<<3);
		hashRH = _PAD_KAZE(*(uint64_t *)(str+0), (8-wrdlen)<<3);
	    chunkA = _mm_set_epi64x(hashLH, hashLH);
	    chunkB = _mm_set_epi64x(hashRH, hashRH);
	    stateMIX = _mm_aesenc_si128(stateMIX, chunkA);
	    stateMIX = _mm_aesenc_si128(stateMIX, chunkB);
	}
    #ifdef eXdupe
	    _mm_storeu_si128((__m128i *)output, stateMIX); // For eXdupe
    #else
		uint64_t result[2];
		_mm_storeu_si128((__m128i *)result, stateMIX);
		uint64_t hash64 = fold64(result[0], result[1]);
		*(uint32_t*)output = (uint32_t)hash64 ^ wrdlen;
    #endif
}

//	Cycles = ((wrdlen - 1)>>5) + 1;
//	NDhead = wrdlen - (Cycles<<4);
// And some visualization for XMM-WORD:
/*
kl= 33..64 Cycles= (kl-1)/32+1=2; MARGINAL CASES:
                                 2nd head starts at 33-2*16=1 or:
                                        0123456789012345 0123456789012345 0
                                 Head1: [XMM-WORD      ] [XMM-WORD      ]
                                 Head2:  [XMM-WORD      ] [XMM-WORD      ]

                                 2nd head starts at 64-2*16=32 or:
                                        0123456789012345 0123456789012345 0123456789012345 0123456789012345
                                 Head1: [XMM-WORD      ] [XMM-WORD      ]
                                 Head2:                                   [XMM-WORD      ] [XMM-WORD      ]

kl=65..96 Cycles= (kl-1)/32+1=3; MARGINAL CASES:
                                 2nd head starts at 65-3*16=17 or:
                                        0123456789012345 0123456789012345 0123456789012345 0123456789012345 0
                                 Head1: [XMM-WORD      ] [XMM-WORD      ] [XMM-WORD      ]
                                 Head2:                   [XMM-WORD      ] [XMM-WORD      ] [XMM-WORD      ]

                                 2nd head starts at 96-3*16=48 or:
                                        0123456789012345 0123456789012345 0123456789012345 0123456789012345 0123456789012345 0123456789012345
                                 Head1: [XMM-WORD      ] [XMM-WORD      ] [XMM-WORD      ]
                                 Head2:                                                    [XMM-WORD      ] [XMM-WORD      ] [XMM-WORD      ]
*/

// And some visualization for Q-WORD:
/*
kl= 9..16 Cycles= (kl-1)/16+1=1; MARGINAL CASES:
                                 2nd head starts at 9-1*8=1 or:
                                        012345678
                                 Head1: [Q-WORD]
                                 Head2:  [Q-WORD]

                                 2nd head starts at 16-1*8=8 or:
                                        0123456789012345
                                 Head1: [Q-WORD]
                                 Head2:         [Q-WORD]

kl=17..24 Cycles= (kl-1)/16+1=2; MARGINAL CASES:
                                 2nd head starts at 17-2*8=1 or:
                                        01234567890123456
                                 Head1: [Q-WORD][Q-WORD]
                                 Head2:  [Q-WORD][Q-WORD]

                                 2nd head starts at 24-2*8=8 or:
                                        012345678901234567890123
                                 Head1: [Q-WORD][Q-WORD]
                                 Head2:         [Q-WORD][Q-WORD]

kl=25..32 Cycles= (kl-1)/16+1=2; MARGINAL CASES:
                                 2nd head starts at 25-2*8=9 or:
                                        0123456789012345678901234
                                 Head1: [Q-WORD][Q-WORD]
                                 Head2:          [Q-WORD][Q-WORD]

                                 2nd head starts at 32-2*8=16 or:
                                        01234567890123456789012345678901
                                 Head1: [Q-WORD][Q-WORD]
                                 Head2:                 [Q-WORD][Q-WORD]

kl=33..40 Cycles= (kl-1)/16+1=3; MARGINAL CASES:
                                 2nd head starts at 33-3*8=9 or:
                                        012345678901234567890123456789012
                                 Head1: [Q-WORD][Q-WORD][Q-WORD]
                                 Head2:          [Q-WORD][Q-WORD][Q-WORD]

                                 2nd head starts at 40-3*8=16 or:
                                        0123456789012345678901234567890123456789
                                 Head1: [Q-WORD][Q-WORD][Q-WORD]
                                 Head2:                 [Q-WORD][Q-WORD][Q-WORD]

kl=41..48 Cycles= (kl-1)/16+1=3; MARGINAL CASES:
                                 2nd head starts at 41-3*8=17 or:
                                        01234567890123456789012345678901234567890
                                 Head1: [Q-WORD][Q-WORD][Q-WORD]
                                 Head2:                  [Q-WORD][Q-WORD][Q-WORD]

                                 2nd head starts at 48-3*8=24 or:
                                        012345678901234567890123456789012345678901234567
                                 Head1: [Q-WORD][Q-WORD][Q-WORD]
                                 Head2:                         [Q-WORD][Q-WORD][Q-WORD]
*/

// The more the merrier, therefore I added the 10,000 GitHub stars performer xxhash also:
// https://github.com/Cyan4973/xxHash/issues/1029
// 
// Pippip is not an extremely fast hash, it is the spirit of the author materialized disregarding anything outside the "staying true to oneself", or as one bona fide man Otto/Pippip once said:
// 
// Translate as verbatim as possible:
// In 1926, Traven wrote that the only biography of a writer should be his
// works: «Die Biographie eines schöpferischen Menschen ist ganz und gar unwichtig.
// Wenn der Mensch in seinen Werken nicht zu erkennen ist, dann ist entweder der
// Mensch nichts wert oder seine Werke sind nichts wert. Darum sollte der schöpferische
// Mensch keine andere Biographie haben als seine Werke» (Hauschild, B. Traven: Die
// unbekannten Jahre, op. cit., p. 31.)
// 
// In 1926, Traven wrote that the only biography of a writer should be his works:
// “The biography of a creative person is completely and utterly unimportant.
// If the person is not recognizable in his works, then either the person is worthless or his works are worthless.
// Therefore, the creative person should have no other biography than his works” (Hauschild, B. Traven: Die unbekannten Jahre, op. cit., p. 31.) 

// https://godbolt.org/
// x86-64 clang 20.1.0
// -O3 -maes -mbmi2
/*
...
*/

// https://godbolt.org/
// x86-64 gcc 14.2
// -O3 -maes -mbmi2
/*
...
*/

