
#ifdef __cplusplus
extern "C" {
#endif
void tsip_seed_state_smhasher_test(int in_bits, const void *seed, void *state);
void tsip_hash_with_state_smhasher_test(const void *key, STRLEN len, const void *state, void *out);
#ifdef __cplusplus
}
#endif

