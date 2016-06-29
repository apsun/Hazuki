#include "hazuki/vector.h"
#include "hazuki/utils.h"
#include <stdio.h>
#include <stdlib.h>

typedef void (*printer)(const void *, FILE *);

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
hz_vector_print(const hz_vector *vec, FILE *file, printer element_printer)
{
    fputc('[', file);
    size_t size = hz_vector_size(vec);
    for (size_t i = 0; i < size; ++i) {
        if (i != 0) {
            fputs(", ", file);
        }
        element_printer(hz_vector_get(vec, i), file);
    }
    fputs("]\n", file);
}

static void
hz_vector_print_int(const hz_vector *vec)
{
    hz_vector_print(vec, stdout, print_int);
}

static void
hz_vector_assert_size(const hz_vector *vec, size_t size)
{
    size_t vec_size = hz_vector_size(vec);
    if (vec_size != size) {
        hz_abort("Vector size (%zu) does not match expected size (%zu)", vec_size, size);
    }
}

static void
hz_vector_assert_capacity(const hz_vector *vec, size_t capacity)
{
    size_t vec_capacity = hz_vector_capacity(vec);
    if (vec_capacity != capacity) {
        hz_abort("Vector capacity (%zu) does not match expected capacity (%zu)", vec_capacity, capacity);
    }
}

static void
hz_vector_assert_find(const hz_vector *vec, void *value, size_t index)
{
    size_t vec_index;
    if (!hz_vector_find(vec, value, &vec_index)) {
        hz_abort("Vector element not found: %p", value);
    }
    if (vec_index != index) {
        hz_abort("Vector element (%p) found at [%zu] but expected at [%zu]", value, vec_index, index);
    }
}

static void
hz_vector_assert_not_find(const hz_vector *vec, void *value)
{
    size_t vec_index;
    if (hz_vector_find(vec, value, &vec_index)) {
        hz_abort("Vector element (%p) found at [%zu] but shouldn't exist", value, vec_index);
    }
}

static void
hz_vector_assert_get(const hz_vector *vec, size_t index, const void *value)
{
    void *vec_item = hz_vector_get(vec, index);
    if (vec_item != value) {
        hz_abort("Vector element at [%zu] (%p) does not match expected value (%p)", index, vec_item, value);
    }
}

static void
hz_vector_assert_eq(const hz_vector *vec, const void **arr, size_t size)
{
    hz_vector_assert_size(vec, size);
    for (size_t i = 0; i < size; ++i) {
        hz_vector_assert_get(vec, i, arr[i]);
    }
}

int
main(int argc, char *argv[])
{
    int test_data[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };

    // Append and insert test
    hz_vector *vec1 = hz_vector_new();
    hz_vector_append(vec1, &test_data[1]);
    hz_vector_append(vec1, NULL);
    hz_vector_append(vec1, &test_data[0]);
    hz_vector_insert(vec1, 1, &test_data[6]);
    hz_vector_insert(vec1, 4, &test_data[7]);
    hz_vector_insert(vec1, 5, NULL);
    hz_vector_insert(vec1, 6, &test_data[9]);
    void *vec1_expected[] = {
        &test_data[1], 
        &test_data[6], 
        NULL, 
        &test_data[0],
        &test_data[7], 
        NULL, 
        &test_data[9]
    };
    hz_vector_assert_eq(vec1, vec1_expected, 7);
    hz_vector_print_int(vec1);

    // Removal test
    hz_vector *vec2 = hz_vector_copy(vec1);
    hz_vector_free(vec1);
    hz_vector_append(vec2, &test_data[4]);
    hz_vector_append(vec2, &test_data[1]);
    hz_vector_remove(vec2, 0);
    hz_vector_remove(vec2, 0);
    void *vec2_expected[] = {
        NULL,
        &test_data[0], 
        &test_data[7],
        NULL,
        &test_data[9],
        &test_data[4],
        &test_data[1]
    };
    hz_vector_assert_eq(vec2, vec2_expected, 7);
    hz_vector_print_int(vec2);

    // Capacity reserve test
    hz_vector *vec3 = hz_vector_copy(vec2);
    hz_vector_free(vec2);
    hz_vector_reserve(vec3, 500);
    hz_vector_remove(vec3, 0);
    hz_vector_insert(vec3, 0, &test_data[1]);
    void *vec3_expected[] = {
        &test_data[1],
        &test_data[0],
        &test_data[7],
        NULL,
        &test_data[9],
        &test_data[4],
        &test_data[1]
    };
    hz_vector_assert_capacity(vec3, 500);
    hz_vector_assert_eq(vec3, vec3_expected, 7);
    hz_vector_print_int(vec3);

    // Value find test
    hz_vector *vec4 = hz_vector_copy(vec3);
    hz_vector_free(vec3);
    hz_vector_assert_find(vec4, &test_data[9], 4);
    hz_vector_assert_find(vec4, NULL, 3);
    hz_vector_assert_find(vec4, &test_data[1], 0);
    hz_vector_assert_not_find(vec4, &test_data[8]);
    hz_vector_trim(vec4);
    hz_vector_assert_capacity(vec4, 7);
    hz_vector_print_int(vec4);

    // Large vector and auto-resize test
    hz_vector *vec5 = hz_vector_copy(vec4);
    hz_vector_free(vec4);
    hz_vector_clear(vec5);
    int test_data2[10000];
    for (int i = 0; i < 10000; ++i) {
        test_data2[i] = i;
        hz_vector_insert(vec5, 0, &test_data2[i]);
    }
    hz_vector_assert_size(vec5, 10000);
    for (int i = 0; i < 10000; ++i) {
        hz_vector_assert_get(vec5, i, &test_data2[10000 - i - 1]);
    }

    // Vector resize test
    hz_vector *vec6 = hz_vector_copy(vec5);
    hz_vector_free(vec5);
    hz_vector_clear(vec6);
    int dummy1 = 0, dummy2 = 1;
    hz_vector_resize(vec6, 10, &dummy2);
    void **buf6 = hz_vector_data(vec6);
    for (int i = 0; i < 5; ++i) {
        buf6[i] = ((i & 1) == 0) ? &dummy1 : &dummy2;
    }
    hz_vector_print_int(vec6);
    hz_vector_resize(vec6, 5, NULL);
    void *vec6_expected[] = {
        &dummy1,
        &dummy2,
        &dummy1,
        &dummy2,
        &dummy1
    };
    hz_vector_assert_eq(vec6, vec6_expected, 5);
    hz_vector_print_int(vec6);
    hz_vector_free(vec6);

    printf("All tests passed!\n");
    getchar();
    return 0;
}
