#include "hazuki/vector.h"
#include "hazuki/utils.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

/**
 * Initial capacity for the vector. Must be an integer > 0.
 */
#define INITIAL_CAPACITY 8

/**
 * Factor by which to scale the vector's internal buffer when full.
 * Must be > 1.
 */
#define SCALING_FACTOR 1.5

struct hz_vector
{
    size_t element_size;
    size_t size;
    size_t capacity;
    char *buffer;
};

static void
hz_vector_resize_capacity(hz_vector *vec, size_t new_capacity)
{
    vec->buffer = hz_realloc(vec->buffer, new_capacity, vec->element_size);
    vec->capacity = new_capacity;
}

static size_t
hz_vector_next_capacity(size_t current_capacity)
{
    if (current_capacity == SIZE_MAX) {
        hz_abort("Cannot resize vector larger than %zu elements", SIZE_MAX);
        return 0;
    } else if (current_capacity > (size_t)(SIZE_MAX / SCALING_FACTOR)) {
        return SIZE_MAX;
    } else {
        size_t scale_capacity = (size_t)(current_capacity * SCALING_FACTOR);
        return hz_max(scale_capacity, INITIAL_CAPACITY);
    }
}

static void
hz_vector_grow_if_full(hz_vector *vec)
{
    if (vec->size == vec->capacity) {
        size_t new_capacity = hz_vector_next_capacity(vec->capacity);
        hz_vector_resize_capacity(vec, new_capacity);
    }
}

static void *
hz_vector_offset_of(const hz_vector *vec, size_t index)
{
    return &vec->buffer[index * vec->element_size];
}

hz_vector *
hz_vector_new(size_t element_size)
{
    hz_vector *vec = hz_malloc(1, sizeof(hz_vector));
    vec->element_size = element_size;
    vec->size = 0;
    vec->capacity = 0;
    vec->buffer = NULL;
    return vec;
}

hz_vector *
hz_vector_copy(const hz_vector *vec)
{
    hz_check_null(vec);
    hz_vector *new_vec = hz_malloc(1, sizeof(hz_vector));
    new_vec->element_size = vec->element_size;
    new_vec->size = vec->size;
    new_vec->capacity = vec->capacity;
    new_vec->buffer = hz_malloc(new_vec->capacity, new_vec->element_size);
    hz_memcpy(new_vec->buffer, vec->buffer, vec->size, vec->element_size);
    return new_vec;
}

void
hz_vector_free(hz_vector *vec)
{
    if (vec != NULL) {
        hz_free(vec->buffer);
        hz_free(vec);
    }
}

size_t
hz_vector_size(const hz_vector *vec)
{
    hz_check_null(vec);
    return vec->size;
}

size_t
hz_vector_capacity(const hz_vector *vec)
{
    hz_check_null(vec);
    return vec->capacity;
}

void
hz_vector_resize(hz_vector *vec, size_t size, const void *fill)
{
    hz_check_null(vec);
    hz_vector_reserve(vec, size);

    // If fill is NULL, the value of the new elements will be undefined
    // until written to. We provide this option so that clients can
    // call resize() and then fill the internal buffer with some code
    // that works with standard C arrays using data().
    if (fill != NULL) {
        for (size_t i = vec->size; i < size; ++i) {
            void *dest = hz_vector_offset_of(vec, i);
            hz_memcpy(dest, fill, 1, vec->element_size);
        }
    }
    vec->size = size;
}

void
hz_vector_reserve(hz_vector *vec, size_t capacity)
{
    hz_check_null(vec);
    if (capacity > vec->capacity) {
        hz_vector_resize_capacity(vec, capacity);
    }
}

void
hz_vector_trim(hz_vector *vec)
{
    hz_check_null(vec);
    hz_vector_resize_capacity(vec, vec->size);
}

void
hz_vector_clear(hz_vector *vec)
{
    hz_check_null(vec);
    vec->size = 0;
}

void
hz_vector_get(const hz_vector *vec, size_t index, void *out_value)
{
    hz_check_null(vec);
    hz_check_null(out_value);
    hz_assert(index < vec->size);

    void *src = hz_vector_offset_of(vec, index);
    hz_memcpy(out_value, src, 1, vec->element_size);
}

void
hz_vector_set(hz_vector *vec, size_t index, const void *value)
{
    hz_check_null(vec);
    hz_check_null(value);
    hz_assert(index < vec->size);

    void *dest = hz_vector_offset_of(vec, index);
    hz_memcpy(dest, value, 1, vec->element_size);
}

void
hz_vector_append(hz_vector *vec, const void *value)
{
    hz_check_null(vec);
    hz_check_null(value);

    hz_vector_grow_if_full(vec);
    void *dest = hz_vector_offset_of(vec, vec->size);
    hz_memcpy(dest, value, 1, vec->element_size);
    vec->size++;
}

