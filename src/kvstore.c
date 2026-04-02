// Fix 1: strdup replaced with portable kvs_strdup (no POSIX dependency)
// Fix 1: hash table now resizes at 70% load instead of rejecting inserts

#include "kvstore.h"
#include <stdlib.h>
#include <string.h>

/* Portable strdup — C11 standard, no POSIX required */
static char *kvs_strdup(const char *s)
{
    size_t len = strlen(s) + 1;
    char *copy = malloc(len);
    if (copy)
        memcpy(copy, s, len);
    return copy;
}

// DJB2 hash — simple, fast, good distribution for short strings
static unsigned long hash_djb2(const char *str)
{
    unsigned long hash = 5381;
    int c;
    while ((c = (unsigned char)*str++))
        hash = ((hash << 5) + hash) + (unsigned long)c;
    return hash;
}

kvs_store_t *kvs_create(size_t initial_capacity)
{
    if (initial_capacity == 0)
        initial_capacity = 64;

    kvs_store_t *store = calloc(1, sizeof(kvs_store_t));
    if (!store)
        return NULL;

    store->entries = calloc(initial_capacity, sizeof(kvs_entry_t));
    if (!store->entries) {
        free(store);
        return NULL;
    }

    store->capacity = initial_capacity;
    store->size = 0;
    return store;
}

void kvs_destroy(kvs_store_t *store)
{
    if (!store)
        return;
    for (size_t i = 0; i < store->capacity; i++) {
        if (store->entries[i].occupied) {
            free(store->entries[i].key);
            free(store->entries[i].value);
        }
    }
    free(store->entries);
    free(store);
}

// Find slot: returns index of existing key or first empty slot
static size_t find_slot(const kvs_store_t *store, const char *key)
{
    size_t idx = hash_djb2(key) % store->capacity;
    while (store->entries[idx].occupied) {
        if (strcmp(store->entries[idx].key, key) == 0)
            return idx;
        idx = (idx + 1) % store->capacity;
    }
    return idx;
}

// Double capacity and rehash all entries
static int kvs_resize(kvs_store_t *store)
{
    size_t new_capacity = store->capacity * 2;
    kvs_entry_t *new_entries = calloc(new_capacity, sizeof(kvs_entry_t));
    if (!new_entries)
        return KVS_ERR;

    for (size_t i = 0; i < store->capacity; i++) {
        if (!store->entries[i].occupied)
            continue;
        size_t idx = hash_djb2(store->entries[i].key) % new_capacity;
        while (new_entries[idx].occupied)
            idx = (idx + 1) % new_capacity;
        new_entries[idx] = store->entries[i];
    }

    free(store->entries);
    store->entries = new_entries;
    store->capacity = new_capacity;
    return KVS_OK;
}

int kvs_set(kvs_store_t *store, const char *key, const char *value)
{
    if (!store || !key || !value)
        return KVS_ERR;

    // Resize when load factor reaches 70%
    if (store->size * 10 >= store->capacity * 7) {
        if (kvs_resize(store) != KVS_OK)
            return KVS_ERR;
    }

    size_t idx = find_slot(store, key);

    if (store->entries[idx].occupied) {
        // Update existing key
        free(store->entries[idx].value);
        store->entries[idx].value = kvs_strdup(value);
        return store->entries[idx].value ? KVS_OK : KVS_ERR;
    }

    // Insert new key
    store->entries[idx].key = kvs_strdup(key);
    store->entries[idx].value = kvs_strdup(value);
    store->entries[idx].occupied = 1;

    if (!store->entries[idx].key || !store->entries[idx].value) {
        free(store->entries[idx].key);
        free(store->entries[idx].value);
        store->entries[idx].key = NULL;
        store->entries[idx].value = NULL;
        store->entries[idx].occupied = 0;
        return KVS_ERR;
    }

    store->size++;
    return KVS_OK;
}

const char *kvs_get(const kvs_store_t *store, const char *key)
{
    if (!store || !key)
        return NULL;
    size_t idx = find_slot(store, key);
    if (store->entries[idx].occupied)
        return store->entries[idx].value;
    return NULL;
}

int kvs_del(kvs_store_t *store, const char *key)
{
    if (!store || !key)
        return KVS_ERR;
    size_t idx = find_slot(store, key);
    if (!store->entries[idx].occupied)
        return KVS_NOT_FOUND;

    free(store->entries[idx].key);
    free(store->entries[idx].value);
    store->entries[idx].key = NULL;
    store->entries[idx].value = NULL;
    store->entries[idx].occupied = 0;
    store->size--;
    return KVS_OK;
}

size_t kvs_count(const kvs_store_t *store)
{
    return store ? store->size : 0;
}
