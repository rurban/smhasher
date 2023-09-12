//! Provide C interfaces to common Rust hash functions

use std::ffi::{c_int, c_uint, c_void};
use std::hash::{BuildHasher, Hasher};
use std::ops::FnOnce;
use std::ptr;
use std::slice;

use digest::{Digest, ExtendableOutput, XofReader};

/// Default domain separation
const TURBO_DOMAIN_SEP: u8 = 0x1f;

/// This maco does the following:
/// - Accepts a repeating list of `identifer: function`
/// - Creates a C function with that identifier
/// - Turn data into a Rust slice
/// - Calls the function
macro_rules! hashes {
    ($($fn_name:ident: $call_fn:expr),* $(,)?) => {$(
        #[no_mangle]
        pub unsafe extern "C" fn $fn_name(
            blob: *const c_void,
            len: c_int,
            seed: c_uint,
            out: *mut c_void
        ) {
            let len = len as usize;
            let data = unsafe { slice::from_raw_parts(blob.cast::<u8>(), len as usize) };
            $call_fn(data, seed, out);
        }
    )*};
}

/// Provide a simple wrapper for anything that implements `Hasher` and `Default`
fn hashdefault_wrapper<H>(data: &[u8], _seed: u32, out: *mut c_void)
where
    H: Default + Hasher,
{
    let mut hasher = H::default();
    hasher.write(data);
    unsafe { ptr::write(out.cast(), hasher.finish()) }
}

/// Provide a simple wrapper for any types that implement the `Digest` interface
fn digest_wrapper<H>(data: &[u8], _seed: u32, out: *mut c_void)
where
    H: Digest,
{
    let mut hash = H::new();
    hash.update(data);
    hash.finalize_into(unsafe { &mut *out.cast() });
}

/// Provide a wrapper for anything that implements `Hasher` and can take a seed
/// This function takes a hash constructor and returns a hash function
/// The constructor function (`C`) can take anything (`T`) that a u32 can
/// be automatically converted into (e.g. `u64`)
const fn seeded_wrapper<H, C, T>(new_fn: C) -> impl FnOnce(&[u8], u32, *mut c_void) + 'static
where
    C: FnOnce(T) -> H + 'static,
    T: From<u32>,
    H: Hasher,
{
    |data: &[u8], seed: u32, out: *mut c_void| {
        let mut hasher = new_fn(seed.into());
        hasher.write(data);
        unsafe { ptr::write(out.cast(), hasher.finish()) }
    }
}

/// Shake digests have a weird API signature since they return variable-length hashes.
/// So, this function takes a hash constructor function and returns a closure that
/// provides the signature we need.
///
/// To keep things simpler, we make the output size the same as the security
const fn extdigest_wrapper<H, const N: usize>(
    new_fn: impl FnOnce(u32) -> H + 'static,
) -> impl FnOnce(&[u8], u32, *mut c_void) + 'static
where
    H: ExtendableOutput,
{
    |data: &[u8], seed: u32, out: *mut c_void| {
        let mut hasher = new_fn(seed);
        hasher.update(data);
        let mut reader = hasher.finalize_xof();
        let out_ref: &mut [u8; N] = unsafe { &mut *out.cast() };
        reader.read(out_ref);
    }
}

