# About This Repository

This is a fork of Google's "smhasher" suite. It incorperates
changes from Reini Urban's fork as well.

It has been substantially extended to:

* Use better stats internally (g-test)
* Better test how the hash functions are seeded
* Separate timing of hashing from that of initialization

My personal interest is to develop and test hash functions which
are suitable in contexts where the hash function will be seeded
relatively rarely, must perform well, are sufficiently "strong"
that they can be safely used to hash untrusted data.

This is a level of security below that of a proper message
digest, but above that of "your average hash function".

In particular a hash function must not leak information
about its seed, and must not have any inherent weaknesses that
allow an attacker to construct a collision attack without
knowing the seed.

I have tried to enhance the smhasher package to make it easier
to test the scenario outlined above. Your mileage may vary.

# A New API for seeding Hash functions

It would seem that smhasher was originally intended to test hash
functions with 32 bit seeds, and this requirement was baked into
the design in various ways. This is problematic because a hash with
a larger seed will have to somehow map the 32-bit seed into a larger
space, interfering with testing. While some hash functions have well
defined mappings to handle 32 bit seeds many, if not most, do not.
In a test like the avalanche test, the results of 32 bit test can
easily differ from that of true seed-size testing.

So to properly test a hash function you need to be able to deal with
whatever seed size it requires, and test based on that size.

Another issue is that some hash functions have expensive seeding
operations which are intended to be amortized to a single seeding
for many hash operations. Timing stats for such functions will not
be accurate unless the action of seeding the state, and actually
executing the hash function are separated.

So where the old interface was:
```C
typedef void (*pfHash) ( const void *blob, const int len, const uint32_t seed, void *out );
```

I have added a new interface with two functions like this:
```C
typedef void (*pfSeedState) ( const int seedbits, const void *seed, const void *state );
typedef void (*pfHashWithState) ( const void *blob, const int len, const void *state, void *out );
```

The seedbits argument in pfSeedState is provided to allow a single
seed preparation function serve multiple seed sizes if required. Most
implementations would ignore it as it would always be constant.

# Anatomy of a String Hash Function

A hash function can be broken down into various components. The various
attributes of the different components influence what applications
a hash function is useful for. A general skeleton is as follows:

    seed -> state
    length -> state
    while data unread:
        data -> state
        mix state
    tail -> state
    mix state
    finalize state

But obviously not all hash functions have every component.

## The Seed

The seed is used to initialize the state so that hashing a given key
or set of keys with a different seed produces unpredictably different
hash values.

The size of the seed influences the security of the hash. An
insufficiently large seed can be brute force attacked to determine
the seed. This is one reason why these days 32 bit seeds are generally
considered obsolete.

## The State

The state is generally larger than the seed, often double the size,
and is used to represent the overall state of the hashing process
as it procedes. As each unit of data is processed by the mix function
the state is updated, and the eventual values of the state when
hashing is done determine the resulting hash.

## Seeding the State

This is the function that maps seed into the state. It may be an
expensive operation in some hash functions, especially if the
state is large.

## Length Mixing

It is common to mix the length of the key into the state before
hashing the key data to prevent against key extension and
multicollission attacks.

## The Mix Function

This is a function which mixes the state together, presumably
in an unpredictable way, and is executed, possibly more than once,
after reading a unit of data, and sometimes in the finalizer as
well.

## The Block Read/Mix Loop

This is the loop that handles reading a block of data and mixing
it into the state using the mix function.

## The Tail Reader

Since most hash functions are block oriented there is a need
for a function that that handles loading any trailing bytes
shorter than a full block size. This code generally needs to
be structured to avoid key extension attacks. Often even when
there is nothing to be read this function will alter the state
anyway as part of the finalization process. It is common to see
similar strategies used to pad the final block of block ciphers.

Byte oriented hash functions may omit the tail reader.

## The Finalizer

This is a function which does the final mix of the state after
all the input data has been read, and which computes the final
hash value from that state.

Often the finalizer is implemented by executing the mix function
multiple times without reading any further data, but it can also
be a custom function distinct from the mix function.

It is common to xor the elements of the state together to produce
the final hash.

# Design Trade Offs

A major factor governing the security profile and performance of
a hash function come down to:

    1. Size of the Seed
    2. Size of the State
    3. Size of the read block.
    4. Size of the hash value

The following relationships tend to hold:

    SeedSize <= StateSize (often 1:2)
    BlockSize < StateSize (often 1:2 or more)
    HashSize < StateSize  (often 1:2 or more)

There are probably good reasons for this:

If the state is larger than the seed then it would seem it is easier
to make guarantees about the state, such as it being non-zero at all
times.

If the block size is not (much) smaller than the state size
then presumably the data read might overwhelm the entropy in the state.

If the hash-size is not smaller than the state-size then the hash
value exposes information about the internal state of the hash function
which might be used to reverse engineer the initial state and/or the
seed. By using some kind of compression function to reduce multiple
state vectors into a single value the actual state is obscured.

An example of these kind of ratios and sizes is SipHash

    SeedSize:128 <= StateSize:256
    BlockSize:64 <  StateSize:256
    HashSize:64  <  StateSize:256

and its final compression function:

    v0 ^ v1 ^ v2 ^ v3

which effectively obscures the values of the state vector v0,v1,v2,v3
and presumably makes it more difficult for a solver to reverse engineer
what the initial state was.

# About the tests

I have not reviewed ALL of the smhasher tests in detail yet. I have
reviewed most of the stats logic, especially logic related to determining
if a set of hash values is "random" or not, and upgraded it to use g-test.

If it is not documented below then I have not gotten around to it yet.

## Avalanche Test

This tests that a hash meets the "strict avalanche criteria", which
is that when changing any single bit of the input (seed or key) we expect
about 50% of the bits change.

This is tested at various key lengths, including 0 byte keys, and stats
are calculated about how input bit affects the out, and whether the
results are random or not.