void
hz_vector_insert(hz_vector *vec, size_t index, const void *value)
{
    hz_check_null(vec);
    hz_check_null(value);

    // Note that this algorithm still works fine if the index
    // is equal to the size; we just shift 0 elements.
    hz_assert(index <= vec->size);

    // Shift elements after target index right by 1 unit
    hz_vector_grow_if_full(vec);
    size_t num = vec->size - index;
    void *offset = hz_vector_offset_of(vec, index);
    void *next_offset = hz_vector_offset_of(vec, index + 1);
    hz_memmove(next_offset, offset, num, vec->element_size);

    // Copy element into buffer
    hz_memcpy(offset, value, 1, vec->element_size);
    vec->size++;
}

void
hz_vector_remove(hz_vector *vec, size_t index)
{
    hz_check_null(vec);
    hz_assert(index < vec->size);

    // Shift elements after target index left by 1 unit
    size_t num = vec->size - index - 1;
    void *offset = hz_vector_offset_of(vec, index);
    void *next_offset = hz_vector_offset_of(vec, index + 1);
    hz_memmove(offset, next_offset, num, vec->element_size);
    vec->size--;
}

void
hz_vector_reverse(hz_vector *vec)
{
    hz_check_null(vec);

    // If we have some free space at the end of the buffer,
    // reuse that; otherwise allocate a temporary buffer.
    void *tmp_mem;
    void *tmp;
    if (vec->capacity > vec->size) {
        tmp = hz_vector_offset_of(vec, vec->size);
        tmp_mem = NULL;
    } else {
        tmp = hz_malloc(1, vec->element_size);
        tmp_mem = tmp;
    }

    for (size_t i = 0; i < vec->size / 2; ++i) {
        void *left = hz_vector_offset_of(vec, i);
        void *right = hz_vector_offset_of(vec, vec->size - i - 1);
        hz_memcpy(tmp, left, 1, vec->element_size);
        hz_memcpy(left, right, 1, vec->element_size);
        hz_memcpy(right, tmp, 1, vec->element_size);
    }

    // Free temporary buffer if we allocated one
    hz_free(tmp_mem);
}

void
hz_vector_sort(hz_vector *vec, hz_vector_cmp_func cmp_func)
{
    hz_check_null(vec);
    hz_check_null(cmp_func);
    if (vec->size != 0) {
        qsort(vec->buffer, vec->size, vec->element_size, cmp_func);
    }
}

bool
hz_vector_search(
    const hz_vector *vec,
    const void *value,
    hz_vector_cmp_func cmp_func,
    size_t *out_index)
{
    hz_check_null(vec);
    hz_check_null(value);
    for (size_t i = 0; i < vec->size; ++i) {
        void *buf_value = hz_vector_offset_of(vec, i);

        // If no comparator was provided, we perform a bitwise comparison
        // of the values. Otherwise, use the given comparator to
        // determine element equality.
        int cmp;
        if (cmp_func == NULL) {
            cmp = hz_memcmp(buf_value, value, 1, vec->element_size);
        } else {
            cmp = cmp_func(value, buf_value);
        }

        if (cmp == 0) {
            if (out_index != NULL) {
                *out_index = i;
            }
            return true;
        }
    }
    return false;
}

bool
hz_vector_bsearch(
    const hz_vector *vec,
    const void *value,
    hz_vector_cmp_func cmp_func,
    size_t *out_index)
{
    hz_check_null(vec);
    hz_check_null(value);
    hz_check_null(cmp_func);

    void *offset = bsearch(
        value, 
        vec->buffer, 
        vec->size,
        vec->element_size,
        cmp_func);

    if (offset == NULL) {
        return false;
    } else {
        // bsearch() returns a pointer to the element, so we have to
        // convert it back into an index
        *out_index = ((char *)offset - vec->buffer) / vec->element_size;
        return true;
    }
}

bool
hz_vector_equals(
    const hz_vector *a,
    const hz_vector *b,
    hz_vector_cmp_func cmp_func)
{
    if (a == b) {
        return true;
    }
    if (a == NULL || b == NULL) {
        return false;
    }
    if (a->element_size != b->element_size) {
        hz_abort("Vectors have diferent element types");
    }
    if (a->size != b->size) {
        return false;
    }

    // If no comparator function was provided, use a single memcmp()
    // over the entire buffer. Otherwise, we have to iterate over
    // the entries one-by-one.
    if (cmp_func == NULL) {
        return hz_memcmp(a->buffer, b->buffer, a->size, a->element_size) == 0;
    } else {
        for (size_t i = 0; i < a->size; ++i) {
            void *a_value = hz_vector_offset_of(a, i);
            void *b_value = hz_vector_offset_of(b, i);
            if (cmp_func(a_value, b_value) != 0) {
                return false;
            }
        }
        return true;
    }
}

void *
hz_vector_data(const hz_vector *vec)
{
    hz_check_null(vec);
    return vec->buffer;
}
