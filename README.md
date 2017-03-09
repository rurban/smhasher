# SMHasher - A hash function test-bench in C++

This is a fork of Google's "smhasher" suite, available here:

    https://chromium.googlesource.com/external/smhasher

It incorporates changes from Reini Urban's fork as well. See:

    https://github.com/rurban/smhasher

Compared to those versions it has been substantially extended to:

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

I have changed to a new interface with two functions like this:
```C
typedef void (*pfSeedState) ( const int seedbits, const void *seed, const void *state );
typedef void (*pfHashWithState) ( const void *blob, const int len, const void *state, void *out );
```

The seedbits argument in pfSeedState is provided to allow a single
seed preparation function serve multiple seed sizes if required. Most
implementations would ignore it as it would always be constant.

The pfSeedState function is *optional*, and when omitted it is assumed that the input
"state" to the hash function corresponds exactly to that of the seed. In other words
when there is no pfSeedState function defined the configuration for the hash should set
the "statebits" to be the same as the "seedbits", and the "state seeding" operation
will simply pass through the seed as a pointer.

The end result of all of this is that we now support any seed size, and provide support
for no, shared, or unique state seeding functions.

# How To Add A New Function

You will need to:

1. Implement functions that match the expected API.
2. Update main.cpp to contain a new HashInfo entry for the hash

    <pre>
        typedef struct HashInfo
        {
          const char * name;
          const char * desc;
          int seedbits;
          int statebits;
          int hashbits;
          uint32_t verification;
          pfSeedState seed_state;
          pfHashWithState hash_with_state;
        } HashInfo;
    </pre>

    For example:
    <pre>
      { "Spooky128", "Bob Jenkins' SpookyHash, 128-bit seed, 128-bit result",
        128, 128, 128, 0xC633C71E,
        SpookyHash_seed_state_test, SpookyHash128_with_state_test },
    </pre>

    The verification value can be set to 0x0, in which case failed verification
    tests will produce a warning, but allow you to continue testing, or you can
    just put whatever you like in there as we will update its value to be correct
    in a following step.

3. Compile And Build

    You can run ./SMHasher --validate to see if any hashes fail test. If they
    do the test harness will not let you continue until you correct them. The
    only exceptions are for those with a verification code of 0. If everything
    is good then you should see:

    <pre>
    Self-test PASSED.
    </pre>

    At this point if you set the verification value to 0 you are done. But you
    will want to return here later once you have finalized the implementation of
    your hash function.

4. Update the Verification Code for your Hash

    You can do this two ways, the first is run

        perl update_hashes.pl

    which /should/ update your hashes verification code (and *only* its code)
    automagically. If it says it has updated more then there is something wrong.
    Alternatively you can do what it does manually and run

        ./SMHasher --validate

    which will produce output saying what the expected, and actual verification
    codes were, you can then C&P the corrected code into the structure. For example:

    <pre>
    ./SMHasher --validate
    Self-test FAILED!
    Spooky128            - Verification value 0xC633C71E : Failed! (Expected 0xC633C70E)
    </pre>

    I would use the perl script personally.

5. Build Again.

    Once you have corrected the verification code you need to rebuild to bake
    it in. Once that is complete you are done. Happy testing!


# About the tests

I have not reviewed ALL of the smhasher tests in detail yet. I have
reviewed most of the stats logic, especially logic related to determining
if a set of hash values is "random" or not, and upgraded it to use g-test.

If it is not documented below then I have not gotten around to it yet.

## Sanity Test

This does some very basic tests. It verifies that the hash is consistent
and that simple key changes produce changed hashes. It also verifies that
the hash is capable of hashing a string of nulls without producing excess
collisions, this is a simpler version of the "Zeroes" test.

<pre>
    [[[ Sanity Tests ]]] - BeagleHash_32_112

    BeagleHash_32_112    - Verification value 0x5D4AA95B : Testing!
    Sanity check simple key bit flips and consistency..........PASS
    Sanity check null suffixes change the hash (simple)..........PASS
</pre>

## Speed Tests

This test actually contains two subtests which can be run independently.

### BulkSpeed Test

Times hashing very long keys at different alignments. Architectures that
allow unaligned access will probably not see much difference at the different
alignments.

<pre>
    [[[ Speed Tests ]]] - BeagleHash_32_112

    Bulk speed test - 262144-byte keys
    Alignment  7 -  1.839 bytes/cycle - 5260.38 MiB/sec @ 3 ghz
    Alignment  6 -  1.839 bytes/cycle - 5260.31 MiB/sec @ 3 ghz
    Alignment  5 -  1.839 bytes/cycle - 5260.33 MiB/sec @ 3 ghz
    Alignment  4 -  1.839 bytes/cycle - 5260.38 MiB/sec @ 3 ghz
    Alignment  3 -  1.839 bytes/cycle - 5260.19 MiB/sec @ 3 ghz
    Alignment  2 -  1.839 bytes/cycle - 5260.34 MiB/sec @ 3 ghz
    Alignment  1 -  1.839 bytes/cycle - 5260.26 MiB/sec @ 3 ghz
    Alignment  0 -  1.850 bytes/cycle - 5292.65 MiB/sec @ 3 ghz
    Average      -  1.840 bytes/cycle - 5264.36 MiB/sec @ 3 ghz
</pre>

### KeySpeed Test

Times hashing keys of various lengths, ranging from the empty string to
relatively long strings.

