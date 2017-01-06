#include "hazuki/vector.h"
#include "hazuki/utils.h"
#include <stdio.h>
#include <stdlib.h>

typedef int T;

static int
cmp_T(const void *a, const void *b)
{
    T at = *(T *)a;
    T bt = *(T *)b;
    if (at < bt) {
        return -1;
    } else if (at > bt) {
        return 1;
    } else {
        return 0;
    }
}

static hz_vector *
hz_vector_new_T(void)
{
    return hz_vector_new(sizeof(T));
}

static void
hz_vector_insert_T(hz_vector *vec, size_t index, T value)
{
    hz_vector_insert(vec, index, &value);
}

static T
hz_vector_get_T(const hz_vector *vec, size_t index)
{
    T value;
    hz_vector_get(vec, index, &value);
    return value;
}

static void
hz_vector_set_T(hz_vector *vec, size_t index, T value)
{
    hz_vector_set(vec, index, &value);
}

static void
hz_vector_append_T(hz_vector *vec, T value)
{
    hz_vector_append(vec, &value);
}

static void
hz_vector_resize_T(hz_vector *vec, size_t size, T fill)
{
    hz_vector_resize(vec, size, &fill);
}

static bool
hz_vector_find_T(const hz_vector *vec, T value, size_t *out_index)
{
    return hz_vector_find(vec, &value, out_index);
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
hz_vector_assert_find(const hz_vector *vec, T value, size_t expected_index)
{
    size_t vec_index;
    if (!hz_vector_find_T(vec, value, &vec_index)) {
        hz_abort("Vector element not found");
    }
    if (vec_index != expected_index) {
        hz_abort("Vector element found at [%zu] but expected at [%zu]", vec_index, expected_index);
    }
}

static void
hz_vector_assert_not_find(const hz_vector *vec, T value)
{
    size_t vec_index;
    if (hz_vector_find_T(vec, value, &vec_index)) {
        hz_abort("Vector element found at [%zu] but shouldn't exist", vec_index);
    }
}

static void
hz_vector_assert_get(const hz_vector *vec, size_t index, T expected_value)
{
    T value = hz_vector_get_T(vec, index);
    if (value != expected_value) {
        hz_abort("Vector element at [%zu] does not match expected value", index);
    }
}

static void
hz_vector_assert_eq(const hz_vector *vec, const T *arr, size_t size)
{
    hz_vector_assert_size(vec, size);
    for (size_t i = 0; i < size; ++i) {
        hz_vector_assert_get(vec, i, arr[i]);
    }
}

static void
hz_vector_assert_equals_true(const hz_vector *a, const hz_vector *b)
{
    if (!hz_vector_equals(a, b)) {
        hz_abort("Vectors should be equal");
    }
}

static void
test_vector_append(void)
{
    hz_vector *vec = hz_vector_new_T();
    hz_vector_append_T(vec, 0);
    hz_vector_append_T(vec, 1);
    hz_vector_append_T(vec, 2);
    T expected[] = { 0, 1, 2 };
    hz_vector_assert_eq(vec, expected, 3);
    hz_vector_free(vec);
}

static void
test_vector_insert(void)
{
    hz_vector *vec = hz_vector_new_T();
    hz_vector_insert_T(vec, 0, 0);
    hz_vector_append_T(vec, 1);
    hz_vector_append_T(vec, 2);
    hz_vector_insert_T(vec, 1, 3);
    hz_vector_insert_T(vec, 1, 4);
    hz_vector_insert_T(vec, 5, 5);
    hz_vector_insert_T(vec, 6, 6);
    T expected[] = { 0, 4, 3, 1, 2, 5, 6 };
    hz_vector_assert_eq(vec, expected, 7);
    hz_vector_free(vec);
}

static void
test_vector_set(void)
{
    hz_vector *vec = hz_vector_new_T();
    hz_vector_append_T(vec, 0);
    hz_vector_append_T(vec, 1);
    hz_vector_append_T(vec, 2);
    hz_vector_append_T(vec, 3);
    hz_vector_set_T(vec, 0, 1);
    hz_vector_set_T(vec, 1, 5);
    T expected[] = { 1, 5, 2, 3 };
    hz_vector_assert_eq(vec, expected, 4);
    hz_vector_free(vec);
}

static void
test_vector_remove(void)
{
    hz_vector *vec = hz_vector_new_T();
    hz_vector_append_T(vec, 0);
    hz_vector_append_T(vec, 1);
    hz_vector_append_T(vec, 2);
    hz_vector_append_T(vec, 3);
    hz_vector_append_T(vec, 4);
    hz_vector_append_T(vec, 5);
    hz_vector_append_T(vec, 6);
    hz_vector_remove(vec, 0);
    hz_vector_remove(vec, 1);
    T expected[] = { 1, 3, 4, 5, 6 };
    hz_vector_assert_eq(vec, expected, 5);
    hz_vector_clear(vec);
    hz_vector_assert_eq(vec, NULL, 0);
    hz_vector_free(vec);
}

static void
test_vector_find(void)
{
    hz_vector *vec = hz_vector_new_T();
    hz_vector_append_T(vec, 0);
    hz_vector_append_T(vec, 1);
    hz_vector_append_T(vec, 0);
    hz_vector_append_T(vec, 1);
    hz_vector_append_T(vec, 4);
    hz_vector_append_T(vec, 5);
    hz_vector_append_T(vec, 6);
    hz_vector_assert_find(vec, 0, 0);
    hz_vector_assert_find(vec, 1, 1);
    hz_vector_assert_find(vec, 4, 4);
    hz_vector_assert_not_find(vec, 7);
    hz_vector_free(vec);
}

static void
test_vector_large(void)
{
    hz_vector *vec = hz_vector_new_T();
    for (int i = 0; i < 10000; ++i) {
        hz_vector_insert_T(vec, 0, i);
    }
    hz_vector_assert_size(vec, 10000);
    for (int i = 0; i < 10000; ++i) {
        hz_vector_assert_get(vec, i, 10000 - i - 1);
    }
    hz_vector_free(vec);
}

static void
test_vector_resize(void)
{
    hz_vector *vec = hz_vector_new_T();
    hz_vector_resize_T(vec, 5, 42);
    T expected[] = { 42, 42, 42, 42, 42 };
    hz_vector_assert_eq(vec, expected, 5);
    hz_vector_free(vec);
}

static void
test_vector_data(void)
{
    hz_vector *vec = hz_vector_new_T();
    hz_vector_resize_T(vec, 8, 42);
    T *data = hz_vector_data(vec);
    for (int i = 0; i < 4; ++i) {
        data[i] = i;
    }
    T expected[] = { 0, 1, 2, 3, 42, 42, 42, 42 };
    hz_vector_assert_eq(vec, expected, 8);
    hz_vector_free(vec);
}

static void
test_vector_copy(void)
{
    hz_vector *vec = hz_vector_new_T();
    hz_vector_append_T(vec, 1);
    hz_vector_append_T(vec, 2);
    hz_vector_append_T(vec, 3);
    hz_vector_append_T(vec, 4);
    hz_vector *copy = hz_vector_copy(vec);
    hz_vector_free(vec);
    T expected[] = { 1, 2, 3, 4 };
    hz_vector_assert_eq(copy, expected, 4);
    hz_vector_free(copy);
}

static void
test_vector_reserve(void)
{
    hz_vector *vec = hz_vector_new_T();
    hz_vector_reserve(vec, 100);
    for (int i = 0; i < 100; ++i) {
        hz_vector_append_T(vec, i);
    }
    hz_vector_assert_capacity(vec, 100);
    hz_vector_free(vec);
}

static void
test_vector_import(void)
{
    T src[5] = { 1, 2, 3, 4, 5 };
    hz_vector *vec = hz_vector_new_T();
    hz_vector_resize(vec, 5, NULL);
    T *buf1 = hz_vector_data(vec);
    hz_memcpy(buf1, src, 5, sizeof(T));
    hz_vector_assert_eq(vec, src, 5);
    hz_vector_free(vec);
}

static void
test_vector_equals(void)
{
    hz_vector *vec1 = hz_vector_new_T();
    hz_vector_append_T(vec1, 1);
    hz_vector_append_T(vec1, 2);
    hz_vector_append_T(vec1, 3);
    hz_vector_append_T(vec1, 4);

    hz_vector *vec2 = hz_vector_new_T();
    hz_vector_insert_T(vec2, 0, 4);
    hz_vector_insert_T(vec2, 0, 3);
    hz_vector_insert_T(vec2, 0, 2);
    hz_vector_insert_T(vec2, 0, 1);

    hz_vector_assert_equals_true(vec1, vec2);
    hz_vector_free(vec1);
    hz_vector_free(vec2);

    hz_vector_assert_equals_true(NULL, NULL);
}

static void
test_vector_reverse(void)
{
    hz_vector *vec = hz_vector_new_T();
    hz_vector_append_T(vec, 1);
    hz_vector_append_T(vec, 2);
    hz_vector_append_T(vec, 3);
    hz_vector_append_T(vec, 4);
    hz_vector_reverse(vec);
    T expected[] = { 4, 3, 2, 1 };
    hz_vector_assert_eq(vec, expected, 4);
}

static void
test_vector_sort(void)
{
    hz_vector *vec = hz_vector_new_T();
    hz_vector_append_T(vec, 0);
    hz_vector_append_T(vec, -2);
    hz_vector_append_T(vec, 100);
    hz_vector_append_T(vec, INT_MIN);
    hz_vector_append_T(vec, 1);
    hz_vector_append_T(vec, -5);
    hz_vector_append_T(vec, INT_MAX);
    hz_vector_sort(vec, cmp_T);
    T expected[] = { INT_MIN, -5, -2, 0, 1, 100, INT_MAX };
    hz_vector_assert_eq(vec, expected, 7);
}

void
test_vector(void)
{
    test_vector_append();
    test_vector_insert();
    test_vector_set();
    test_vector_remove();
    test_vector_find();
    test_vector_large();
    test_vector_resize();
    test_vector_data();
    test_vector_copy();
    test_vector_reserve();
    test_vector_import();
    test_vector_equals();
    test_vector_reverse();
    test_vector_sort();
    printf("All vector tests passed!\n");
}
