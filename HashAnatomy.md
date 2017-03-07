# Anatomy of a String Hash Function

A hash function can be broken down into various components. The various
attributes of the different components influence what applications
a hash function is useful for. A general skeleton is as follows:

``
    seed -> state
    length -> state
    while data unread:
        data -> state
        mix state
    tail -> state
    mix state
    finalize state
``

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
<!--- vim: set ft=markdown: --!>
