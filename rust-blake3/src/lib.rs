use std::slice;

#[no_mangle]
pub unsafe extern "C" fn smhasher_rust_blake3_digest(
    input: *const u8,
    len: usize,
    output: *mut u8,
) {
    let bytes = if len == 0 {
        &[]
    } else {
        slice::from_raw_parts(input, len)
    };
    let digest = blake3::hash(bytes);
    std::ptr::copy_nonoverlapping(digest.as_bytes().as_ptr(), output, 32);
}
