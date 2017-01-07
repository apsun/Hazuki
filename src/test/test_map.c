#include "hazuki/map.h"
#include "hazuki/utils.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef short TKey;
typedef char *TValue;
typedef struct
{
    TKey key;
    TValue value;
} TEntry;

static size_t
key_hash_T(const void *key)
{
    return (size_t)*(TKey *)key;
}

static size_t
key_hash_bad_T(const void *key)
{
    (void)key;
    return 0;
}

static int
key_cmp_T(const void *a, const void *b)
{
    return *(TKey *)a != *(TKey *)b;
}

static int
value_cmp_T(const void *a, const void *b)
{
    const char *as = *(const char **)a;
    const char *bs = *(const char **)b;
    if (as == NULL || bs == NULL) {
        return as != bs;
    }
    return strcmp(as, bs);
}

hz_map *
hz_map_new_T(hz_map_hash_func hash_func)
{
    return hz_map_new(sizeof(TKey), sizeof(TValue), hash_func, key_cmp_T);
}

bool
hz_map_get_T(const hz_map *map, TKey key, TValue *out_value)
{
    return hz_map_get(map, &key, out_value);
}

bool
hz_map_put_T(hz_map *map, TKey key, TValue value, TValue *old_value)
{
    return hz_map_put(map, &key, &value, old_value);
}

bool
hz_map_remove_T(hz_map *map, TKey key, TValue *old_value)
{
    return hz_map_remove(map, &key, old_value);
}

bool
hz_map_iterator_next_T(hz_map_iterator *it, TKey *key, TValue *value)
{
    return hz_map_iterator_next(it, key, value);
}

static void
hz_map_assert_get(const hz_map *map, TKey key, TValue expected_value)
{
    TValue value;
    if (!hz_map_get_T(map, key, &value)) {
        hz_abort("Map does not contain key");
    }
    if (value_cmp_T(&value, &expected_value) != 0) {
        hz_abort("Map contains key, but value is incorrect");
    }
}

static void
hz_map_assert_put_new(hz_map *map, TKey key, TValue value)
{
    if (hz_map_put_T(map, key, value, NULL)) {
        hz_abort("Replaced key when it shouldn't have");
    }
}

static void
hz_map_assert_put_replace(hz_map *map, TKey key, TValue value, TValue expeced_old_value)
{
    TValue old_value;
    if (!hz_map_put_T(map, key, value, &old_value)) {
        hz_abort("Should have replaced key");
    }
    if (value_cmp_T(&old_value, &expeced_old_value) != 0) {
        hz_abort("Old value mismatch");
    }
}

static void
hz_map_assert_remove(hz_map *map, TKey key, TValue expected_value)
{
    TValue value;
    if (!hz_map_remove_T(map, key, &value)) {
        hz_abort("Map does not contain key");
    }
    if (value_cmp_T(&value, &expected_value) != 0) {
        hz_abort("Old value mismatch");
    }
}

static void
hz_map_assert_not_remove(hz_map *map, TKey key)
{
    if (hz_map_remove_T(map, key, NULL)) {
        hz_abort("Map contains key when it shouldn't");
    }
}

static void
hz_map_assert_not_get(const hz_map *map, TKey key)
{
    if (hz_map_get_T(map, key, NULL)) {
        hz_abort("Map contains key but shouldn't");
    }
}

static void
hz_map_assert_size(const hz_map *map, size_t expected_size)
{
    size_t map_size = hz_map_size(map);
    if (map_size != expected_size) {
        hz_abort("Expected %zu items in map, got %zu", expected_size, map_size);
    }
}

static void
hz_map_assert_eq(const hz_map *map, TEntry *entries, size_t count)
{
    hz_map_assert_size(map, count);
    for (size_t i = 0; i < count; ++i) {
        TKey key = entries[i].key;
        TValue value = entries[i].value;
        hz_map_assert_get(map, key, value);
    }
}

