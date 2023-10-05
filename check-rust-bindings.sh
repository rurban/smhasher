#!/bin/sh
cd rust-hashes
if [ -f rust_hashes.h ]; then
    mv rust_hashes.h committed.h
    # regenerate the bindings
    RUST_GENERATE_BINDINGS=1 cargo +nightly build || \
        (mv committed.h rust_hashes.h; exit 1)
    diff -up committed.h rust_hashes.h
    if [ $? -eq 0 ]; then
        echo "rust_hashes.h matches"
    else
        echo "rust_hashes.h changed or does not match"
        echo "regenerate them with `RUST_GENERATE_BINDINGS=1 cargo +nightly build`"
        exit 1
    fi
fi
