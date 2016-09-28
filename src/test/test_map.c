#include "hazuki/map.h"
#include "hazuki/utils.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

typedef void (*printer)(const void *, FILE *);

static size_t
hash_int(const void *key)
{
    if (key == NULL) {
        return 0;
    } else {
        return (size_t)*(int *)key;
    }
}

static int
cmp_int(const void *a, const void *b)
{
    if (a == NULL || b == NULL) {
        return a != b;
    } else {
        return *(int *)a != *(int *)b;
    }
}

static void
print_int(const void *value, FILE *file)
{
    if (value == NULL) {
        fprintf(file, "(null)");
    } else {
        fprintf(file, "%d", *(int *)value);
    }
}

static void
print_string(const void *value, FILE *file)
{
    if (value == NULL) {
        fprintf(file, "(null)");
    } else {
        fprintf(file, "\"%s\"", (char *)value);
    }
}

static void
hz_map_print(const hz_map *map, FILE *file, printer key_printer, printer value_printer)
{
    fputc('{', file);
    void *key, *value;
    bool first = true;
    hz_map_iterator *it = hz_map_iterator_new(map);
    while (hz_map_iterator_next(it, &key, &value)) {
        if (!first) {
            fputs(", ", file);
        } else {
            first = false;
        }
        key_printer(key, file);
        fputs(": ", file);
        value_printer(value, file);
    }
    hz_map_iterator_free(it);
    fputs("}\n", file);
}

static void
hz_map_assert_get(const hz_map *map, void *key, void *value)
{
    if (!hz_map_contains(map, key)) {
        hz_abort("Map does not contain key: %p", key);
    }
    void *map_value = hz_map_get(map, key);
    if (value != map_value) {
        hz_abort("Unexpected map result for [%p] (got %p, wanted %p)", key, map_value, value);
    }
}

static void
hz_map_assert_not_get(const hz_map *map, void *key)
{
    if (hz_map_contains(map, key)) {
        hz_abort("Map contains key but shouldn't: %p", key);
    }
    void *map_value = hz_map_get(map, key);
    if (map_value != NULL) {
        hz_abort("Map does not contain key (%p) but get() returned %p", key, map_value);
    }
}

static void
hz_map_assert_eq(const hz_map *map, void **kvps, size_t count)
{
    for (size_t i = 0; i < count; ++i) {
        void *key = kvps[i * 2];
        void *value = kvps[i * 2 + 1];
        hz_map_assert_get(map, key, value);
    }
}

static void
hz_map_assert_size(const hz_map *map, size_t size)
{
    size_t map_size = hz_map_size(map);
    if (map_size != size) {
        hz_abort("Expected %zu items in map, got %zu", map_size, size);
    }
}

static void
hz_map_print_int_string(const hz_map *map)
{
    hz_map_print(map, stdout, print_int, print_string);
}

void
test_map(void)
{
    int test_data[] = {
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9
    };
    char *test_data_names[] = {
        "zero", "one", "two", "three", "four",
        "five", "six", "seven", "eight", "nine",
        "zero-new", "one-new", "two-new", "three-new", "four-new",
        "five-new", "six-new", "seven-new", "eight-new", "nine-new"
    };

    // Insertion test, key 10 should overwrite key 0
    hz_map *map1 = hz_map_new(hash_int, cmp_int);
    hz_map_put(map1, &test_data[0], test_data_names[0]);
    hz_map_put(map1, &test_data[1], test_data_names[1]);
    hz_map_put(map1, &test_data[2], test_data_names[3]);
    hz_map_put(map1, &test_data[10], test_data_names[10]);
    hz_map_put(map1, &test_data[2], NULL);
    void *map1_expected[] = {
        &test_data[0], test_data_names[10],
        &test_data[1], test_data_names[1],
        &test_data[10], test_data_names[10],
        &test_data[2], NULL
    };
    hz_map_assert_eq(map1, map1_expected, 4);
    hz_map_assert_not_get(map1, &test_data[5]);
    hz_map_assert_size(map1, 3);
    hz_map_print_int_string(map1);

    // Removal test, value 10 should be written to key 3, removal should be no-op
    hz_map *map2 = hz_map_copy(map1);
    hz_map_free(map1);
    hz_map_put(map2, &test_data[3], hz_map_remove(map2, &test_data[0]));
    hz_map_remove(map2, &test_data[10]);
    void *map2_expected[] = {
        &test_data[3], test_data_names[10],
        &test_data[1], test_data_names[1],
        &test_data[2], NULL
    };
    hz_map_assert_eq(map2, map2_expected, 3);
    hz_map_assert_not_get(map2, &test_data[10]);
    hz_map_assert_not_get(map2, &test_data[0]);
    hz_map_assert_size(map2, 3);
    hz_map_print_int_string(map2);

    // Clear test, previous keys should not exist anymore
    hz_map *map3 = hz_map_copy(map2);
    hz_map_free(map2);
    hz_map_clear(map3);
    hz_map_assert_not_get(map3, &test_data[2]);
    hz_map_assert_size(map3, 0);
    hz_map_print_int_string(map3);

    // NULL key and key overwrite test, value 14 should overwrite value 4
    hz_map *map4 = hz_map_copy(map3);
    hz_map_free(map3);
    hz_map_put(map4, NULL, test_data_names[4]);
    hz_map_put(map4, NULL, test_data_names[14]);
    void *map4_expected[] = {
        NULL, test_data_names[14]
    };
    hz_map_assert_eq(map4, map4_expected, 1);
    hz_map_print_int_string(map4);

    // Large map and resize test
    hz_map *map5 = hz_map_copy(map4);
    hz_map_free(map4);
    hz_map_clear(map5);
    int test_data2[10000];
    for (int i = 0; i < 10000; ++i) {
        test_data2[i] = i;
        hz_map_put(map5, &test_data2[i], &test_data2[i]);
    }
    hz_map_assert_size(map5, 10000);
    for (int i = 0; i < 10000; ++i) {
        hz_map_assert_get(map5, &test_data2[i], &test_data2[i]);
    }
    hz_map_free(map5);

    printf("All map tests passed!\n");
}
