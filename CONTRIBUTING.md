# Contributing to smhasher

First off, thanks for taking the time to contribute!

## No Code of Conduct

This project adheres to [No Code of Conduct](https://github.com/domgetter/NCoC#what-is-ncoc).
We are all adults.
We accept anyone's contributions.
If you are feeling harassed speak up by yourself.
We do not censor, we don't stop any discussions.
Nothing else matters.

## New hash functions

We take functions written in C++, C and sometimes even in ASM.
The smokers run on travis (x64 - Linux and darwin, x86, aarch64), appveyor (MSVC 17),
and cirrus for FreeBSD 12.1.
So g++ and clang++, linux, darwin, freebsd and Windows MSVC is tested, no mingw, no arm32 (yet).
Note that Windows MSVC is only C89 or C++, but not C99, and a long is not long.
Use stdint.h

`git add submodule` is fine.

## New tests

Please check that a change should not exceed the test function >5min,
esp. for >=64bit hashes. A 2hr test (e.g. for 128bit) is unacceptable.
We don't want to wait 4 days to run through all test for all hashes.
Adjust the window length or repetition count on `info->hashbits` then.
My testmachine is a MacAir 2013 i7-4650U with 8GB RAM on darwin, which has a
faster CPU and libc than my big linux machine.

Hide longer tests behind `--extra` (`g_testExtra`).

64bit hashes don't have collisions.
A practically useful test tests only the 8 lowest bits, as they are the
most common bits uses in a hash table. The highest bits are usually better, thus
rather test the lower bits.

Every test which breaks a hash function is appreciated.
Look at the random crusher tests also (TestU01, Dieharder).