<pre>
    BeagleHash_32_112         0 byte keys       15.254 c/h
    BeagleHash_32_112         1 byte keys       31.000 c/h       31.000 c/b        0.032 b/c
    BeagleHash_32_112         2 byte keys       35.022 c/h       17.511 c/b        0.057 b/c
    BeagleHash_32_112         3 byte keys       35.000 c/h       11.667 c/b        0.086 b/c
    BeagleHash_32_112         4 byte keys       36.000 c/h        9.000 c/b        0.111 b/c
    BeagleHash_32_112         5 byte keys       36.000 c/h        7.200 c/b        0.139 b/c
    BeagleHash_32_112         6 byte keys       36.000 c/h        6.000 c/b        0.167 b/c
    BeagleHash_32_112         7 byte keys       36.000 c/h        5.143 c/b        0.194 b/c
    BeagleHash_32_112         8 byte keys       46.000 c/h        5.750 c/b        0.174 b/c
    BeagleHash_32_112         9 byte keys       46.000 c/h        5.111 c/b        0.196 b/c
    BeagleHash_32_112        10 byte keys       46.000 c/h        4.600 c/b        0.217 b/c
    BeagleHash_32_112        11 byte keys       46.000 c/h        4.182 c/b        0.239 b/c
    BeagleHash_32_112        12 byte keys       46.000 c/h        3.833 c/b        0.261 b/c
    BeagleHash_32_112        13 byte keys       46.000 c/h        3.538 c/b        0.283 b/c
    BeagleHash_32_112        14 byte keys       46.000 c/h        3.286 c/b        0.304 b/c
    BeagleHash_32_112        15 byte keys       46.000 c/h        3.067 c/b        0.326 b/c
    BeagleHash_32_112        16 byte keys       49.981 c/h        3.124 c/b        0.320 b/c
    BeagleHash_32_112        17 byte keys       48.994 c/h        2.882 c/b        0.347 b/c
    BeagleHash_32_112        18 byte keys       49.000 c/h        2.722 c/b        0.367 b/c
    BeagleHash_32_112        19 byte keys       49.648 c/h        2.613 c/b        0.383 b/c
    BeagleHash_32_112        20 byte keys       49.000 c/h        2.450 c/b        0.408 b/c
    BeagleHash_32_112        21 byte keys       48.733 c/h        2.321 c/b        0.431 b/c
    BeagleHash_32_112        22 byte keys       49.000 c/h        2.227 c/b        0.449 b/c
    BeagleHash_32_112        23 byte keys       48.991 c/h        2.130 c/b        0.469 b/c
    BeagleHash_32_112        24 byte keys       55.776 c/h        2.324 c/b        0.430 b/c
    BeagleHash_32_112        25 byte keys       55.923 c/h        2.237 c/b        0.447 b/c
    BeagleHash_32_112        26 byte keys       55.963 c/h        2.152 c/b        0.465 b/c
    BeagleHash_32_112        27 byte keys       55.646 c/h        2.061 c/b        0.485 b/c
    BeagleHash_32_112        28 byte keys       55.922 c/h        1.997 c/b        0.501 b/c
    BeagleHash_32_112        29 byte keys       55.000 c/h        1.897 c/b        0.527 b/c
    BeagleHash_32_112        30 byte keys       55.262 c/h        1.842 c/b        0.543 b/c
    BeagleHash_32_112        31 byte keys       54.992 c/h        1.774 c/b        0.564 b/c
    BeagleHash_32_112        Average < 32       45.816 c/h        2.956 c/b        0.338 b/c

    BeagleHash_32_112        32 byte keys       57.592 c/h        1.800 c/b        0.556 b/c
    BeagleHash_32_112        36 byte keys       57.000 c/h        1.583 c/b        0.632 b/c
    BeagleHash_32_112        40 byte keys       63.000 c/h        1.575 c/b        0.635 b/c
    BeagleHash_32_112        44 byte keys       63.985 c/h        1.454 c/b        0.688 b/c
    BeagleHash_32_112        48 byte keys       66.988 c/h        1.396 c/b        0.717 b/c
    BeagleHash_32_112        52 byte keys       65.000 c/h        1.250 c/b        0.800 b/c
    BeagleHash_32_112        56 byte keys       72.184 c/h        1.289 c/b        0.776 b/c
    BeagleHash_32_112        60 byte keys       71.738 c/h        1.196 c/b        0.836 b/c
    BeagleHash_32_112        64 byte keys       74.352 c/h        1.162 c/b        0.861 b/c
    BeagleHash_32_112        68 byte keys       73.191 c/h        1.076 c/b        0.929 b/c
    BeagleHash_32_112        72 byte keys       80.513 c/h        1.118 c/b        0.894 b/c
    BeagleHash_32_112        76 byte keys       79.667 c/h        1.048 c/b        0.954 b/c
    BeagleHash_32_112        80 byte keys       82.933 c/h        1.037 c/b        0.965 b/c
    BeagleHash_32_112        84 byte keys       81.762 c/h        0.973 c/b        1.027 b/c
    BeagleHash_32_112        88 byte keys       89.614 c/h        1.018 c/b        0.982 b/c
    BeagleHash_32_112        92 byte keys       88.339 c/h        0.960 c/b        1.041 b/c
    BeagleHash_32_112        96 byte keys       89.750 c/h        0.935 c/b        1.070 b/c
    BeagleHash_32_112       100 byte keys       90.725 c/h        0.907 c/b        1.102 b/c
    BeagleHash_32_112       104 byte keys       97.953 c/h        0.942 c/b        1.062 b/c
    BeagleHash_32_112       108 byte keys       97.282 c/h        0.901 c/b        1.110 b/c
    BeagleHash_32_112       112 byte keys      100.531 c/h        0.898 c/b        1.114 b/c
    BeagleHash_32_112       116 byte keys       99.792 c/h        0.860 c/b        1.162 b/c
    BeagleHash_32_112       120 byte keys      106.307 c/h        0.886 c/b        1.129 b/c
    BeagleHash_32_112       124 byte keys      105.801 c/h        0.853 c/b        1.172 b/c
    BeagleHash_32_112       Average < 128       61.109 c/h        1.445 c/b        0.692 b/c

    BeagleHash_32_112       128 byte keys      108.190 c/h        0.845 c/b        1.183 b/c
    BeagleHash_32_112       256 byte keys      148.665 c/h        0.581 c/b        1.722 b/c
    BeagleHash_32_112       512 byte keys      304.271 c/h        0.594 c/b        1.683 b/c
    BeagleHash_32_112      1024 byte keys      583.592 c/h        0.570 c/b        1.755 b/c
    BeagleHash_32_112      2048 byte keys     1145.131 c/h        0.559 c/b        1.788 b/c
    BeagleHash_32_112      4096 byte keys     2239.439 c/h        0.547 c/b        1.829 b/c
    BeagleHash_32_112      8192 byte keys     4451.394 c/h        0.543 c/b        1.840 b/c
    BeagleHash_32_112     16384 byte keys     8878.160 c/h        0.542 c/b        1.845 b/c
    BeagleHash_32_112     32768 byte keys    17739.524 c/h        0.541 c/b        1.847 b/c
    BeagleHash_32_112     65536 byte keys    35449.071 c/h        0.541 c/b        1.849 b/c
    BeagleHash_32_112        Average Bulk     1128.326 c/h        0.559 c/b        1.790 b/c
</pre>

