// Fix 10: libFuzzer target for kvstore
// Build: cmake -DENABLE_FUZZING=ON -DCMAKE_C_COMPILER=clang
// Run:   ./fuzz_kvstore -max_total_time=30 -max_len=256
#include "../src/kvstore.h"
#include <stddef.h>
#include <stdint.h>
#include <string.h>

// libFuzzer entry point — no main(), linked with -fsanitize=fuzzer
int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    if (size < 3)
        return 0;

    // Small initial capacity forces resize paths to be exercised
    kvs_store_t *store = kvs_create(8);
    if (!store)
        return 0;

    // Interpret input as a stream of (op, key_len, key..., [val_len, val...]) records
    size_t i = 0;
    while (i + 2 < size) {
        uint8_t op = data[i++];
        uint8_t klen = (data[i++] % 32) + 1; // 1..32
        if (i + klen > size)
            break;

        char key[33] = {0};
        memcpy(key, data + i, klen);
        i += klen;

        switch (op % 3) {
        case 0: { // set
            if (i >= size)
                break;
            uint8_t vlen = (data[i++] % 32) + 1;
            size_t actual = (i + vlen <= size) ? vlen : (size - i);
            char val[33] = {0};
            memcpy(val, data + i, actual);
            i += actual;
            kvs_set(store, key, val);
            break;
        }
        case 1: // get
            kvs_get(store, key);
            break;
        case 2: // del
            kvs_del(store, key);
            break;
        }
    }

    kvs_destroy(store);
    return 0;
}
