// kvstore.h — Mini in-memory key-value store
// NOT thread-safe. External synchronization required for concurrent access.
#ifndef KVSTORE_H
#define KVSTORE_H

#include <stddef.h>

#define KVS_OK 0
#define KVS_ERR (-1)
#define KVS_NOT_FOUND (-2)

typedef struct {
    char *key;
    char *value;
    int occupied;
} kvs_entry_t;

typedef struct {
    kvs_entry_t *entries;
    size_t capacity;
    size_t size;
} kvs_store_t;

// Lifecycle
kvs_store_t *kvs_create(size_t initial_capacity);
void kvs_destroy(kvs_store_t *store);

// Operations
int kvs_set(kvs_store_t *store, const char *key, const char *value);
const char *kvs_get(const kvs_store_t *store, const char *key);
int kvs_del(kvs_store_t *store, const char *key);
size_t kvs_count(const kvs_store_t *store);

#endif