static void
hz_map_assert_it_eq(const hz_map *map, TEntry *entries, size_t count)
{
    hz_map_assert_size(map, count);
    hz_map_iterator *it = hz_map_iterator_new(map);
    TKey key;
    TValue value;
    size_t n = 0;
    while (hz_map_iterator_next_T(it, &key, &value)) {
        bool found = false;
        for (size_t i = 0; i < count; ++i) {
            if (entries[i].key == key) {
                if (entries[i].value != value) {
                    hz_abort("Iterator key-value mismatch");
                }
                found = true;
                n++;
                break;
            }
        }
        if (!found) {
            hz_abort("Iterator returned unknown entry");
        }
    }
    hz_map_iterator_free(it);
    if (n != count) {
        hz_abort("Missing entries in iterator");
    }
}

static void
hz_map_assert_equals_true(const hz_map *a, const hz_map *b, hz_map_cmp_func cmp_func)
{
    if (!hz_map_equals(a, b, cmp_func)) {
        hz_abort("Maps should be equal");
    }
}

static void
hz_map_assert_equals_false(const hz_map *a, const hz_map *b, hz_map_cmp_func cmp_func)
{
    if (hz_map_equals(a, b, cmp_func)) {
        hz_abort("Maps should not be equal");
    }
}

static void
test_map_put(void)
{
    hz_map *map = hz_map_new_T(key_hash_T);
    hz_map_assert_put_new(map, 0, "zero");
    hz_map_assert_put_new(map, 1, "one");
    hz_map_assert_put_new(map, 2, "two");
    hz_map_assert_put_new(map, 3, "three");
    hz_map_assert_put_new(map, 4, "four");
    hz_map_assert_not_get(map, 5);
    hz_map_assert_not_get(map, 50);
    TEntry entries[] = {
        { 0, "zero" },
        { 1, "one" },
        { 2, "two" },
        { 3, "three" },
        { 4, "four" }
    };
    hz_map_assert_eq(map, entries, 5);
    hz_map_assert_put_replace(map, 2, "new two", "two");
    TEntry new_entries[] = {
        { 0, "zero" },
        { 1, "one" },
        { 2, "new two" },
        { 3, "three" },
        { 4, "four" }
    };
    hz_map_assert_eq(map, new_entries, 5);
    hz_map_free(map);
}

static void
test_map_remove(void)
{
    hz_map *map = hz_map_new_T(key_hash_T);
    hz_map_assert_put_new(map, 0, "zero");
    hz_map_assert_put_new(map, 1, "one");
    hz_map_assert_put_new(map, 2, "two");
    hz_map_assert_put_new(map, 3, "three");
    hz_map_assert_put_new(map, 4, "four");
    hz_map_assert_remove(map, 1, "one");
    hz_map_assert_not_remove(map, 5);
    TEntry entries[] = {
        { 0, "zero" },
        { 2, "two" },
        { 3, "three" },
        { 4, "four" }
    };
    hz_map_assert_eq(map, entries, 4);
    hz_map_free(map);
}

static void
test_map_clear(void)
{
    hz_map *map = hz_map_new_T(key_hash_T);
    hz_map_assert_put_new(map, 0, "zero");
    hz_map_assert_put_new(map, 1, "one");
    hz_map_assert_put_new(map, 2, "two");
    hz_map_clear(map);
    hz_map_assert_eq(map, NULL, 0);
    hz_map_free(map);
}

static void
test_map_large(void)
{
    TValue values[] = {
        "zero",
        "one",
        "two",
        "three",
        "four",
    };
    hz_map *map = hz_map_new_T(key_hash_T);
    for (TKey i = 0; i < 10000; ++i) {
        hz_map_assert_put_new(map, i, values[i % 5]);
    }
    hz_map_assert_size(map, 10000);
    for (TKey i = 0; i < 10000; ++i) {
        hz_map_assert_get(map, i, values[i % 5]);
    }
    hz_map_free(map);
}