## Differential Tests

This hashes strings of various sizes with relatively few bits set,
and then looks for excess collisions. Hash functions with a weak mix
function, or weak seeding will often fail these tests.

(NB: I don't know too much about these tests. More to come later.)

<pre>
    [[[ Differential Tests ]]] - BeagleHash_32_112

    Testing 8303632 up-to-5-bit differentials in 64-bit keys -> 32 bit hashes.
    1000 reps, 8303632000 total tests, expecting 1.93 random collisions..........
    6 total collisions, of which 6 single collisions were ignored

    Testing 11017632 up-to-4-bit differentials in 128-bit keys -> 32 bit hashes.
    1000 reps, 11017632000 total tests, expecting 2.57 random collisions..........
    2 total collisions, of which 2 single collisions were ignored

    Testing 2796416 up-to-3-bit differentials in 256-bit keys -> 32 bit hashes.
    1000 reps, 2796416000 total tests, expecting 0.65 random collisions..........
    0 total collisions, of which 0 single collisions were ignored
</pre>

## Avalanche Test

This tests that a hash meets the "strict avalanche criteria", which
is that when changing any single bit of the input (seed or key) we expect
about 50% of the output bits to change.

This is tested at various key lengths, including 0 byte keys, and stats
are calculated about how input bit affects the output, and whether the
results are random or not.

Passing tests look like this:

<pre>
    [[[ Avalanche Tests ]]] - BeagleHash_32_112 seed-bits: 112 hash-bits: 32

    Samples 1000000, expected error 0.00025600, confidence level 99.99994267%

    Testing   0-bit keys.......... ok.     worst-bit:   0.402% error-ratio: 1.012786e+00
    Testing   8-bit keys.......... ok.     worst-bit:   0.398% error-ratio: 9.612077e-01
    Testing  16-bit keys.......... ok.     worst-bit:   0.385% error-ratio: 9.728366e-01
    Testing  24-bit keys.......... ok.     worst-bit:   0.383% error-ratio: 1.011299e+00
    Testing  32-bit keys.......... ok.     worst-bit:   0.365% error-ratio: 9.807127e-01
    Testing  40-bit keys.......... ok.     worst-bit:   0.381% error-ratio: 9.642346e-01
    Testing  48-bit keys.......... ok.     worst-bit:   0.431% error-ratio: 1.014530e+00
    Testing  56-bit keys.......... ok.     worst-bit:   0.411% error-ratio: 1.004617e+00
    Testing  64-bit keys.......... ok.     worst-bit:   0.377% error-ratio: 9.840337e-01
    Testing  72-bit keys.......... ok.     worst-bit:   0.441% error-ratio: 9.754174e-01
    Testing  80-bit keys.......... ok.     worst-bit:   0.379% error-ratio: 1.019198e+00
    Testing  88-bit keys.......... ok.     worst-bit:   0.375% error-ratio: 9.997662e-01
    Testing  96-bit keys.......... ok.     worst-bit:   0.374% error-ratio: 9.804174e-01
    Testing 104-bit keys.......... ok.     worst-bit:   0.377% error-ratio: 9.984745e-01
    Testing 112-bit keys.......... ok.     worst-bit:   0.438% error-ratio: 9.695704e-01
    Testing 120-bit keys.......... ok.     worst-bit:   0.378% error-ratio: 1.003672e+00
    Testing 128-bit keys.......... ok.     worst-bit:   0.365% error-ratio: 1.017423e+00
    Testing 136-bit keys.......... ok.     worst-bit:   0.377% error-ratio: 9.985610e-01
    Testing 144-bit keys.......... ok.     worst-bit:   0.368% error-ratio: 9.937084e-01
    Testing 152-bit keys.......... ok.     worst-bit:   0.397% error-ratio: 1.000956e+00
</pre>

Worst bit shows how many percentages away from the expected 50/50 changed/unchanged ratio
we want to see. Anything under %1.00 is considered acceptable. A scaled expected error is
calculated for the test, and then the sum of the square of the difference from 0.5 is used
to calculate an "error ratio". A good hash function should have a ratio of very close to
or below 1. A bad hash function could have wildly higher numbers.

When tests fail two charts are produced which show how every bit of the input
affected every bit of the hash produced. One chart shows the distance from
the expected %50 change, and the other shows the g-test probability of getting
that result. Additionally each "row" and "column" is checked to ensure that
they are correct as well. The following is an example which shows a hash function
which really does not mix very well at all, with many input bits having little
if any affect on the output. Only the positions with "." in them are "ok".

<pre>
    [[[ Avalanche Tests ]]] - bernstein seed-bits: 32 hash-bits: 32

    Samples 1000000, expected error 0.00025600, confidence level 99.99994267%

    Testing  32-bit keys.......... not ok! worst-bit: 100.000% error-ratio: 7.624407e+05
                 +---------------------------------------------------+
                 |012345678901234567890123456789012345678901234567890|
                 +---------------------------------------------------+
    Scale:       |.1234567890abcdefghijklmnopqrstuvwxyzãäåêëîïðñôõöûü|
                 |üÿABCDEFGHIJKLMNOPQRSTUVWXYZÂÃÄÅÊËÑÔÕÖÛÜÝø¤*©®¶&%@#|
                 +---------------------------------------------------+
                 |pct diff from 50%: abs((0.5-(changed/reps))*2) *100|
                 +--------------------------------+
                 |0         1         2         3 |
                 |01234567890123456789012345678901|
                 +--------------------------------+
    seed     0.0 |#.üXÛ©&%1üXÖ6BmKÅÝ5öWcïUÕ*¶%@###|
    seed     1.0 |##.üXÖ©&%1üXÖ6BmKÅÝ5öWcïUÕ*¶%@##|
    seed     2.0 |###.üXÛ©&%1üXÖ6BnKÅÝ5öWcïUÕ*¶%@#|
    seed     3.0 |####.üXÖ©&%1üXÖ6BnKÅÝ5öWcïUÕ*¶%@|
    seed     4.0 |#####.üXÛ©&%1üXÖ7BmKÅÝ5öWcïUÕ*¶%|
    seed     5.0 |######.üXÛ©&%1üXÖ6BnKÅÝ5öWcïUÕ*¶|
    seed     6.0 |#######.üXÛ©&%1üXÖ6BmKÅÝ5öWcðUÕ*|
    seed     7.0 |########.üXÛ©&%1üXÖ6BmKÅÝ5õWcïUÕ|
    seed     8.0 |#########.üXÖ©&%1üXÖ6BnKÅÝ5öWcïU|
    seed     9.0 |##########.üXÖ©&%1üXÖ6BmKÅÝ5öWcï|
    seed    10.0 |###########.üXÛ©&%1üXÖ6BmKÅÝ5öWc|
    seed    11.0 |############.üXÖ©&%1üXÖ6BmKÅÝ5öW|
    seed    12.0 |#############.üXÖ©&%1üXÖ6BmKÅÝ5ö|
    seed    13.0 |##############.üXÖ©&%1üXÖ6BmKÅÝ5|
    seed    14.0 |###############.üXÖ©&%1üXÖ6BmKÅÝ|
    seed    15.0 |################.üXÛ©&%1üXÖ6BmKÅ|
    seed    16.0 |#################.üXÖ©&%1üXÖ6BmK|
    seed    17.0 |##################.üXÖ©&%1üXÖ6Bm|
    seed    18.0 |###################.üXÖ©&%1üXÖ6B|
    seed    19.0 |####################.üXÛ©&%1üXÖ6|
    seed    20.0 |#####################.üXÛ©&%1üXÖ|
    seed    21.0 |######################.üXÖ©&%1üX|
    seed    22.0 |#######################.üXÖ©&%1ü|
    seed    23.0 |########################.üXÖ©&%1|
    seed    24.0 |#########################.üXÛ©&%|
    seed    25.0 |##########################.üXÛ©&|
    seed    26.0 |###########################.üXÛ©|
    seed    27.0 |############################.üXÖ|
    seed    28.0 |#############################.üX|
    seed    29.0 |##############################.ü|
    seed    30.0 |###############################.|
    seed    31.0 |################################|
    key      0.0 |#.üXÛ©3AnKÅ9DmJÅ0ñVÕ*&%@########|
    key      1.0 |##.üXÛ©3AnKÅ0DmJÅ0ñVÕ*&%@#######|
    key      2.0 |###.üXÖ©3AnKÅ9DmJÅ0ñVÕ*&%@######|
    key      3.0 |####.üXÖ©3AnKÅ0DmJÅ0ñUÕ*&%@#####|
    key      4.0 |#####.üXÛ©3AnKÅ0DmJÅ0ñVÕ*&%@####|
    key      5.0 |######.üXÛ©3AnKÅ0DmJÅ0ñVÕ*&%@###|
    key      6.0 |#######.üXÛ©3AnKÅ9DmJÅ0ñVÕ*&%@##|
    key      7.0 |########.üXÛ©3AnKÅ0DmJÅ0ñVÕ*&%@#|
    key      8.0 |#.üXÖ©&2ûXÖ6õVÖ*&%@#############|
    key      9.0 |##.üXÖ©&2ûXÖ6õVÖ*&%@############|
    key     10.0 |###.üXÖ©&2ûXÖ7õVÖ*&%@###########|
    key     11.0 |####.üXÛ©&2ûXÖ6õVÖ*&%@##########|
    key     12.0 |#####.üXÛ©&2ûXÖ6õVÖ*&%@#########|
    key     13.0 |######.üXÛ©&2ûXÖ6õVÖ*&%@########|
    key     14.0 |#######.üXÖ©&2ûXÖ6õVÖ*&%@#######|
    key     15.0 |########.üXÖ©&2ûXÖ6õVÖ*&%@######|
    key     16.0 |#.üXÖ©3ûWÖ©&%@##################|
    key     17.0 |##.üXÛ©3öWÖ©&%@#################|
    key     18.0 |###.üXÖ©3ûWÖ©&%@################|
    key     19.0 |####.üXÖ©3öWÖ©&%@###############|
    key     20.0 |#####.üXÖ©3öWÖ*&%@##############|
    key     21.0 |######.üXÛ©3öWÖ©&%@#############|
    key     22.0 |#######.üXÖ©3öWÖ*&%@############|
    key     23.0 |########.üXÛ©3öWÖ©&%@###########|
    key     24.0 |#.üXÖ©&%@#######################|
    key     25.0 |##.üXÛ©&%@######################|
    key     26.0 |###.üXÖ©&%@#####################|
    key     27.0 |####.üXÖ©&%@####################|
    key     28.0 |#####.üXÛ©&%@###################|
    key     29.0 |######.üXÛ©&%@##################|
    key     30.0 |#######.üXÖ©&%@#################|
    key     31.0 |########.üXÖ©&%@################|
                 +--------------------------------+

                 +---------------------------------------------------+
                 |012345678901234567890123456789012345678901234567890|
                 +---------------------------------------------------+
    Scale:       |.1234567890abcdefghijklmnopqrstuvwxyzãäåêëîïðñôõöûü|
                 |üÿABCDEFGHIJKLMNOPQRSTUVWXYZÂÃÄÅÊËÑÔÕÖÛÜÝø¤*©®¶&%@#|
                 +---------------------------------------------------+
                 |scaled p-value above confidence level (zero is ok) |
                 +--------------------------------+
                 |0         1         2         3 |
                 |01234567890123456789012345678901|
                 +--------------------------------+
    seed     0.0 |#.##############################|
    seed     1.0 |##.######@######################|
    seed     2.0 |###.######@#####################|
    seed     3.0 |####.######@####################|
    seed     4.0 |#####.######@###################|
    seed     5.0 |######.#########################|
    seed     6.0 |#######.######@#################|
    seed     7.0 |########.######@################|
    seed     8.0 |#########.######@###############|
    seed     9.0 |##########.######@##############|
    seed    10.0 |###########.######@#############|
    seed    11.0 |############.###################|
    seed    12.0 |#############.######@###########|
    seed    13.0 |##############.#################|
    seed    14.0 |###############.######@#########|
    seed    15.0 |################.###############|
    seed    16.0 |#################.######@#######|
    seed    17.0 |##################.######@######|
    seed    18.0 |###################.######@#####|
    seed    19.0 |####################.###########|
    seed    20.0 |#####################.######@###|
    seed    21.0 |######################.######@##|
    seed    22.0 |#######################.######@#|
    seed    23.0 |########################.######@|
    seed    24.0 |#########################.######|
    seed    25.0 |##########################.#####|
    seed    26.0 |###########################.####|
    seed    27.0 |############################.###|
    seed    28.0 |#############################.##|
    seed    29.0 |##############################.#|
    seed    30.0 |###############################.|
    seed    31.0 |################################|
    key      0.0 |#.##############################|
    key      1.0 |##.#############################|
    key      2.0 |###.############################|
    key      3.0 |####.###########################|
    key      4.0 |#####.##########################|
    key      5.0 |######.#########################|
    key      6.0 |#######.########################|
    key      7.0 |########.#######################|
    key      8.0 |#.##############################|
    key      9.0 |##.#############################|
    key     10.0 |###.############################|
    key     11.0 |####.###########################|
    key     12.0 |#####.##########################|
    key     13.0 |######.#########################|
    key     14.0 |#######.########################|
    key     15.0 |########.#######################|
    key     16.0 |#.##############################|
    key     17.0 |##.#############################|
    key     18.0 |###.############################|
    key     19.0 |####.###########################|
    key     20.0 |#####.##########################|
    key     21.0 |######.#########################|
    key     22.0 |#######.########################|
    key     23.0 |########.#######################|
    key     24.0 |#.##############################|
    key     25.0 |##.#############################|
    key     26.0 |###.############################|
    key     27.0 |####.###########################|
    key     28.0 |#####.##########################|
    key     29.0 |######.#########################|
    key     30.0 |#######.########################|
    key     31.0 |########.#######################|
                 +--------------------------------+
    1985 of 2048 bits failed (96.92%) failed at 99.999943 confidence

        g-test: 100.000000%
        sum-error-square: 195.18480644
        key/seed errors: 64/64
        - seed bit 0 gtest probability not random: 100.0000 (10437402/21562598)
        - seed bit 1 gtest probability not random: 100.0000 (10435821/21564179)
        - key bit 0 gtest probability not random: 100.0000 (9429356/22570644)
        - key bit 1 gtest probability not random: 100.0000 (9429964/22570036)
        - with 30 more seed errors and 30 more key errors not described above.
        hash bit-level errors: 32/32
        - hash bit 0 gtest-prob not-random: 100.0000 (5000000/59000000)
        - hash bit 1 gtest-prob not-random: 100.0000 (7500126/56499874)
        - hash bit 2 gtest-prob not-random: 100.0000 (8749735/55250265)
        - with 29 more hash bit errors not described above.
</pre>

## Cyclic Tests

This tests various cyclic keys to ensure that they produce
the expected collision counts.

<pre>
    [[[ Keyset 'Cyclic' Tests ]]] - BeagleHash_32_112

    Keyset 'Cyclic' - 8 cycles of 4 bytes - 10000000 keys
    Testing collisions   - Expected 11641.53, actual 11598.00 ( 1.00x)
    Testing distribution - ok. (99.999943 confidence)

    Keyset 'Cyclic' - 8 cycles of 5 bytes - 10000000 keys
    Testing collisions   - Expected 11641.53, actual 11801.00 ( 1.01x)
    Testing distribution - ok. (99.999943 confidence)

    Keyset 'Cyclic' - 8 cycles of 6 bytes - 10000000 keys
    Testing collisions   - Expected 11641.53, actual 11732.00 ( 1.01x)
    Testing distribution - ok. (99.999943 confidence)

    Keyset 'Cyclic' - 8 cycles of 7 bytes - 10000000 keys
    Testing collisions   - Expected 11641.53, actual 11759.00 ( 1.01x)
    Testing distribution - ok. (99.999943 confidence)

    Keyset 'Cyclic' - 8 cycles of 8 bytes - 10000000 keys
    Testing collisions   - Expected 11641.53, actual 11542.00 ( 0.99x)
    Testing distribution - ok. (99.999943 confidence)
</pre>

## TwoBytes

This tests keys composed of 2-character tuples repeated to various lengths.

<pre>
    [[[ Keyset 'TwoBytes' Tests ]]] - BeagleHash_32_112

    Keyset 'TwoBytes' - up-to-4-byte keys, 652545 total keys
    Testing collisions   - Expected    49.57, actual    44.00 ( 0.89x)
    Testing distribution - ok. (99.999943 confidence)

    Keyset 'TwoBytes' - up-to-8-byte keys, 5471025 total keys
    Testing collisions   - Expected  3484.56, actual  3520.00 ( 1.01x)
    Testing distribution - ok. (99.999943 confidence)

    Keyset 'TwoBytes' - up-to-12-byte keys, 18616785 total keys
    Testing collisions   - Expected 40347.77, actual 40516.00 ( 1.00x)
    Testing distribution - ok. (99.999943 confidence)

    Keyset 'TwoBytes' - up-to-16-byte keys, 44251425 total keys
    Testing collisions   - Expected 227963.15, actual 227111.00 ( 1.00x)
    Testing distribution - ok. (99.999943 confidence)

    Keyset 'TwoBytes' - up-to-20-byte keys, 86536545 total keys
    Testing collisions   - Expected 871784.70, actual 865018.00 ( 0.99x)
    Testing distribution - ok. (99.999943 confidence)
</pre>

## Sparse Tests

This tests strings of various lengths with very few bits set. I have
found This test is relatively sensitive. It will often fail when the
other tests do not, especially when a hash has a weak mixing function.

<pre>
    [[[ Keyset 'Sparse' Tests ]]] - BeagleHash_32_112

    Keyset 'Sparse' - 32-bit keys with up to 6 bits set - 1149017 keys
    Testing collisions   - Expected   153.70, actual   167.00 ( 1.09x)
    Testing distribution - ok. (99.999943 confidence)

    Keyset 'Sparse' - 40-bit keys with up to 6 bits set - 4598479 keys
    Testing collisions   - Expected  2461.72, actual  2481.00 ( 1.01x)
    Testing distribution - ok. (99.999943 confidence)

    Keyset 'Sparse' - 48-bit keys with up to 5 bits set - 1925357 keys
    Testing collisions   - Expected   431.55, actual   400.00 ( 0.93x)
    Testing distribution - ok. (99.999943 confidence)

    Keyset 'Sparse' - 56-bit keys with up to 5 bits set - 4216423 keys
    Testing collisions   - Expected  2069.66, actual  2104.00 ( 1.02x)
    Testing distribution - ok. (99.999943 confidence)

    Keyset 'Sparse' - 64-bit keys with up to 5 bits set - 8303633 keys
    Testing collisions   - Expected  8026.87, actual  8097.00 ( 1.01x)
    Testing distribution - ok. (99.999943 confidence)

    Keyset 'Sparse' - 96-bit keys with up to 4 bits set - 3469497 keys
    Testing collisions   - Expected  1401.34, actual  1386.00 ( 0.99x)
    Testing distribution - ok. (99.999943 confidence)

    Keyset 'Sparse' - 256-bit keys with up to 3 bits set - 2796417 keys
    Testing collisions   - Expected   910.36, actual   974.00 ( 1.07x)
    Testing distribution - ok. (99.999943 confidence)

    Keyset 'Sparse' - 2048-bit keys with up to 2 bits set - 2098177 keys
    Testing collisions   - Expected   512.50, actual   517.00 ( 1.01x)
    Testing distribution - ok. (99.999943 confidence)
</pre>

## Combination tests

I don't know what these tests do in detail yet.

<pre>
    [[[ Keyset 'Combination Lowbits' Tests ]]] - BeagleHash_32_112

    Keyset 'Combination' - up to 8 blocks from a set of 8 - 19173960 keys
    Testing collisions   - Expected 42799.01, actual 43179.00 ( 1.01x)
    Testing distribution - ok. (99.999943 confidence)


    [[[ Keyset 'Combination Highbits' Tests ]]] - BeagleHash_32_112

    Keyset 'Combination' - up to 8 blocks from a set of 8 - 19173960 keys
    Testing collisions   - Expected 42799.01, actual 42912.00 ( 1.00x)
    Testing distribution - ok. (99.999943 confidence)


    [[[ Keyset 'Combination 0x8000000' Tests ]]] - BeagleHash_32_112

    Keyset 'Combination' - up to 20 blocks from a set of 2 - 2097150 keys
    Testing collisions   - Expected   512.00, actual   536.00 ( 1.05x)
    Testing distribution - ok. (99.999943 confidence)


    [[[ Keyset 'Combination 0x0000001' Tests ]]] - BeagleHash_32_112

    Keyset 'Combination' - up to 20 blocks from a set of 2 - 2097150 keys
    Testing collisions   - Expected   512.00, actual   479.00 ( 0.94x)
    Testing distribution - ok. (99.999943 confidence)


    [[[ Keyset 'Combination Hi-Lo' Tests ]]] - BeagleHash_32_112

    Keyset 'Combination' - up to 6 blocks from a set of 15 - 12204240 keys
    Testing collisions   - Expected 17339.30, actual 17502.00 ( 1.01x)
    Testing distribution - ok. (99.999943 confidence)
</pre>

## Window Tests

These test that keys of various sizes produce hashes with
good distributions across all of the bit ranges in the hash.

The exact validity of these tests is yet to be determined. Failing them
is definitely bad, but passing these tests does not necessarily say much.

Currently the rule for failing a test is: the g-test of the bucket distribution
must exceed the required confidence level, AND the distribution must produce a
bad quality score. I have tested using the g-test alone and even hash functions
like sha1 fail the tests, on the other hand, what the g-test considers non-random
the quality score metric may consider within tolerance. These test need
further mathematical rigour.

<pre>
    [[[ Keyset 'Window' Tests ]]] - BeagleHash_32_112

    Keyset 'Windowed' -  64-bit key,  20-bit window - 64 tests, 1048576 keys per test
    Window at   0 - Testing collisions   - Expected   128.00, actual   147.00 ( 1.15x)
    Testing distribution - ok. (99.999943 confidence)
    Window at   1 - Testing collisions   - Expected   128.00, actual   127.00 ( 0.99x)
    Testing distribution - ok. (99.999943 confidence)
    Window at   2 - Testing collisions   - Expected   128.00, actual   131.00 ( 1.02x)
    Testing distribution - ok. (99.999943 confidence)
    Window at   3 - Testing collisions   - Expected   128.00, actual   127.00 ( 0.99x)
    Testing distribution - ok. (99.999943 confidence)
    Window at   4 - Testing collisions   - Expected   128.00, actual   137.00 ( 1.07x)
    Testing distribution - ok. (99.999943 confidence)
    Window at   5 - Testing collisions   - Expected   128.00, actual   139.00 ( 1.09x)
    Testing distribution - ok. (99.999943 confidence)
    Window at   6 - Testing collisions   - Expected   128.00, actual   141.00 ( 1.10x)
    Testing distribution - ok. (99.999943 confidence)
    Window at   7 - Testing collisions   - Expected   128.00, actual   129.00 ( 1.01x)
    Testing distribution - ok. (99.999943 confidence)
    Window at   8 - Testing collisions   - Expected   128.00, actual   122.00 ( 0.95x)
    Testing distribution - ok. (99.999943 confidence)
    Window at   9 - Testing collisions   - Expected   128.00, actual   122.00 ( 0.95x)
    Testing distribution - ok. (99.999943 confidence)
    Window at  10 - Testing collisions   - Expected   128.00, actual   129.00 ( 1.01x)
    Testing distribution - ok. (99.999943 confidence)
    Window at  11 - Testing collisions   - Expected   128.00, actual   118.00 ( 0.92x)
    Testing distribution - ok. (99.999943 confidence)
    Window at  12 - Testing collisions   - Expected   128.00, actual   133.00 ( 1.04x)
    Testing distribution - ok. (99.999943 confidence)
    Window at  13 - Testing collisions   - Expected   128.00, actual   126.00 ( 0.98x)
    Testing distribution - ok. (99.999943 confidence)
    Window at  14 - Testing collisions   - Expected   128.00, actual   136.00 ( 1.06x)
    Testing distribution - ok. (99.999943 confidence)
    Window at  15 - Testing collisions   - Expected   128.00, actual   121.00 ( 0.95x)
    Testing distribution - ok. (99.999943 confidence)
    Window at  16 - Testing collisions   - Expected   128.00, actual   126.00 ( 0.98x)
    Testing distribution - ok. (99.999943 confidence)
    Window at  17 - Testing collisions   - Expected   128.00, actual   126.00 ( 0.98x)
    Testing distribution - ok. (99.999943 confidence)
    Window at  18 - Testing collisions   - Expected   128.00, actual   104.00 ( 0.81x)
    Testing distribution - ok. (99.999943 confidence)
    Window at  19 - Testing collisions   - Expected   128.00, actual   128.00 ( 1.00x)
    Testing distribution - ok. (99.999943 confidence)
    Window at  20 - Testing collisions   - Expected   128.00, actual   135.00 ( 1.05x)
    Testing distribution - ok. (99.999943 confidence)
    Window at  21 - Testing collisions   - Expected   128.00, actual   151.00 ( 1.18x)
    Testing distribution - ok. (99.999943 confidence)
    Window at  22 - Testing collisions   - Expected   128.00, actual   128.00 ( 1.00x)
    Testing distribution - ok. (99.999943 confidence)
    Window at  23 - Testing collisions   - Expected   128.00, actual   128.00 ( 1.00x)
    Testing distribution - ok. (99.999943 confidence)
    Window at  24 - Testing collisions   - Expected   128.00, actual   114.00 ( 0.89x)
    Testing distribution - ok. (99.999943 confidence)
    Window at  25 - Testing collisions   - Expected   128.00, actual   106.00 ( 0.83x)
    Testing distribution - ok. (99.999943 confidence)
    Window at  26 - Testing collisions   - Expected   128.00, actual   124.00 ( 0.97x)
    Testing distribution - ok. (99.999943 confidence)
    Window at  27 - Testing collisions   - Expected   128.00, actual   139.00 ( 1.09x)
    Testing distribution - ok. (99.999943 confidence)
    Window at  28 - Testing collisions   - Expected   128.00, actual   134.00 ( 1.05x)
    Testing distribution - ok. (99.999943 confidence)
    Window at  29 - Testing collisions   - Expected   128.00, actual   147.00 ( 1.15x)
    Testing distribution - ok. (99.999943 confidence)
    Window at  30 - Testing collisions   - Expected   128.00, actual   135.00 ( 1.05x)
    Testing distribution - ok. (99.999943 confidence)
    Window at  31 - Testing collisions   - Expected   128.00, actual   146.00 ( 1.14x)
    Testing distribution - ok. (99.999943 confidence)
    Window at  32 - Testing collisions   - Expected   128.00, actual   130.00 ( 1.02x)
    Testing distribution - ok. (99.999943 confidence)
    Window at  33 - Testing collisions   - Expected   128.00, actual   106.00 ( 0.83x)
    Testing distribution - ok. (99.999943 confidence)
    Window at  34 - Testing collisions   - Expected   128.00, actual   112.00 ( 0.88x)
    Testing distribution - ok. (99.999943 confidence)
    Window at  35 - Testing collisions   - Expected   128.00, actual   125.00 ( 0.98x)
    Testing distribution - ok. (99.999943 confidence)
    Window at  36 - Testing collisions   - Expected   128.00, actual   125.00 ( 0.98x)
    Testing distribution - ok. (99.999943 confidence)
    Window at  37 - Testing collisions   - Expected   128.00, actual   122.00 ( 0.95x)
    Testing distribution - ok. (99.999943 confidence)
    Window at  38 - Testing collisions   - Expected   128.00, actual   115.00 ( 0.90x)
    Testing distribution - ok. (99.999943 confidence)
    Window at  39 - Testing collisions   - Expected   128.00, actual   117.00 ( 0.91x)
    Testing distribution - ok. (99.999943 confidence)
    Window at  40 - Testing collisions   - Expected   128.00, actual   119.00 ( 0.93x)
    Testing distribution - ok. (99.999943 confidence)
    Window at  41 - Testing collisions   - Expected   128.00, actual   130.00 ( 1.02x)
    Testing distribution - ok. (99.999943 confidence)
    Window at  42 - Testing collisions   - Expected   128.00, actual   130.00 ( 1.02x)
    Testing distribution - ok. (99.999943 confidence)
    Window at  43 - Testing collisions   - Expected   128.00, actual   133.00 ( 1.04x)
    Testing distribution - ok. (99.999943 confidence)
    Window at  44 - Testing collisions   - Expected   128.00, actual   132.00 ( 1.03x)
    Testing distribution - ok. (99.999943 confidence)
    Window at  45 - Testing collisions   - Expected   128.00, actual   120.00 ( 0.94x)
    Testing distribution - ok. (99.999943 confidence)
    Window at  46 - Testing collisions   - Expected   128.00, actual   132.00 ( 1.03x)
    Testing distribution - ok. (99.999943 confidence)
    Window at  47 - Testing collisions   - Expected   128.00, actual   130.00 ( 1.02x)
    Testing distribution - ok. (99.999943 confidence)
    Window at  48 - Testing collisions   - Expected   128.00, actual   134.00 ( 1.05x)
    Testing distribution - ok. (99.999943 confidence)
    Window at  49 - Testing collisions   - Expected   128.00, actual   137.00 ( 1.07x)
    Testing distribution - ok. (99.999943 confidence)
    Window at  50 - Testing collisions   - Expected   128.00, actual   140.00 ( 1.09x)
    Testing distribution - ok. (99.999943 confidence)
    Window at  51 - Testing collisions   - Expected   128.00, actual   125.00 ( 0.98x)
    Testing distribution - ok. (99.999943 confidence)
    Window at  52 - Testing collisions   - Expected   128.00, actual   120.00 ( 0.94x)
    Testing distribution - ok. (99.999943 confidence)
    Window at  53 - Testing collisions   - Expected   128.00, actual   111.00 ( 0.87x)
    Testing distribution - ok. (99.999943 confidence)
    Window at  54 - Testing collisions   - Expected   128.00, actual   120.00 ( 0.94x)
    Testing distribution - ok. (99.999943 confidence)
    Window at  55 - Testing collisions   - Expected   128.00, actual   115.00 ( 0.90x)
    Testing distribution - ok. (99.999943 confidence)
    Window at  56 - Testing collisions   - Expected   128.00, actual   117.00 ( 0.91x)
    Testing distribution - ok. (99.999943 confidence)
    Window at  57 - Testing collisions   - Expected   128.00, actual   123.00 ( 0.96x)
    Testing distribution - ok. (99.999943 confidence)
    Window at  58 - Testing collisions   - Expected   128.00, actual   123.00 ( 0.96x)
    Testing distribution - ok. (99.999943 confidence)
    Window at  59 - Testing collisions   - Expected   128.00, actual   141.00 ( 1.10x)
    Testing distribution - ok. (99.999943 confidence)
    Window at  60 - Testing collisions   - Expected   128.00, actual   132.00 ( 1.03x)
    Testing distribution - ok. (99.999943 confidence)
    Window at  61 - Testing collisions   - Expected   128.00, actual   136.00 ( 1.06x)
    Testing distribution - ok. (99.999943 confidence)
    Window at  62 - Testing collisions   - Expected   128.00, actual   145.00 ( 1.13x)
    Testing distribution - ok. (99.999943 confidence)
    Window at  63 - Testing collisions   - Expected   128.00, actual   139.00 ( 1.09x)
    Testing distribution - ok. (99.999943 confidence)
    Window at  64 - Testing collisions   - Expected   128.00, actual   147.00 ( 1.15x)
    Testing distribution - ok. (99.999943 confidence)
</pre>

## Text Tests

This tests various simple string keys of specific forms
at different lengths and ensures they do not produce excessive
collisions or bad distributions.

<pre>
    [[[ Keyset 'Text' Tests ]]] - BeagleHash_32_112

    Keyset 'Text' - keys of form "Foo[XXXX]Bar" - 14776336 keys
    Testing collisions   - Expected 25418.13, actual 25273.00 ( 0.99x)
    Testing distribution - ok. (99.999943 confidence)

    Keyset 'Text' - keys of form "FooBar[XXXX]" - 14776336 keys
    Testing collisions   - Expected 25418.13, actual 25079.00 ( 0.99x)
    Testing distribution - ok. (99.999943 confidence)

    Keyset 'Text' - keys of form "[XXXX]FooBar" - 14776336 keys
    Testing collisions   - Expected 25418.13, actual 25330.00 ( 1.00x)
    Testing distribution - ok. (99.999943 confidence)
</pre>

## Zeroes Test

Tests keys consisting of only the null byte of various lengths.

<pre>
    [[[ Keyset 'Zeroes' Tests ]]] - BeagleHash_32_112

    Keyset 'Zeroes' - 262144 keys
    Testing collisions   - Expected     8.00, actual     8.00 ( 1.00x)
    Testing distribution - ok. (99.999943 confidence)
</pre>

## Seed Tests

Tests hashing the same key with many different and unique seeds and
then ensures that the result has a good distribution, and collision rate.

<pre>
    [[[ Keyset 'Seed' Tests ]]] - BeagleHash_32_112

    Keyset 'Seed' - 2000000 seeds, key length 43
      Key "The quick brown fox jumps over the lazy dog"
    Testing collisions   - Expected   465.66, actual   452.00 ( 0.97x)
    Testing distribution - ok. (99.999943 confidence)

    Keyset 'Seed' - 2000000 seeds, key length 0
      Key ""
    Testing collisions   - Expected   465.66, actual   479.00 ( 1.03x)
    Testing distribution - ok. (99.999943 confidence)

    Keyset 'Seed' - 2000000 seeds, key length 17
      Key "00101100110101101"
    Testing collisions   - Expected   465.66, actual   471.00 ( 1.01x)
    Testing distribution - ok. (99.999943 confidence)

    Keyset 'Seed' - 2000000 seeds, key length 60
      Key "abcbcddbdebdcaaabaaababaaabacbeedbabseeeeeeeesssssseeeewwwww"
    Testing collisions   - Expected   465.66, actual   388.00 ( 0.83x)
    Testing distribution - ok. (99.999943 confidence)
</pre>

## Effs Test

Similar to the Zeros test, this verifies that hashing keys of ever
longer sequences of 0xFF bytes does not produce excess collisions and
produces good distributions.

<pre>
    [[[ Keyset 'Effs' Tests ]]] - BeagleHash_32_112

    Keyset 'Effs' - 262144 keys
    Testing collisions   - Expected     8.00, actual     6.00 ( 0.75x)
    Testing distribution - ok. (99.999943 confidence)
</pre>

# Collision Tests

These test that for a given set of keys the results produce the expected
number of collisions. For instance with 262144 keys a 32 bit hash should
produce about 8 collisions. The test will fail if the actual count exceeds
double that of expected. This is not very sensitive, and my be adjusted later.

# Distribution Tests

This checks that the hashes produce reasonable distributions.

XXX: Document exact rules.

## Using the g-test

Where possible I have changed smhasher to use the g-test for determining if
a distribution is non-random. The test is a more accurate version of the chi-square
test and can be used to calculate the probability that the distribution of N items
into M buckets is "random":

<img src="https://github.com/demerphq/smhasher/blob/master/doc/gtest.png?raw=true" />

or in pseudo code:

     g += v * log(v/(n/m))
        for each non-zero v in buckets_array

The g-value follows a chi-squared distribution with the same number of
degrees-of-freedom, so any function that can convert a chi-square value to
a probability can be used with the g-test. We use:

     1.0 - gsl_sf_gamma_inc_Q( ( double(m) - 1.0) / 2.0, g )

from the GNU scientific library.

Wikipedia has more info on the <a href="https://en.wikipedia.org/wiki/G-test">G-Test</a>
and the <a href="https://en.wikipedia.org/wiki/Chi-squared_distribution">Chi-Squared Distribution</a>.

Note that the standard C log() function suffers some interesting and subtle
cancellation errors when its argument is very close to and above 1, so the
actual code uses log1p() when v > n/m.

## Quality Score

The distribution tests have always used a "score" to determine if the distribution
was good. I did not understand the old score, and found it produced weird results,
so I replaced it with something that seemed similar but better known to me.

The following assumes we are hashing /n/ items into /m/ buckets.

The old score was

<img src="https://github.com/demerphq/smhasher/blob/master/doc/old_score.png?raw=true" />

which in pseudo-code is

    sum_sq += v * v
        for v in buckets_array
    old_score = 1.0 - ( ( ( n * n )  - 1.0 ) / ( sum_sq - n ) / m )

The new score is

<img src="https://github.com/demerphq/smhasher/blob/master/doc/new_score.png?raw=true" />

which in pseudo-code is

    qs_sum += ( v * ( v + 1.0 ) ) / 2.0
        for v in buckets_array
    quality_score = qs_sum / ( ( n / ( 2.0 * m ) ) *
                               ( n + ( 2.0 * m ) - 1.0 ) )
    score = abs( 1.0 - quality_score )

The quality score formula is from the Red Dragon book. A good hash function
should have a quality score of close to one, with values between 0.95 to 1.05 being
normal. In theory having a lower quality score is better, however excessively
low values indicate non-random behavior that probably indicates some
other weakness. Therefore our tests use the distance from 1 instead, and consider
anything higher than 0.01 to be a failure.

Further discussion on hash functions and quality scores can be found
<a href="http://www.strchr.com/hash_functions">here</a> (along with all kinds
of other valuable information).

