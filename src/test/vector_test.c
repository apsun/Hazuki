#include "hazuki/vector.h"
#include "hazuki/utils.h"
#include <stdlib.h>
#include <stdio.h>

static void print_int(void *value, FILE *file)
{
    if (value == NULL) {
        fprintf(file, "(null)");
    } else {
        fprintf(file, "%d", *(int *)value);
    }
}

static void hz_vector_print(const hz_vector *vec, FILE *file, void (*element_printer)(void *, FILE *))
{
    fputc('[', file);
    size_t size = hz_vector_size(vec);
    for (size_t i = 0; i < size; ++i) {
        if (i != 0) {
            fputs(", ", file);
        }
        element_printer(hz_vector_get(vec, i), file);
    }
    fputc(']', file);
    fputc('\n', file);
}

static void hz_vector_assert_capacity(const hz_vector *vec, size_t capacity)
{
    size_t vec_capacity = hz_vector_capacity(vec);
    if (vec_capacity != capacity) {
        hz_abort("Vector capacity (%d) does not match expected capacity (%d)", vec_capacity, capacity);
    }
}

static void hz_vector_assert_find(const hz_vector *vec, void *value, size_t index)
{
    size_t vec_index;
    if (!hz_vector_find(vec, value, &vec_index)) {
        hz_abort("Vector element not found: %x", value);
    }
    if (vec_index != index) {
        hz_abort("Vector element (%x) found at [%d] but expected at [%d]", value, vec_index, index);
    }
}

static void hz_vector_assert_not_find(const hz_vector *vec, void *value)
{
    size_t vec_index;
    if (hz_vector_find(vec, value, &vec_index)) {
        hz_abort("Vector element (%x) found at [%d] but shouldn't exist", value, vec_index);
    }
}

static void hz_vector_assert_eq(const hz_vector *vec, const void **arr, size_t size)
{
    size_t vec_size = hz_vector_size(vec);
    if (vec_size != size) {
        hz_abort("Vector size (%d) does not match expected size (%d)", vec_size, size);
    }
    for (size_t i = 0; i < size; ++i) {
        void *vec_item = hz_vector_get(vec, i);
        if (vec_item != arr[i]) {
            hz_abort("Vector element at [%d] (%x) does not match expected value (%x)", i, vec_item, arr[i]);
        }
    }
}

int main(int argc, char *argv[])
{
    int test_data[10] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
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
    hz_vector_print(vec1, stdout, print_int);

    hz_vector *vec2 = hz_vector_copy(vec1);
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
    hz_vector_print(vec2, stdout, print_int);

    hz_vector *vec3 = hz_vector_copy(vec2);
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
    hz_vector_print(vec3, stdout, print_int);

    hz_vector *vec4 = hz_vector_copy(vec3);
    hz_vector_assert_find(vec4, &test_data[9], 4);
    hz_vector_assert_find(vec4, NULL, 3);
    hz_vector_assert_find(vec4, &test_data[1], 0);
    hz_vector_assert_not_find(vec4, &test_data[8]);
    hz_vector_trim(vec4);
    hz_vector_assert_capacity(vec4, 7);
    hz_vector_print(vec4, stdout, print_int);

    hz_vector_free(vec1);
    hz_vector_free(vec2);
    hz_vector_free(vec3);
    hz_vector_free(vec4);

    printf("All tests passed!\n");
    getchar();
    return 0;
}