// This macro creates the given symbol for each wrapper
hashes! {
    adler_rs: hashdefault_wrapper::<adler::Adler32>,
    ahash_rs: seeded_wrapper(|seed: u32|
        ahash::RandomState::with_seed(seed as usize).build_hasher()
    ),
    ascon_rs: digest_wrapper::<ascon_hash::AsconHash>,
    ascona_rs: digest_wrapper::<ascon_hash::AsconAHash>,
    blake2b512_rs: digest_wrapper::<blake2::Blake2b512>,
    blake2s256_rs: digest_wrapper::<blake2::Blake2s256>,
    blake3_rs: |buf, _seed, out: *mut c_void| unsafe {
        ptr::write(out.cast(), blake3::hash(buf));
    },
    crc32fast_rs: |buf, _seed, out: *mut c_void| unsafe {
        ptr::write(out.cast(), crc32fast::hash(buf));
    },
    fnvhash_rs: seeded_wrapper(fnv::FnvHasher::with_key),
    fxhash32_rs: hashdefault_wrapper::<fxhash::FxHasher32>,
    fxhash64_rs: hashdefault_wrapper::<fxhash::FxHasher64>,
    hash_hasher_rs: hashdefault_wrapper::<hash_hasher::HashHasher>,
    highway_rs: seeded_wrapper(|seed: u32|
        highway::HighwayHasher::new(highway::Key([seed.into(), 0, 0, 0]))
    ),
    md5_rs: digest_wrapper::<md5::Md5>,
    ripemd128_rs: digest_wrapper::<ripemd::Ripemd128>,
    ripemd160_rs: digest_wrapper::<ripemd::Ripemd160>,
    ripemd256_rs: digest_wrapper::<ripemd::Ripemd256>,
    ripemd320_rs: digest_wrapper::<ripemd::Ripemd320>,
    sha1_rs: digest_wrapper::<sha1::Sha1>,
    sha224_rs: digest_wrapper::<sha2::Sha224>,
    sha256_rs: digest_wrapper::<sha2::Sha256>,
    sha384_rs: digest_wrapper::<sha2::Sha384>,
    sha512_rs: digest_wrapper::<sha2::Sha512>,
    sha512_224_rs: digest_wrapper::<sha2::Sha512_224>,
    sha512_256_rs: digest_wrapper::<sha2::Sha512_256>,
    cshake128_rs: extdigest_wrapper::<sha3::CShake128, 16>(
        |seed| sha3::CShake128::from_core(sha3::CShake128Core::new(&seed.to_ne_bytes()))
    ),
    cshake256_rs: extdigest_wrapper::<sha3::CShake256, 32>(
        |seed| sha3::CShake256::from_core(sha3::CShake256Core::new(&seed.to_ne_bytes()))
    ),
    keccak224_rs: digest_wrapper::<sha3::Keccak224>,
    keccak256_rs: digest_wrapper::<sha3::Keccak256>,
    keccak256full_rs: digest_wrapper::<sha3::Keccak256Full>,
    keccak384_rs: digest_wrapper::<sha3::Keccak384>,
    keccak512_rs: digest_wrapper::<sha3::Keccak512>,
    sha3_224_rs: digest_wrapper::<sha3::Sha3_224>,
    sha3_256_rs: digest_wrapper::<sha3::Sha3_256>,
    sha3_384_rs: digest_wrapper::<sha3::Sha3_384>,
    sha3_512_rs: digest_wrapper::<sha3::Sha3_512>,
    shake128_rs: extdigest_wrapper::<sha3::Shake128, 16>(|_buf| sha3::Shake128::default()),
    shake256_rs: extdigest_wrapper::<sha3::Shake256, 32>(|_buf| sha3::Shake256::default()),
    turboshake128_1f_rs: extdigest_wrapper::<sha3::TurboShake128, 16>(|_seed|
        sha3::TurboShake128::from_core(sha3::TurboShake128Core::new(TURBO_DOMAIN_SEP))
    ),
    turboshake256_1f_rs: extdigest_wrapper::<sha3::TurboShake256, 32>(|_seed|
        sha3::TurboShake256::from_core(sha3::TurboShake256Core::new(TURBO_DOMAIN_SEP))
    ),
    twox_xxhash32_rs: seeded_wrapper(twox_hash::XxHash32::with_seed),
    twox_xxhash64_rs: seeded_wrapper(twox_hash::XxHash64::with_seed),
    siphash13_rs: seeded_wrapper(|seed: u32|
        siphasher::sip::SipHasher13::new_with_keys(seed.into(), 0)
    ),
    siphash24_rs: seeded_wrapper(|seed: u32|
        siphasher::sip::SipHasher24::new_with_keys(seed.into(), 0)
    ),
    siphash128_13_rs: seeded_wrapper(|seed: u32|
        siphasher::sip128::SipHasher13::new_with_keys(seed.into(), 0)
    ),
    siphash128_24_rs: seeded_wrapper(|seed: u32|
        siphasher::sip128::SipHasher24::new_with_keys(seed.into(), 0)
    ),
    whirlpool_rs: digest_wrapper::<whirlpool::Whirlpool>,
    wyhash_rs: seeded_wrapper(wyhash::WyHash::with_seed),
    xxhash3_rs: seeded_wrapper(xxhash_rust::xxh3::Xxh3::with_seed),
    xxhash128_rs: |buf, _seed, out: *mut c_void| unsafe {
        ptr::write(out.cast(), xxhash_rust::xxh3::xxh3_128(buf));
    },
    xxhash32_rs: |buf, seed, out: *mut c_void| unsafe {
        ptr::write(out.cast(), xxhash_rust::xxh32::xxh32(buf, seed));
    },
    xxhash64_rs: seeded_wrapper(xxhash_rust::xxh64::Xxh64::new),
}
