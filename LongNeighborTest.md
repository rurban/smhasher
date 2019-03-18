# LongNeighborTest

This test was developed by [Henning Makholm](http://henning.makholm.net)
in January 2019, while working at [Semmle Ltd](https://www.semmle.com).

As of this writing Semmle has not yet officially allowed it to be released
under a permissive license, so beware of possible legal trouble if you
use it. (I'm currently researching how to get such official permission).

## Overview

This test uses a meet-in-the-middle approach to look for hash collisions
between messages with low Hamming distances.

Developing the test was prompted by the discovery that
[SpookyHash V2](http://burtleburtle.net/bob/hash/spooky.html)
has lots of 3-bit collisions due to insufficient diffusion between
the processing of the last full input block and the partial block
at the end of the input. The goal of the test is to discover similar
trouble in other hashes.

Because Spooky has an unusually large block size of 96 bytes (=768 bits),
and it uses a completely different algorithm for messages shorter than 2
blocks, this problem only shows up for messages of length 281 to 287 bytes,
and then again from 377 to 383 bytes, etc. Notably, messages of any "nice"
length are not affected, which means that we need to test for all possible
message lenghts.

## Method

We generate a lot of "base" messages of lengths between 10 and 300 bytes.
For each length there is one base message consisting entirely of zero bits,
one that consists entirely of ones, and three with pseudorandom bits.
Each of these base messages are processed separately.

With the default parameters, we derive about 3 million variants of the
base message; then we look for hash collisions between those variants.
There's a variant for each 1-bit flip anywhere in the message, one for
each combination of 2 bit-flips within the last 2048 bits of the message,
and one for each combination of 3 flips in the last 160 bits of the message.
There are also some variants that consist of appending some zero bytes
to the message and then flipping 2 bits. (Appending zeroes often has the
same effect as bit flips, due to the way many hashes include a length
counter in their final padding).

If we find any collision between variants of a base message, that base
is declared "bad". At the end we compare the number of bad bases found with
the number we would expect with an ideal ("truly random") hash function.

## Interpreting the results

The test tries to adapt its reporting to odd hash sizes. SMHasher
currently only supports sizes that are powers of two, which leads to three
rather distinct reporting regimes:

### 128-bit hashes or longer

An 128-bit hash is disqualified by discovering even a single bad base,
since it would be astronomically implausible to find one for an ideal
hash.

The collision is displayed as soon as it is found, but the test continues
for some time afterwards, in the hope of finding even more striking
collisions (that is, one with fewer bits or the bits closer together)
with slightly longer bases.

### 64-bit hashes

With the default search parameters we would expect to find a bad base
for about one in 8000 truly-random 64-bit hash functions tried, so a
single bad base is not an absolute condemnation. If the hash has a
_systematic_ tendency towards small-distance collisions it would generally
show up as finding more than one bad base, in which case it fails the test.

If any bad base is found, the "most striking" collision among those found
will be printed at the end of the test. Even if there is only a single
example (and the hash therefore passes), it is probably worthwhile to
investigate how it managed to collide, and see if there is a discernible
reason that can be fixed somehow.

Collisions with particularly simple deltas may still cause a 64-bit hash
to fail with only a single bad base. (See "horrible collisions" below).

### 32-bit hashes

With so few bits, the usual battery of variants would exceed the birthday
bound for _every_ base. This would invalidate the methodology of comparing
the number of bad and good bases.

The test will automatically reduce how many 2- and 3-bit variants it tries,
such as to keep below the birthday bound. This means, in effect, that
collisions of Hamming distance 3 or more are expected if one of more of
their bits are far from the _end_ of the message.

There are still enough variants that even with an ideal hash, a few hundred
bad bases is expected. If the _actual_ number of bad bases is more than 1.3
times the theoretical value, the hash fails the test.

The 1.3 factor is more or less pulled out of a hat. It causes the
`MurmurOAAT` and `Murmur2A` hashes to fail the test just barely --
they seem to have particular problems with inputs that are mostly
zeroes. The newer version `Murmur3A` passes the test nicely.

_TODO:_ Rather than a fixed factor, we really ought to compute a p-value
for the observed number of bads and compare that to some chosen
significance level. (To make this feasible we'd probably have to assume
that the number of bads follow a binomial distribution, even though very
short bases actually have a lower chance of being bad because some deltas
won't fit).

## Surprise score

The test computes a "surprise score" for each collision it finds, mostly
so it can choose the "most surprising" of the collisions to print out
at the end of the test.

The score is computed as `(1/p)-1` where `p` is a somewhat sloppily computed
probability that an ideal hash function would have a collision for
_any prefix of this particular base message_ and either this particular
delta or a simpler one. (A simpler delta is roughly one with the same
number of bits flipped, but where those bits are closer to the end of
the message.)

**Warning:**
The surprise score is relative to _one particular_ base message. Because of
the selection bias inherent in displaying the collision with the _highest_
surprise score, the `p` **should not** be interpreted as the likelihood of
a hash function to have a collision as bad as this anywhere. Completely
fine 32-bit hashes will routinely find bases that have a collision with
a surprise score in the hundreds.

### Horrible collisions

That being said, if the test sees a surprise score of 10<sup>12</sup> of
more, it is considered a "horrible collision". These are ones we don't
ever expect to come across for any good hash function. (Note that an
"indistinguishably random" hash function will _have_ bases that have
horrible collisions, but those bases ought to be so rare that we should
never randomly find one during any realistic test run).

A horrible collision will be printed out immediately when it is found
(unless an even more horrible one has already been printed), and will in
and of itself cause the hash to fail the test.

A collision with a 128-bit hash is always horrible. (Or more precisely,
the test would not reach any delta that could produce a non-horrible score
until after hashing several hundred terabytes of data).

It is impossible for a collision with a 32-bit hash to be horrible
-- even if the hash of the one-byte messages `00` and `01` coincide,
the score will be less than 10<sup>10</sup>.

64-bit collisions may land on either side of the threshold.

## Tweaking the search parameters

The throroughness of the search is controlled by hard-coded constants
at the top of `LongNeighborTest.cpp`.

If you know the input block size of the hash you're testing you can save
time by decreasing `MAXMSGBYTES` so it corresponds to just a few (say, three)
input blocks.

The three `MAXRANGE_xDELTA` constants control how long a distance in _bits_
from the end of the message will be considered for constructing 1-, 2- or
3-bit deltas.

The default value for `MAXRANGE_1DELTA` is effectively infinity.

`MAXRANGE_2DELTA` defaults to 2048 bits, which should be "large enough
for everyone".

`MAXRANGE_3DELTA` is 160 bits by default, which makes for roughly as many
3-bit deltas as 2-bit deltas. Increase it at your own risk; the cost scales
_cubically_. Setting it to 512 will let you search for arbitrary 6-bit
deltas in a 64-byte message, but the test will take most of a day unless
you scale `MAXMSGBYTES` back. More than 1024 will (a) take forever and
(b) require you to increase the size of `PackedDelta::codeword_t`.

You can set `STORE_3BIT_HASHES` to `false` to conserve memory at the expense
of not discovering collisions between two 3-bit variants. This is generally
only useful if you are on a 32-bit machine, or if you have jacked
`MAXRANGE_3DELTA` up way high and have the patience to compute
billions of hashes per base but not the RAM to store them.

If you have even more time to spare you can increase `BASES_PER_LENGTH`
to try more random base messages for each message lenght. But beware that
this may sometimes mask problems, such as the particular problems Murmur2A
has with all-zeroes bases.

Note that the random base messages are _deterministic_, using the message
length as a seed. If you want to improve coverage by running the test several
times, you'll need to modify the code that generates random bases.

## Detaching the test from SMHasher

At the time of this writing, the code is only weakly coupled to SMHasher.
You can grab `LongNeighborTest.{cpp,h}` and `Birthday.{cpp.h}` (which
contains a helper function for computing both small and large
birthday-collision probabilities in a numerically stable way),
and be on your way.

The main code depends on SMHasher's `Random.h` for creating base messages,
but it should be easy to substitute another PRNG.

## So ... what about SpookyHash anyway?

I recommend you use version 1 rather than version 2. It is marginally
slower but does not appear to have any problems with small-delta collisions.
(I've ran the test with `MAXRANGE_3DELTA=800` on the most relevant message
lengths without finding and).

At your option you may use the change to _short_ hashes from version 2;
that seems to be harmless and possibly improve the hash quality.
