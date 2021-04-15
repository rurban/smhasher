# Pearson Block Hashing

The relatively low speed seen with regular [Pearson Hashing](https://github.com/Logan007/pearson) on modern CPUs might originate from the fact that it only processes one byte of input at a time. So, this is an attempt to read and digest multiple bytes, a block of bytes, in order to speed things up. Still relying on the underlying Pearson Hasing scheme, I tenderly call it **Pearson B. Hashing**.

:information_source: **RECAP**

> Pearson Hashing basically digests every new input byte by exclusive-oring it into the current internal state and afterwards applying a permutation on that state:  `new_state = permute(current_state ^ input_byte);`. Using different permutations allows for wider output – each byte position of the output hash value gets its own  permutation (derived from a basic one).

## Blocksize

As a look-up table for numbers above 16-bit width definitely is out of reach, the permutation needs to be calculated arithmetically anyway. So, we need to rule which of the common block sizes to favor: 32 or 64 bit. Both are common and assumed to natively be supported on a lot of platforms. However, to digest twice as many bits or bytes per step, we will stick with 64 bit for now.

## Permutation

Starting some research from my unsuccessful ideas about linear congruential generators I once tried with pure Pearson Hashing, I came across [Mix13](http://zimbry.blogspot.com/2011/09/better-bit-mixing-improving-on.html) as with constants discovered by David Stafford using simulation. Published on his blog in 2011, this function meanwhile gets widely used and is better known as part of `splitmix64()`. David clarified to me in a very kind eMail that this of his work is released to the public domain. David, thank you very much for sharing!

```C
#define permute64(in)         \
    in ^= (in >> 30);         \
    in *= 0xbf58476d1ce4e5b9; \
    in ^= (in >> 27);         \
    in *= 0x94d049bb133111eb; \
    in ^= (in >> 31)
```

:information_source: **NOTE**
> If ever considering 32-bit block size, Chris Wellons' [Hash Prospector](https://github.com/skeeto/hash-prospector) will be helpful. Chris found some very interesting permutation candidates as detailed in his [blog](https://nullprogram.com/blog/2018/07/31/). Credits to Chris for his work and especially for giving his findings to the public domain! His `triple32()` seems to be a very prospective candidate:
>
>```C
>#define permute32(in) \
>    in ^= in >> 17;    \
>    in *= 0xed5ad4bb); \
>    in ^= in >> 11;    \
>    in *= 0xac4c1b51); \
>    in ^= in >> 15;    \
>    in *= 0x31848bab); \
>    in ^= in >> 14
>```

I plan to decrement the input by one to avoid the obvious fixed point zero being mapped to zero again. To allow for longer hash values according to the Pearson hashing scheme, additional blocksize-wide "digits" get their different permutation by decrementing the input by two, three, ….

## Excess

If a string to be hashed is not of some multiple-of-four-bytes length, we need a determine how to handle the excess bytes. They shall be digested as single values casted to 64-bit, e.g. `… some text of length mod 8 … | 0x01  | 0x02 | 0x03` would require three more rounds to digest `0x00…001`, `0x00…002`, and `0x00…003`. Not to allow for a collision with the hash value of the very similar input ` … same text of length mod 8 … | 0x00…001 | 0x00…002 | 0x00…03` (length modulo 8 again), an additional effort needs to be taken:

At some point, we could mix the total string length into the hash (does not help all cases) or give it an extra scramble before taking care of the excess bytes. I chose to do both. Let's apply a logical negation once before handling excess bytes. This is a fast operation which we repeat before finally digesting the string length on top.

## Speed

Compiled by `gcc -O3 -march=native pearsonb.c test.c`, the resulting executable `./a.out` shows the following results of its speed measurement:

| plain C            | 32-bit hash | 64-bit hash | 128-bit hash | 256-bit hash |
| :---               | ---:        | ---:        | ---:         | ---:         | 
| i7 7500U           |1825.6 MB/s  |1866.9 MB/s  |1699.8 MB/s   |1476.0 MB/s   |
| i7 5775C           |1955.3 MB/s  |1953.6 MB/s  |1698.2 MB/s   |1201.1 MB/s   |
| i7 2860QM          |1630.6 MB/s  |1682.4 MB/s  |1367.9 MB/s   |1041.0 MB/s   |
| i5 M430            |1051.6 MB/s  |1145.1 MB/s  | 943.3 MB/s   | 720.0 MB/s   | 
| Cortex A53 RPi3B+  | 524.3 MB/s  | 525.3 MB/s  | 420.3 MB/s   | 284.3 MB/s   |    

## Further Thoughts

Input data blocks are assumed to be in little endian byte order.

Based on earlier experience, I think that SSE might slow things down. But I will give vectorization a try sometime or other.

To handle streamed data in chunks just as they arrive, the hashing could be splitted into _updating_ the hash value with pieces of blockwidth-sized pieces of data, and _finalizing_, i.e. handling the excess bytes and total length.

Please keep in mind that this is not a cryptographically secure hashing scheme. It however can safely be used as internal checksum, for mixing bits, or in case there no intention to make the hash value a _public proof of knowledge_ for some secret without presenting that secret itself.
