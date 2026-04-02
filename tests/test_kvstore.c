// Minimal test framework — no external deps needed
#include "../src/kvstore.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static int tests_run = 0;
static int tests_passed = 0;

#define TEST(name)                                                                                 \
    do {                                                                                           \
        tests_run++;                                                                               \
        printf("  TEST %s ... ", #name);

#define PASS()                                                                                     \
    tests_passed++;                                                                                \
    printf("OK\n");                                                                                \
    }                                                                                              \
    while (0)

#define ASSERT_EQ_STR(a, b)                                                                        \
    if (strcmp((a), (b)) != 0) {                                                                   \
        printf("FAIL: '%s' != '%s'\n", (a), (b));                                                  \
        break;                                                                                     \
    }

#define ASSERT_EQ_INT(a, b)                                                                        \
    if ((a) != (b)) {                                                                              \
        printf("FAIL: %d != %d\n", (int)(a), (int)(b));                                            \
        break;                                                                                     \
    }

#define ASSERT_NULL(a)                                                                             \
    if ((a) != NULL) {                                                                             \
        printf("FAIL: expected NULL\n");                                                           \
        break;                                                                                     \
    }

void test_create_destroy(void)
{
    TEST(create_destroy);
    kvs_store_t *s = kvs_create(16);
    ASSERT_EQ_INT(kvs_count(s), 0);
    kvs_destroy(s);
    PASS();
}

void test_set_get(void)
{
    TEST(set_get);
    kvs_store_t *s = kvs_create(16);
    ASSERT_EQ_INT(kvs_set(s, "key1", "val1"), KVS_OK);
    ASSERT_EQ_STR(kvs_get(s, "key1"), "val1");
    ASSERT_EQ_INT(kvs_count(s), 1);
    kvs_destroy(s);
    PASS();
}

void test_update(void)
{
    TEST(update_existing_key);
    kvs_store_t *s = kvs_create(16);
    ASSERT_EQ_INT(kvs_set(s, "key", "old"), KVS_OK);
    ASSERT_EQ_INT(kvs_set(s, "key", "new"), KVS_OK);
    ASSERT_EQ_STR(kvs_get(s, "key"), "new");
    ASSERT_EQ_INT(kvs_count(s), 1);
    kvs_destroy(s);
    PASS();
}

void test_delete(void)
{
    TEST(delete);
    kvs_store_t *s = kvs_create(16);
    ASSERT_EQ_INT(kvs_set(s, "key", "val"), KVS_OK);
    ASSERT_EQ_INT(kvs_del(s, "key"), KVS_OK);
    ASSERT_NULL(kvs_get(s, "key"));
    ASSERT_EQ_INT(kvs_count(s), 0);
    kvs_destroy(s);
    PASS();
}

void test_delete_nonexistent(void)
{
    TEST(delete_nonexistent);
    kvs_store_t *s = kvs_create(16);
    ASSERT_EQ_INT(kvs_del(s, "nope"), KVS_NOT_FOUND);
    kvs_destroy(s);
    PASS();
}

void test_multiple_keys(void)
{
    TEST(multiple_keys);
    kvs_store_t *s = kvs_create(64);
    for (int i = 0; i < 20; i++) {
        char key[16], val[16];
        snprintf(key, sizeof(key), "k%d", i);
        snprintf(val, sizeof(val), "v%d", i);
        ASSERT_EQ_INT(kvs_set(s, key, val), KVS_OK);
    }
    ASSERT_EQ_INT(kvs_count(s), 20);
    ASSERT_EQ_STR(kvs_get(s, "k0"), "v0");
    ASSERT_EQ_STR(kvs_get(s, "k19"), "v19");
    kvs_destroy(s);
    PASS();
}

// Fix 1: verify store grows beyond initial capacity via auto-resize
void test_resize(void)
{
    TEST(auto_resize);
    // Start with capacity 8; insert 60 entries — forces multiple resizes
    kvs_store_t *s = kvs_create(8);
    for (int i = 0; i < 60; i++) {
        char key[16], val[16];
        snprintf(key, sizeof(key), "rk%d", i);
        snprintf(val, sizeof(val), "rv%d", i);
        ASSERT_EQ_INT(kvs_set(s, key, val), KVS_OK);
    }
    ASSERT_EQ_INT(kvs_count(s), 60);
    // Verify data integrity after resize
    ASSERT_EQ_STR(kvs_get(s, "rk0"), "rv0");
    ASSERT_EQ_STR(kvs_get(s, "rk59"), "rv59");
    kvs_destroy(s);
    PASS();
}

void test_null_safety(void)
{
    TEST(null_safety);
    ASSERT_EQ_INT(kvs_set(NULL, "k", "v"), KVS_ERR);
    ASSERT_NULL(kvs_get(NULL, "k"));
    ASSERT_EQ_INT(kvs_del(NULL, "k"), KVS_ERR);
    ASSERT_EQ_INT(kvs_count(NULL), 0);
    PASS();
}

int main(void)
{
    printf("=== mini-kvstore tests ===\n");

    test_create_destroy();
    test_set_get();
    test_update();
    test_delete();
    test_delete_nonexistent();
    test_multiple_keys();
    test_resize();
    test_null_safety();

    printf("\n%d/%d tests passed\n", tests_passed, tests_run);
    return (tests_passed == tests_run) ? 0 : 1;
}
