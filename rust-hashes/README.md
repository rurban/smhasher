# Rust hashes

This crate provides bindings for Rust hash algorithms. To use it, run
`cargo build --release`. This will generate `librust_hashes.a` in
`target/release`.

There are a few other build options:

```sh
# build with the current toolchain (1.68 minimum)
cargo build --release

# build with native SIMD instructions
RUSTFLAGS="-Ctarget-cpu=native" cargo build --release

# update hashes (requires a nightly toolchain)
RUST_GENERATE_BINDINGS=1 cargo +nightly build
```

`--release` can be omitted for faster test builds.

When building the entire smhasher peroject, cmake handles these options
