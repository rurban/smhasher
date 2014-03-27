typedef void* fhtw ;

fhtw fhtw_new(int size);
void fhtw_free(fhtw);
int fhtw_set(fhtw, void* key, int key_len, void* value);
void* fhtw_get(fhtw, void* key, int key_len);
int fhtw_hash(void* key, int key_len);



