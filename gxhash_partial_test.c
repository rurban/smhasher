#include "gxhash.h"
#include <stdint.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

int main(void)
{
    long page = sysconf(_SC_PAGESIZE);
    if (page <= 0) return 1;
    uint8_t *memory = mmap(0, 2 * page, PROT_READ | PROT_WRITE,
                           MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (memory == MAP_FAILED || mprotect(memory + page, page, PROT_NONE)) return 1;
    for (int len = 1; len < 16; ++len) {
        uint8_t *key = memory + page - len;
        memset(key, 0, len);
        uint64_t original = gxhash64(key, len, 7);
        for (int i = 0; i < len; ++i) {
            key[i] = 1;
            uint64_t changed = gxhash64(key, len, 7);
            key[i] = 0;
            if (changed == original) return 1;
        }
    }
    return munmap(memory, 2 * page) != 0;
}
