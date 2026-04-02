#include "kvstore.h"
#include <stdio.h>

int main(void)
{
    kvs_store_t *store = kvs_create(64);
    if (!store) {
        fprintf(stderr, "Failed to create store\n");
        return 1;
    }

    if (kvs_set(store, "hello", "world") != KVS_OK ||
        kvs_set(store, "redis", "fast") != KVS_OK) {
        fprintf(stderr, "Failed to set key\n");
        kvs_destroy(store);
        return 1;
    }

    printf("hello = %s\n", kvs_get(store, "hello"));
    printf("redis = %s\n", kvs_get(store, "redis"));
    printf("count = %zu\n", kvs_count(store));

    kvs_destroy(store);
    return 0;
}