static void
test_map_bad_hash(void)
{
    hz_map *map = hz_map_new_T(key_hash_bad_T);
    hz_map_assert_put_new(map, 0, "zero");
    hz_map_assert_put_new(map, 1, "one");
    hz_map_assert_put_new(map, 2, "two");
    hz_map_assert_put_new(map, 3, "three");
    hz_map_assert_put_new(map, 4, "four");
    hz_map_assert_put_replace(map, 0, "new zero", "zero");
    hz_map_assert_put_replace(map, 1, "new one", "one");
    hz_map_assert_remove(map, 0, "new zero");
    TEntry entries[] = {
        { 1, "new one" },
        { 2, "two" },
        { 3, "three" },
        { 4, "four" }
    };
    hz_map_assert_eq(map, entries, 4);
    hz_map_free(map);
}

static void
test_map_iterator(void)
{
    hz_map *map = hz_map_new_T(key_hash_T);
    hz_map_assert_put_new(map, 0, "zero");
    hz_map_assert_put_new(map, 1, "one");
    hz_map_assert_put_new(map, 2, "two");
    hz_map_assert_put_new(map, 3, "three");
    hz_map_assert_put_replace(map, 0, "new zero", "zero");
    hz_map_assert_put_replace(map, 1, "new one", "one");
    hz_map_assert_remove(map, 0, "new zero");
    TEntry entries[] = {
        { 1, "new one" },
        { 2, "two" },
        { 3, "three" }
    };
    hz_map_assert_it_eq(map, entries, 3);
    hz_map_free(map);
}

static void
test_map_copy(void)
{
    hz_map *map = hz_map_new_T(key_hash_T);
    hz_map_assert_put_new(map, 0, "zero");
    hz_map_assert_put_new(map, 1, "one");
    hz_map_assert_put_new(map, 2, "two");
    hz_map_assert_put_new(map, 3, "three");
    hz_map_assert_remove(map, 0, "zero");
    hz_map *copy = hz_map_copy(map);
    hz_map_free(map);
    TEntry entries[] = {
        { 1, "one" },
        { 2, "two" },
        { 3, "three" }
    };
    hz_map_assert_eq(copy, entries, 3);
    hz_map_free(copy);
}

static void
test_map_equals(void)
{
    hz_map *map1 = hz_map_new_T(key_hash_T);
    hz_map_assert_put_new(map1, 0, "zero");
    hz_map_assert_put_new(map1, 1, "one");
    hz_map_assert_put_new(map1, 2, "two");
    hz_map_assert_put_new(map1, 3, "three");
    hz_map *map2 = hz_map_new_T(key_hash_bad_T);
    for (TKey i = 0; i < 100; ++i) {
        hz_map_assert_put_new(map2, i, "dummy");
    }
    for (TKey i = 0; i < 100; ++i) {
        hz_map_assert_remove(map2, i, "dummy");
    }
    hz_map_assert_put_new(map2, 3, "three");
    hz_map_assert_put_new(map2, 0, "zero");
    hz_map_assert_put_new(map2, 2, "two");
    hz_map_assert_put_new(map2, 1, "one");
    hz_map_assert_equals_true(map1, map2, value_cmp_T);
    hz_map_assert_equals_true(map1, map2, NULL);
    hz_map_assert_put_replace(map2, 1, NULL, "one");
    hz_map_assert_equals_false(map1, map2, value_cmp_T);
    hz_map_assert_equals_false(map1, map2, NULL);
    hz_map_free(map1);
    hz_map_free(map2);
}

void
test_map(void)
{
    test_map_put();
    test_map_remove();
    test_map_clear();
    test_map_large();
    test_map_bad_hash();
    test_map_iterator();
    test_map_copy();
    test_map_equals();
    printf("All map tests passed!\n");
}
