/* -------------------------------------------------------------------------------
 * Copyright (c) 2014, Dmytro Ivanchykhin, Sergey Ignatchenko, Daniel Lemire
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met: 
 * 
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer. 
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution. 
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * -------------------------------------------------------------------------------
 * 
 * PMP+-Multilinear hash family implementation
 * 
 * v.1.00    Apr-14-2014    Initial release
 * 
 * -------------------------------------------------------------------------------*/

// PMP_Multilinear.cpp: coefficients for an instance of a 32-bit hash function of PMP+-Multilinear hash family

const char* __MULTILINEARPRIMESTRINGHASHFUNCTOR_CPP__REVISION_(){return "$Rev: 367 $";} // for automated version information update; could be removed, if not desired

#include "PMP_Multilinear_common.h"


const random_data_for_MPSHF rd_for_MPSHF[ PMPML_LEVELS ] =
{
#if ( PMPML_LEVELS > 0 )
        // Level 0
        {
                UINT64_C( 0xb5ae35fa ),
				UINT64_C( 0x45dfdab824 ),
#ifdef PMPML_USE_SSE
#if PMPML_USE_SSE_SIZE == 128
#elif PMPML_USE_SSE_SIZE == 256
				{UINT64_C( 0 ), UINT64_C( 0 )}, // dummy
#else
#error unsupported PMPML_USE_SSE_SIZE value
#endif
#endif
                {
#if ( PMPML_CHUNK_SIZE > 0 )
                        0x801841bb,
#endif // ( PMPML_CHUNK_SIZE > 0 )
#if ( PMPML_CHUNK_SIZE > 1 )
                        0x5ef2b6fc,
#endif // ( PMPML_CHUNK_SIZE > 1 )
#if ( PMPML_CHUNK_SIZE > 2 )
                        0xcc5a24e2,
                        0x1b6c5dd5,
#endif // ( PMPML_CHUNK_SIZE > 2 )
#if ( PMPML_CHUNK_SIZE > 4 )
                        0xeb07483b,
                        0xef894c5b,
                        0x02213973,
                        0x2d34d946,
#endif // ( PMPML_CHUNK_SIZE > 4 )
#if ( PMPML_CHUNK_SIZE > 8 )
                        0x11af1a4d,
                        0xd0a96734,
                        0xf39454a6,
                        0x58574f85,
                        0x08bc3780,
                        0x3d5e4d6e,
                        0x72302724,
                        0x89d2f7d4,
#endif // ( PMPML_CHUNK_SIZE > 8 )
#if ( PMPML_CHUNK_SIZE > 16 )
                        0x97d9459e,
                        0xba75d6d3,
                        0x69efa09d,
                        0x56f8f06a,
                        0x7345e990,
                        0x8ac230e9,
                        0xd21f3d0c,
                        0x3fffba8a,
                        0xd6dd6772,
                        0xd8c69c6b,
                        0x77a68e52,
                        0xde17020d,
                        0xf969ac45,
                        0x4ec4e3fb,
                        0x66e1eaae,
                        0x8c3e2c33,
#endif // ( PMPML_CHUNK_SIZE > 16 )
#if ( PMPML_CHUNK_SIZE > 32 )
                        0xd031a884,
                        0x5942d1f7,
                        0x355157a1,
                        0x79e517ce,
                        0x6f6e67c9,
                        0xdbeb2ce9,
                        0xaf4c5195,
                        0x1d72b4ce,
                        0x2214d9f3,
                        0xdab836c3,
                        0x94a54c8d,
                        0xa259587e,
                        0x8e5a6bd6,
                        0x75d23672,
                        0xf08fcd74,
                        0x59297837,
                        0xc1f093c7,
                        0xb1e14572,
                        0x84e25787,
                        0xfa18cbdd,
                        0xc0a8efe1,
                        0x8f746f29,
                        0xd1dfea17,
                        0xd17d1d65,
                        0x99c0334e,
                        0xc200ce59,
                        0xbac039b7,
                        0xaa8da145,
                        0x91787415,
                        0x7478d0e6,
                        0xd4fcb135,
                        0x76c4ce66,
#endif // ( PMPML_CHUNK_SIZE > 32 )
#if ( PMPML_CHUNK_SIZE > 64 )
                        0xdf1d9e9b,
                        0xe6a6640f,
                        0x94dd9b8e,
                        0x7f530896,
                        0xd5a76dff,
                        0xda99ae01,
                        0x2830dcad,
                        0x18421917,
                        0xc98aeb4f,
                        0x0048fdda,
                        0xd5ae8cba,
                        0xe9d27a3f,
                        0xc51ba04d,
                        0x8f1403e7,
                        0x2cbc94bd,
                        0x2c47c847,
                        0xbf127785,
                        0x54d2a15b,
                        0x6a818544,
                        0x993ca700,
                        0x31f529ed,
                        0x4cf30c4c,
                        0x386af44a,
                        0x1378d4c0,
                        0x3c40ac83,
                        0x3d27aaa4,
                        0x9b1c685e,
                        0x61dbbba6,
                        0xe5fbbd87,
                        0x800c57fd,
                        0xccd49830,
                        0x1ee12d69,
                        0x84868385,
                        0xbaf5679f,
                        0xd0417045,
                        0x4f5c30f0,
                        0x70558f08,
                        0x7c1e281d,
                        0xfe17014e,
                        0x56404d7c,
                        0x77dcfdd3,
                        0xf0d53161,
                        0xf9914927,
                        0x69bc0362,
                        0x609759cb,
                        0xfc9afc53,
                        0xc5f28ba8,
                        0x9cbe677d,
                        0x8b8311e5,
                        0x40a1fbde,
                        0x500ef7fc,
                        0xd51ceaa4,
                        0x2c666e8f,
                        0xbf81662b,
                        0xa0922fe9,
                        0x65a75374,
                        0xc744184e,
                        0x1fad7a1a,
                        0xbc3678c2,
                        0xde23fbbc,
                        0x0403fd45,
                        0x69cd23ae,
                        0xf3dc2f19,
                        0x31416e93,
#endif // ( PMPML_CHUNK_SIZE > 64 )
#if ( PMPML_CHUNK_SIZE > 128 )
#error VALUES NOT GENERATED for PMPML_CHUNK_SIZE > 128
#endif // ( PMPML_CHUNK_SIZE > 128 )

                },
        },
#endif // ( PMPML_LEVELS > 0 )

#if ( PMPML_LEVELS > 1 )
        // Level 0
        {
                UINT64_C(0xc3dbb82),
				UINT64_C( 0x3c33d12213 ),
#ifdef PMPML_USE_SSE
#if PMPML_USE_SSE_SIZE == 128
#elif PMPML_USE_SSE_SIZE == 256
				{UINT64_C( 0 ), UINT64_C( 0 )}, // dummy
#else
#error unsupported PMPML_USE_SSE_SIZE value
#endif
#endif
                {
#if ( PMPML_CHUNK_SIZE > 0 )
                        0xd233467b,
#endif // ( PMPML_CHUNK_SIZE > 0 )
#if ( PMPML_CHUNK_SIZE > 1 )
                        0x72a70d41,
#endif // ( PMPML_CHUNK_SIZE > 1 )
#if ( PMPML_CHUNK_SIZE > 2 )
                        0x8bd6cb67,
                        0x2e954d02,
#endif // ( PMPML_CHUNK_SIZE > 2 )
#if ( PMPML_CHUNK_SIZE > 4 )
                        0x08142b46,
                        0xb9613249,
                        0x8136a81d,
                        0x3cdab6cf,
#endif // ( PMPML_CHUNK_SIZE > 4 )
#if ( PMPML_CHUNK_SIZE > 8 )
                        0x70433dfc,
                        0x984d385b,
                        0x66f13c63,
                        0x392a028c,
                        0x84b10a87,
                        0xb54b7873,
                        0x7af58609,
                        0xbe835997,
#endif // ( PMPML_CHUNK_SIZE > 8 )
#if ( PMPML_CHUNK_SIZE > 16 )
                        0x09878350,
                        0x2702ed23,
                        0x940ffe4b,
                        0x073982e4,
                        0x4b565486,
                        0xc1872a1b,
                        0xcb9af7a0,
                        0xd8a84f81,
                        0xd8234048,
                        0x3d9a44b4,
                        0xfcecd1d5,
                        0x114fe193,
                        0x7e848584,
                        0x0082760d,
                        0x0ede3da7,
                        0x0040762c,
#endif // ( PMPML_CHUNK_SIZE > 16 )
#if ( PMPML_CHUNK_SIZE > 32 )
                        0xe522397a,
                        0x44ec8715,
                        0x422bc161,
                        0x0764c174,
                        0x3c511482,
                        0xd7dea424,
                        0xa12ec3c0,
                        0x66d33ec0,
                        0x0aaa55ce,
                        0x65f93ec0,
                        0xadaaaf7f,
                        0x647e772d,
                        0xa6b0a4fa,
                        0x88a72a0d,
                        0x1cfa03b4,
                        0x4f28c0c6,
                        0xa7c64b56,
                        0xedd8af5e,
                        0xa47e7242,
                        0x99f8d210,
                        0x8ad70f5f,
                        0xa8e3cdfb,
                        0x0a1db865,
                        0x56b2e1b0,
                        0x0dd7b307,
                        0x564a191f,
                        0xca38b54f,
                        0x61567b67,
                        0xd50c9644,
                        0x7671637e,
                        0x92d511cc,
                        0x25057afc,
#endif // ( PMPML_CHUNK_SIZE > 32 )
#if ( PMPML_CHUNK_SIZE > 64 )
                        0xd286cba4,
                        0x71f8dda9,
                        0x2ad9996c,
                        0x75ad65f0,
                        0x9418c0e9,
                        0xe6d0066b,
                        0xf1d15419,
                        0x264afe8b,
                        0x98c932e2,
                        0x3a6d5f8d,
                        0x289a7d0c,
                        0x3d18290d,
                        0xb9ecee8d,
                        0xdff7a79b,
                        0x7ecc3cde,
                        0x583e06a0,
                        0x8e29d297,
                        0xdc8650cb,
                        0x30f7861d,
                        0xf2de5cf9,
                        0x924dc8bc,
                        0x5afb46e9,
                        0xb997b1d9,
                        0x463d84a2,
                        0xfb8e2e7e,
                        0x043418b8,
                        0xa94e6a05,
                        0xae5c1efa,
                        0x7c7e4583,
                        0xcb6755ac,
                        0xf3359dba,
                        0xf05fdf94,
                        0x79db25ea,
                        0xed490569,
                        0x993d8da0,
                        0x6593ce5a,
                        0x03e3ed39,
                        0x044f74a3,
                        0x84777814,
                        0xcb2848d7,
                        0x41881b64,
                        0xf52d206e,
                        0x1fb1ebaf,
                        0x07a3d4b3,
                        0x63a5924f,
                        0x35c21005,
                        0xc981c63c,
                        0x9e3fdbaa,
                        0x89b64b0d,
                        0x0f2aba74,
                        0x512f3cfe,
                        0xb053e5d0,
                        0x59a69c4a,
                        0x400c442f,
                        0x28afebd0,
                        0x4540c190,
                        0xc7f5e757,
                        0x7d40152b,
                        0x321fa235,
                        0xb6309529,
                        0x021c71e1,
                        0x7474f524,
                        0xc4f2e22e,
                        0x778b9371,
#endif // ( PMPML_CHUNK_SIZE > 64 )
#if ( PMPML_CHUNK_SIZE > 128 )
#error VALUES NOT GENERATED for PMPML_CHUNK_SIZE > 128
#endif // ( PMPML_CHUNK_SIZE > 128 )

                },
        },
#endif // ( PMPML_LEVELS > 1 )

#if ( PMPML_LEVELS > 2 )
        // Level 0
        {
                UINT64_C(0x4ae2b467),
				UINT64_C( 0x41b6700d41 ),
#ifdef PMPML_USE_SSE
#if PMPML_USE_SSE_SIZE == 128
#elif PMPML_USE_SSE_SIZE == 256
				{UINT64_C( 0 ), UINT64_C( 0 )}, // dummy
#else
#error unsupported PMPML_USE_SSE_SIZE value
#endif
#endif
                {
#if ( PMPML_CHUNK_SIZE > 0 )
                        0xf8898c22,
#endif // ( PMPML_CHUNK_SIZE > 0 )
#if ( PMPML_CHUNK_SIZE > 1 )
                        0x863868bc,
#endif // ( PMPML_CHUNK_SIZE > 1 )
#if ( PMPML_CHUNK_SIZE > 2 )
                        0xd35470e9,
                        0x58d21ad6,
#endif // ( PMPML_CHUNK_SIZE > 2 )
#if ( PMPML_CHUNK_SIZE > 4 )
                        0xa2fce702,
                        0xe4f58530,
                        0x0225c8a9,
                        0x9b29b401,
#endif // ( PMPML_CHUNK_SIZE > 4 )
#if ( PMPML_CHUNK_SIZE > 8 )
                        0xf4f6d3eb,
                        0xf751b2ce,
                        0x2afa3d7a,
                        0xc1edf3e9,
                        0x4c57e2d1,
                        0xc2ef970d,
                        0x8a70aa25,
                        0x887d0102,
#endif // ( PMPML_CHUNK_SIZE > 8 )
#if ( PMPML_CHUNK_SIZE > 16 )
                        0xcc09e169,
                        0xeb5b75e2,
                        0x760b047e,
                        0xa2d21874,
                        0xc2bf310a,
                        0x8f030e02,
                        0x4b97fa22,
                        0x6a413ddb,
                        0x708062b4,
                        0x58cc67d3,
                        0x52459895,
                        0x78d345e3,
                        0x2b7a9415,
                        0xbaf4d1fe,
                        0x83462969,
                        0x923fa257,
#endif // ( PMPML_CHUNK_SIZE > 16 )
#if ( PMPML_CHUNK_SIZE > 32 )
                        0x91617494,
                        0xedf8d2f5,
                        0xc3d41302,
                        0xdf1934ff,
                        0x78a27863,
                        0xe7bf06a2,
                        0xc21b996d,
                        0x1e72411e,
                        0x98da3053,
                        0x0c2195ad,
                        0xf984dd09,
                        0x4b30dac8,
                        0xf3a03a7a,
                        0xee6540ec,
                        0x966dffb7,
                        0xb463fdbe,
                        0xbec26037,
                        0xcc9adad0,
                        0xdb71b8ef,
                        0x57341ca0,
                        0xa742ec7b,
                        0xe86321e9,
                        0x7a9d9f15,
                        0x7809e2a6,
                        0x2cb6a0a0,
                        0x344756d0,
                        0x6e8e8c88,
                        0x7ecf3ff7,
                        0x129d18a0,
                        0x0965dc6a,
                        0xf6a2cad1,
                        0xd938681b,
#endif // ( PMPML_CHUNK_SIZE > 32 )
#if ( PMPML_CHUNK_SIZE > 64 )
                        0xa1d07081,
                        0x4253df74,
                        0x774a5200,
                        0x59e1356d,
                        0x7aad36b5,
                        0x7dd6414a,
                        0x4700a70e,
                        0xd0da811c,
                        0x1fd2a8b8,
                        0x1dee15ad,
                        0x7f15ae5a,
                        0xc1f74f27,
                        0xfd8bfb7f,
                        0x16815bb9,
                        0x64d29007,
                        0xc8919e9f,
                        0x0b8c7e82,
                        0xfd5e92c2,
                        0x6e073fb7,
                        0xd52df9c2,
                        0x0c5c519d,
                        0x3ad86cb4,
                        0xfde300c8,
                        0x674c4dac,
                        0x54899a0a,
                        0xbf9a9be5,
                        0xe198c073,
                        0x6025af27,
                        0x433bac50,
                        0x669d3281,
                        0xee3838b3,
                        0x0df3a048,
                        0x2d0de6cd,
                        0xd289c8eb,
                        0x6b1c9eb1,
                        0x1634922b,
                        0x61917d41,
                        0x8b8bdeec,
                        0x12b73dcf,
                        0x96353517,
                        0x20e29858,
                        0xecc04cb9,
                        0x0074a2ca,
                        0x58a0f1ba,
                        0x6ed4e71f,
                        0x063fec8e,
                        0xc5bc30c2,
                        0x77af6d46,
                        0x078a6a93,
                        0x8c8da7a2,
                        0x1d02b1cc,
                        0x96b659f9,
                        0x8d8b4fbd,
                        0x521b2964,
                        0x990235f7,
                        0x55c63419,
                        0x1ad869a5,
                        0x51987dbd,
                        0x99e7a3ff,
                        0xf584d99a,
                        0xc11c3506,
                        0xb1adca80,
                        0x55007e41,
                        0x09efa72b,
#endif // ( PMPML_CHUNK_SIZE > 64 )
#if ( PMPML_CHUNK_SIZE > 128 )
#error VALUES NOT GENERATED for PMPML_CHUNK_SIZE > 128
#endif // ( PMPML_CHUNK_SIZE > 128 )

                },
        },
#endif // ( PMPML_LEVELS > 2 )

#if ( PMPML_LEVELS > 3 )
        // Level 0
        {
                UINT64_C(0xae82fd43),
				UINT64_C( 0x4358e7ef21 ),
#ifdef PMPML_USE_SSE
#if PMPML_USE_SSE_SIZE == 128
#elif PMPML_USE_SSE_SIZE == 256
				{UINT64_C( 0 ), UINT64_C( 0 )}, // dummy
#else
#error unsupported PMPML_USE_SSE_SIZE value
#endif
#endif
                {
#if ( PMPML_CHUNK_SIZE > 0 )
                        0x9e6c8a0f,
#endif // ( PMPML_CHUNK_SIZE > 0 )
#if ( PMPML_CHUNK_SIZE > 1 )
                        0x9107b963,
#endif // ( PMPML_CHUNK_SIZE > 1 )
#if ( PMPML_CHUNK_SIZE > 2 )
                        0xdc39a0eb,
                        0x9fb2328d,
#endif // ( PMPML_CHUNK_SIZE > 2 )
#if ( PMPML_CHUNK_SIZE > 4 )
                        0xd4f03812,
                        0xce7ff238,
                        0x99710f09,
                        0x90b5a0ba,
#endif // ( PMPML_CHUNK_SIZE > 4 )
#if ( PMPML_CHUNK_SIZE > 8 )
                        0x53cb9654,
                        0xdca51386,
                        0x5a03c91d,
                        0x542e4280,
                        0x92d368ff,
                        0x6769cd0b,
                        0xacad27d0,
                        0x3947f94b,
#endif // ( PMPML_CHUNK_SIZE > 8 )
#if ( PMPML_CHUNK_SIZE > 16 )
                        0xf33a3265,
                        0x2f298054,
                        0x5094d047,
                        0x962591a6,
                        0x89c1de39,
                        0x0ef43de4,
                        0xe87f5576,
                        0xb342b1dc,
                        0xffb893e3,
                        0x08a96d7d,
                        0xe1023f0d,
                        0x054ac7ea,
                        0xeb0a8934,
                        0xe1558e68,
                        0xce76025c,
                        0x47c0a61f,
#endif // ( PMPML_CHUNK_SIZE > 16 )
#if ( PMPML_CHUNK_SIZE > 32 )
                        0x9d476622,
                        0xee83acc6,
                        0x5fb7a3fd,
                        0xa1798b06,
                        0x97cfbc96,
                        0x341dc4f8,
                        0x079d4d68,
                        0x85811d0d,
                        0xe81cd930,
                        0x83f55707,
                        0x7cd3da51,
                        0xe504fcf6,
                        0x5afed439,
                        0x35677002,
                        0x40d755aa,
                        0xcea876c6,
                        0x1c8a9953,
                        0x9a7d47c1,
                        0x9343c019,
                        0x60ffafe4,
                        0x7c12e1c5,
                        0xa64b2499,
                        0x9e13587f,
                        0x6e690d98,
                        0x24a0dcfe,
                        0xfc4c35a6,
                        0x66eca52a,
                        0xe9e0315f,
                        0xa208fe48,
                        0x16d7bd81,
                        0xd5c9b0fb,
                        0xe7337bf9,
#endif // ( PMPML_CHUNK_SIZE > 32 )
#if ( PMPML_CHUNK_SIZE > 64 )
                        0x2d3ad9dc,
                        0x6924c3f3,
                        0x8e7174f8,
                        0x01f7e499,
                        0x2e3edfb8,
                        0x8dfe2b6a,
                        0x40f43c09,
                        0xcf51dafc,
                        0xafe98c70,
                        0x31b3d859,
                        0x07f28e34,
                        0x6527d100,
                        0x5274484e,
                        0x92fa82fe,
                        0xf059d18a,
                        0x55e4c67c,
                        0x51e5d061,
                        0xaa4408e9,
                        0xbd7463cc,
                        0xb587505f,
                        0xfc88d42e,
                        0x70b3e921,
                        0xeabb6770,
                        0xfb3a060b,
                        0xd675527a,
                        0xb8d6153f,
                        0xbd1763ad,
                        0x6f1a2573,
                        0xf96490be,
                        0xce99095f,
                        0x966d1090,
                        0x65e2a371,
                        0x3a81e7f8,
                        0x769315db,
                        0xaa973861,
                        0x8d6d798c,
                        0xa935a7ae,
                        0x194de67a,
                        0x402f5da2,
                        0x58a7f932,
                        0xa1eb519c,
                        0x65125c5b,
                        0x961b4b6c,
                        0x518c8dab,
                        0x47233e7f,
                        0x1b19109b,
                        0x46a1b3c1,
                        0x5dc3dd6c,
                        0x709b63af,
                        0x3e43e71c,
                        0x7b997703,
                        0xa2259145,
                        0x81f87a1c,
                        0xa6c8a082,
                        0xa12ef053,
                        0x412e7f0e,
                        0x29bef6e8,
                        0xcc8fca68,
                        0xf521167a,
                        0x203c0e84,
                        0xe92d5cd7,
                        0x9589c2d1,
                        0x208e2f28,
                        0x906bd537,
#endif // ( PMPML_CHUNK_SIZE > 64 )
#if ( PMPML_CHUNK_SIZE > 128 )
#error VALUES NOT GENERATED for PMPML_CHUNK_SIZE > 128
#endif // ( PMPML_CHUNK_SIZE > 128 )

                },
        },
#endif // ( PMPML_LEVELS > 3 )

#if ( PMPML_LEVELS > 4 )
        // Level 0
        {
                UINT64_C(0xc3b9656e ),
				UINT64_C( 0x3f969c7ed3 ),
#ifdef PMPML_USE_SSE
#if PMPML_USE_SSE_SIZE == 128
#elif PMPML_USE_SSE_SIZE == 256
				{UINT64_C( 0 ), UINT64_C( 0 )}, // dummy
#else
#error unsupported PMPML_USE_SSE_SIZE value
#endif
#endif
                {
#if ( PMPML_CHUNK_SIZE > 0 )
                        0x60731d8f,
#endif // ( PMPML_CHUNK_SIZE > 0 )
#if ( PMPML_CHUNK_SIZE > 1 )
                        0x2e17b1b7,
#endif // ( PMPML_CHUNK_SIZE > 1 )
#if ( PMPML_CHUNK_SIZE > 2 )
                        0xb808f3c7,
                        0xf20f223c,
#endif // ( PMPML_CHUNK_SIZE > 2 )
#if ( PMPML_CHUNK_SIZE > 4 )
                        0xb964bc3c,
                        0xaa61a231,
                        0x3d84cd54,
                        0x94f006d6,
#endif // ( PMPML_CHUNK_SIZE > 4 )
#if ( PMPML_CHUNK_SIZE > 8 )
                        0x684e8f60,
                        0xb64adf58,
                        0x7033ff6c,
                        0x01ea1b40,
                        0xbcaf2776,
                        0x70250562,
                        0x342ec517,
                        0x1e280438,
#endif // ( PMPML_CHUNK_SIZE > 8 )
#if ( PMPML_CHUNK_SIZE > 16 )
                        0xaeaa96ba,
                        0x802391c2,
                        0x35a7f213,
                        0x8d0f57aa,
                        0xf8a1153b,
                        0x917a692a,
                        0xbac0385c,
                        0x6dc2f7dd,
                        0xc573a21b,
                        0x0469558c,
                        0xf206c551,
                        0xfe683c17,
                        0x54d0c3bc,
                        0x80734381,
                        0xc4eef75c,
                        0x22648b9e,
#endif // ( PMPML_CHUNK_SIZE > 16 )
#if ( PMPML_CHUNK_SIZE > 32 )
                        0xede23e78,
                        0x8823f123,
                        0xd687c6a7,
                        0x85b6752b,
                        0xb8cf5160,
                        0x8109a1c8,
                        0x1b4c7ceb,
                        0xaa8b17a6,
                        0xeda3fcbf,
                        0xb6d65214,
                        0xe6171214,
                        0x98f4ee28,
                        0xc1ac9d91,
                        0x0810d22e,
                        0x1ccec281,
                        0xd1911b8a,
                        0x272b7696,
                        0x860fc01d,
                        0x903c0029,
                        0xf3308e35,
                        0x8c2021ef,
                        0x52ebae93,
                        0x6ece3f90,
                        0x2d01f59f,
                        0x15cf87c9,
                        0x79c113fd,
                        0xcee953e9,
                        0x6152456a,
                        0x82d25ea1,
                        0x743316c4,
                        0x351f50d1,
                        0x06e3708f,
#endif // ( PMPML_CHUNK_SIZE > 32 )
#if ( PMPML_CHUNK_SIZE > 64 )
                        0x45060a80,
                        0x4c13c59a,
                        0x0a737387,
                        0x3eaa3672,
                        0xe5176942,
                        0x8431098a,
                        0x0cd55f05,
                        0x9d5c2eda,
                        0x6df6d514,
                        0x41a412ea,
                        0x67606dd0,
                        0xdec02567,
                        0xaebddaad,
                        0xf48d85d8,
                        0x7f41af4b,
                        0xbb8b03b7,
                        0x29bb612f,
                        0xc96546c9,
                        0xb04dfcc9,
                        0x2ee6c830,
                        0xafb0bc9e,
                        0x08e0ef18,
                        0xea81d1fc,
                        0xa58be897,
                        0xee996482,
                        0xb7ee4493,
                        0x0c561cd5,
                        0x7695207b,
                        0x763a34f3,
                        0x7093196a,
                        0xecf527bd,
                        0xb3037632,
                        0x40fdbc46,
                        0x72a3f33d,
                        0xb09e2e73,
                        0x1b41ab32,
                        0x32c280f4,
                        0x865d6444,
                        0xa998ef38,
                        0xe1f097de,
                        0x5f6c5d4f,
                        0xfebdf03d,
                        0xc569ef53,
                        0xec6decf1,
                        0x03de6003,
                        0x0e3063d7,
                        0x8dd9c0a0,
                        0x062c97a4,
                        0xa45c835e,
                        0xd167187d,
                        0xfe55e66e,
                        0x6b24b6df,
                        0x572c5189,
                        0x30c18b20,
                        0x3c0346f8,
                        0x5982a13e,
                        0xbf491b0f,
                        0x248df32c,
                        0x6f572546,
                        0x51296aff,
                        0x1a8c0702,
                        0x94a21284,
                        0x371e69c8,
                        0x2298720e,
#endif // ( PMPML_CHUNK_SIZE > 64 )
#if ( PMPML_CHUNK_SIZE > 128 )
#error VALUES NOT GENERATED for PMPML_CHUNK_SIZE > 128
#endif // ( PMPML_CHUNK_SIZE > 128 )

                },
        },
#endif // ( PMPML_LEVELS > 4 )

#if ( PMPML_LEVELS > 5 )
        // Level 0
        {
                UINT64_C(0xe3c9939c ),
				UINT64_C( 0x3d848fecbb ),
#ifdef PMPML_USE_SSE
#if PMPML_USE_SSE_SIZE == 128
#elif PMPML_USE_SSE_SIZE == 256
				{UINT64_C( 0 ), UINT64_C( 0 )}, // dummy
#else
#error unsupported PMPML_USE_SSE_SIZE value
#endif
#endif
                {
#if ( PMPML_CHUNK_SIZE > 0 )
                        0x78bb7f84,
#endif // ( PMPML_CHUNK_SIZE > 0 )
#if ( PMPML_CHUNK_SIZE > 1 )
                        0xc6a18ac7,
#endif // ( PMPML_CHUNK_SIZE > 1 )
#if ( PMPML_CHUNK_SIZE > 2 )
                        0xeb321f90,
                        0x35d4f871,
#endif // ( PMPML_CHUNK_SIZE > 2 )
#if ( PMPML_CHUNK_SIZE > 4 )
                        0x61a5f4a7,
                        0x6d591ba2,
                        0x7f93ad57,
                        0x96841919,
#endif // ( PMPML_CHUNK_SIZE > 4 )
#if ( PMPML_CHUNK_SIZE > 8 )
                        0xea7890a9,
                        0x0fa2f69c,
                        0x1866af58,
                        0x7f257346,
                        0xdcc51cd9,
                        0x92e78656,
                        0xc4628292,
                        0x42e01b49,
#endif // ( PMPML_CHUNK_SIZE > 8 )
#if ( PMPML_CHUNK_SIZE > 16 )
                        0x40541662,
                        0x37af7888,
                        0x4faa39af,
                        0xa3207d98,
                        0x63750fda,
                        0x2767c143,
                        0xf11a2916,
                        0x618ceb9b,
                        0x9d684ce0,
                        0x69088033,
                        0x1ab5a1c7,
                        0x0f0a4f86,
                        0x4e49f893,
                        0x0ca32464,
                        0x90a7c38e,
                        0x5a0aded0,
#endif // ( PMPML_CHUNK_SIZE > 16 )
#if ( PMPML_CHUNK_SIZE > 32 )
                        0x2dae1926,
                        0x0d935a0e,
                        0xde592a69,
                        0x085299b2,
                        0x4977a3a0,
                        0x7e82d9bc,
                        0x399e6a95,
                        0xdb9f1b90,
                        0xe1dfe431,
                        0xbac5a72d,
                        0x168fe9ef,
                        0x9727301e,
                        0x76cd1ddb,
                        0x2bcd89e0,
                        0x45b7de13,
                        0xf239f2ad,
                        0xae66187d,
                        0xb92a6f32,
                        0xf0fb1c7f,
                        0xb77384f2,
                        0x6e405312,
                        0x6616a82e,
                        0x9bdca728,
                        0x1b5e6782,
                        0xdd243a3f,
                        0xf148d161,
                        0xfe0e7b47,
                        0x0fdadcf7,
                        0x9f21d59d,
                        0x5057328f,
                        0x22f944b9,
                        0x7e68d807,
#endif // ( PMPML_CHUNK_SIZE > 32 )
#if ( PMPML_CHUNK_SIZE > 64 )
                        0x46de914d,
                        0x2d351dad,
                        0x6b0f3436,
                        0x6d6a8943,
                        0xcd18923c,
                        0x2e8fa891,
                        0x33f1ed84,
                        0x30e3a20a,
                        0xa15f52a0,
                        0x3162fa56,
                        0xa60d4a72,
                        0x3e9fab64,
                        0x0a584673,
                        0x99d08542,
                        0x5ce99b5a,
                        0xcf1be8b0,
                        0xe83225e3,
                        0xad522e70,
                        0xb17e0c87,
                        0x5b081b14,
                        0xc4c71a48,
                        0xb430a70b,
                        0xf38673cd,
                        0x1aad3b26,
                        0x0e50ca70,
                        0xa1aeb568,
                        0x4140ea0c,
                        0xdabeee2d,
                        0x2779c11b,
                        0x5e06c86e,
                        0x12803b8f,
                        0xa46fd322,
                        0x7de67db9,
                        0x7d1ee355,
                        0xbea94742,
                        0xf529e572,
                        0x5374fffc,
                        0xf9037c7a,
                        0x1010523f,
                        0xb1a96f9c,
                        0x89b49bfc,
                        0xf2469dc2,
                        0x1692f9e1,
                        0x95ec9a68,
                        0x09426ab7,
                        0x0bc30953,
                        0x8628bd58,
                        0xa28375f2,
                        0xd9d4c2bf,
                        0xaae40027,
                        0x2b56df1b,
                        0x9d9fbc50,
                        0x14bf937d,
                        0xe7b0fb0a,
                        0xa5e40995,
                        0xfae90145,
                        0x1ea68371,
                        0x671f2f40,
                        0xc654778c,
                        0x477cf3fd,
                        0x6aa5cbda,
                        0x8f9960c8,
                        0xc08542ef,
                        0x88bbddc8,
#endif // ( PMPML_CHUNK_SIZE > 64 )
#if ( PMPML_CHUNK_SIZE > 128 )
#error VALUES NOT GENERATED for PMPML_CHUNK_SIZE > 128
#endif // ( PMPML_CHUNK_SIZE > 128 )

                },
        },
#endif // ( PMPML_LEVELS > 5 )

#if ( PMPML_LEVELS > 6 )
        // Level 0
        {
                UINT64_C(0xf33fe2d4 ),
				UINT64_C( 0x3be3330adb ),
#ifdef PMPML_USE_SSE
#if PMPML_USE_SSE_SIZE == 128
#elif PMPML_USE_SSE_SIZE == 256
				{UINT64_C( 0 ), UINT64_C( 0 )}, // dummy
#else
#error unsupported PMPML_USE_SSE_SIZE value
#endif
#endif
               {
#if ( PMPML_CHUNK_SIZE > 0 )
                        0x413faa9b,
#endif // ( PMPML_CHUNK_SIZE > 0 )
#if ( PMPML_CHUNK_SIZE > 1 )
                        0x1a3a2814,
#endif // ( PMPML_CHUNK_SIZE > 1 )
#if ( PMPML_CHUNK_SIZE > 2 )
                        0x957ff066,
                        0xfc5c55ec,
#endif // ( PMPML_CHUNK_SIZE > 2 )
#if ( PMPML_CHUNK_SIZE > 4 )
                        0x7898f40d,
                        0x30d71b62,
                        0xab1f1b9a,
                        0x5c93c31a,
#endif // ( PMPML_CHUNK_SIZE > 4 )
#if ( PMPML_CHUNK_SIZE > 8 )
                        0x27e1bf84,
                        0x277fd4f4,
                        0xc8de8b61,
                        0x619ec0a3,
                        0xcc3106c9,
                        0x7e07e8c7,
                        0xadbbff04,
                        0x986f8050,
#endif // ( PMPML_CHUNK_SIZE > 8 )
#if ( PMPML_CHUNK_SIZE > 16 )
                        0x26cd3f0a,
                        0xe7dcfd5a,
                        0xed3be524,
                        0x4a1e0f2b,
                        0xe0888023,
                        0x24d0c5eb,
                        0x476e89ae,
                        0x1a222b82,
                        0xb3d0cd98,
                        0x8856e275,
                        0x95ac5c19,
                        0xbbf334b5,
                        0x1a346ac4,
                        0x9f9ed27d,
                        0xe64567c6,
                        0xfc52f176,
#endif // ( PMPML_CHUNK_SIZE > 16 )
#if ( PMPML_CHUNK_SIZE > 32 )
                        0x98c8223c,
                        0xc09233fb,
                        0x078e98a4,
                        0xa36a369a,
                        0x89dfd3f0,
                        0x10a40ad1,
                        0xd14f4f1f,
                        0xe8ec2908,
                        0xb9af0bd3,
                        0x4d55c288,
                        0xc235e430,
                        0x77564268,
                        0x42c4877e,
                        0x00baab49,
                        0xd79bda2b,
                        0x490fcfc2,
                        0x225bfa4b,
                        0x216af042,
                        0xac221547,
                        0x6d8d84e0,
                        0x17dc383c,
                        0x49dcb049,
                        0x46d29882,
                        0x6661b4ed,
                        0x77b0becd,
                        0xf7a52591,
                        0x70c7256d,
                        0x0872d1fd,
                        0x2940fad9,
                        0x2c857e39,
                        0x358bf808,
                        0x0081180c,
#endif // ( PMPML_CHUNK_SIZE > 32 )
#if ( PMPML_CHUNK_SIZE > 64 )
                        0x01ec2a40,
                        0x3b7e716d,
                        0x2e0da024,
                        0xb77c9d9f,
                        0x725b6a35,
                        0x42d22b0c,
                        0x30fe2079,
                        0x8b72db40,
                        0xba80de6a,
                        0x03fb3689,
                        0x0557ad42,
                        0x7237cc5d,
                        0x792b74ae,
                        0x3bd5a870,
                        0x136749ef,
                        0x81c9ddf5,
                        0x95b80aa7,
                        0x7e885861,
                        0xc797839c,
                        0x667083b5,
                        0xe8e9b2d7,
                        0x9b282b8e,
                        0x8e7a7db0,
                        0x79d39fea,
                        0x1f9cea00,
                        0xf7c5c4f1,
                        0x9e669399,
                        0x136a5889,
                        0x680d40a6,
                        0xea6ba4fa,
                        0xf7660f4b,
                        0xfd9af075,
                        0xf242ad0c,
                        0xcf89799a,
                        0x1173b431,
                        0x8b3b0aa0,
                        0xd8e862ff,
                        0x6ee0e93e,
                        0x482772e0,
                        0x6f382985,
                        0x995506f1,
                        0x5f1c3b7f,
                        0xc54d0f78,
                        0x5ba663aa,
                        0x91e7cc43,
                        0x07295028,
                        0xe1f9640d,
                        0x5e0d49cb,
                        0xd1d6d96a,
                        0x7e602d59,
                        0xc8a376ac,
                        0x15ddcff4,
                        0x90481328,
                        0x543e0eb7,
                        0x07d297e4,
                        0xddfb2d18,
                        0x94a578aa,
                        0x9a39368e,
                        0x6aab286e,
                        0x0a39debd,
                        0x8ee5e818,
                        0x5c30655e,
                        0x661772e5,
                        0x527b25c1,
#endif // ( PMPML_CHUNK_SIZE > 64 )
#if ( PMPML_CHUNK_SIZE > 128 )
#error VALUES NOT GENERATED for PMPML_CHUNK_SIZE > 128
#endif // ( PMPML_CHUNK_SIZE > 128 )

                },
        },
#endif // ( PMPML_LEVELS > 6 )

#if ( PMPML_LEVELS > 7 )
        // Level 0
        {
                UINT64_C(0x6d983dad ),
				UINT64_C( 0x3e435b56e5 ),
#ifdef PMPML_USE_SSE
#if PMPML_USE_SSE_SIZE == 128
#elif PMPML_USE_SSE_SIZE == 256
				{UINT64_C( 0 ), UINT64_C( 0 )}, // dummy
#else
#error unsupported PMPML_USE_SSE_SIZE value
#endif
#endif
                {
#if ( PMPML_CHUNK_SIZE > 0 )
                        0x4014ee95,
#endif // ( PMPML_CHUNK_SIZE > 0 )
#if ( PMPML_CHUNK_SIZE > 1 )
                        0xfdbe07f6,
#endif // ( PMPML_CHUNK_SIZE > 1 )
#if ( PMPML_CHUNK_SIZE > 2 )
                        0x27a2c5d7,
                        0x497ae9f0,
#endif // ( PMPML_CHUNK_SIZE > 2 )
#if ( PMPML_CHUNK_SIZE > 4 )
                        0x18a372d5,
                        0x375c55ae,
                        0x4aab4110,
                        0x2d554d43,
#endif // ( PMPML_CHUNK_SIZE > 4 )
#if ( PMPML_CHUNK_SIZE > 8 )
                        0x9504cbcd,
                        0xfbaedcce,
                        0x758c4326,
                        0xfafbba66,
                        0x9bda2b02,
                        0x1d955954,
                        0xe4bb3e12,
                        0xd558ed02,
#endif // ( PMPML_CHUNK_SIZE > 8 )
#if ( PMPML_CHUNK_SIZE > 16 )
                        0x770c3bec,
                        0x6fcf284d,
                        0x7142cbb0,
                        0xefe84369,
                        0x9516d833,
                        0x097022c9,
                        0x8572785a,
                        0xcc866071,
                        0x11084cac,
                        0x15707ce6,
                        0xc8a05f69,
                        0xf15c7b38,
                        0x3607b067,
                        0xa8f646b2,
                        0x62949620,
                        0x0e013130,
#endif // ( PMPML_CHUNK_SIZE > 16 )
#if ( PMPML_CHUNK_SIZE > 32 )
                        0xe73a8f37,
                        0x853e3bd2,
                        0x4ad40839,
                        0x961fff58,
                        0x5b9a291e,
                        0x4df678ae,
                        0x9e49ab57,
                        0x12c0823b,
                        0x804a15b9,
                        0xedbe4a7f,
                        0x3f65fe91,
                        0x0aca6940,
                        0xa14a7dc6,
                        0xd9a78895,
                        0x4c90b7fa,
                        0x90443c6a,
                        0xc1325ada,
                        0x48876a7b,
                        0x091df649,
                        0x7ae46bc8,
                        0xdcfdc695,
                        0xc398dd91,
                        0xe6a24f20,
                        0x333f496b,
                        0xe08413da,
                        0xbd197fa0,
                        0x55abc5e6,
                        0xa1abe124,
                        0x1cfdeee2,
                        0x48732fff,
                        0xdb2f1a4a,
                        0x192de0ae,
#endif // ( PMPML_CHUNK_SIZE > 32 )
#if ( PMPML_CHUNK_SIZE > 64 )
                        0x87a288b7,
                        0x406f0062,
                        0xc4358b22,
                        0x19ccdeba,
                        0xa30cd0c5,
                        0x848d1e9a,
                        0x2fd31932,
                        0x7b78238e,
                        0x9e9a208e,
                        0x517f5394,
                        0x8b689859,
                        0xe2202a00,
                        0x7d82aa8d,
                        0x736d2f4c,
                        0x8a5c630a,
                        0xaf1857bf,
                        0xd56d5b1f,
                        0x3416feea,
                        0x6b16d737,
                        0xf61f0747,
                        0x359f0963,
                        0x6044d7c6,
                        0xedcdcafd,
                        0xa53ff8c5,
                        0x09c7732a,
                        0x7f1b4137,
                        0x9d63e5c0,
                        0x776c5120,
                        0x0b0d231e,
                        0x57e54da1,
                        0x3b5e1e5e,
                        0x63069af7,
                        0xa44a600c,
                        0x3d5a02fb,
                        0x2387039e,
                        0xf32214b4,
                        0x95707014,
                        0x65ae19ab,
                        0xa906bfd3,
                        0x41083458,
                        0x106bdfd4,
                        0x41a3efe8,
                        0xb58bee3f,
                        0xaa70953c,
                        0x01cf2485,
                        0x40e5bdb9,
                        0xc94b2765,
                        0xc79cd151,
                        0xad2d9daa,
                        0x62b40b60,
                        0x02800b32,
                        0x97d69686,
                        0xa9f0efdb,
                        0x24952809,
                        0x48694c4f,
                        0x630104fe,
                        0x24f26b53,
                        0xc94d2a0f,
                        0x8635b8db,
                        0xb6822421,
                        0xe53c26dd,
                        0x9286330f,
                        0xf5a431ec,
                        0xacbb86b4,
#endif // ( PMPML_CHUNK_SIZE > 64 )
#if ( PMPML_CHUNK_SIZE > 128 )
#error VALUES NOT GENERATED for PMPML_CHUNK_SIZE > 128
#endif // ( PMPML_CHUNK_SIZE > 128 )

                },
        },
#endif // ( PMPML_LEVELS > 7 )

#if ( PMPML_LEVELS > 8 )
#error VALUES NOT GENERATED for PMPML_LEVELS > 8
#endif // ( PMPML_LEVELS > 8 )

};
