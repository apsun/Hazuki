#include "hazuki/vector.h"
#include "hazuki/utils.h"
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#define INITIAL_CAPACITY 8
#define SCALING_FACTOR 1.5

struct hz_vector
{
    size_t size;
    size_t capacity;
    void **elements;
};

static void
hz_vector_check_null(const hz_vector *vec)
{
    if (vec == NULL) {
        hz_abort("Vector is null");
    }
}

static void
hz_vector_check_index(const hz_vector *vec, size_t index)
{
    if (index >= vec->size) {
        hz_abort("Invalid vector index: %zu (size: %zu)", index, vec->size);
    }
}

static void
hz_vector_resize_capacity(hz_vector *vec, size_t new_capacity)
{
    vec->elements = hz_realloc(vec->elements, new_capacity, sizeof(void *));
    vec->capacity = new_capacity;
}

static size_t
hz_vector_next_capacity(size_t current_capacity)
{
    if (current_capacity == SIZE_MAX) {
        hz_abort("Cannot resize vector larger than %zu elements", SIZE_MAX);
        return 0;
    } else if (current_capacity == 0) {
        return INITIAL_CAPACITY;
    } else if (current_capacity > (size_t)(SIZE_MAX / SCALING_FACTOR)) {
        return SIZE_MAX;
    } else {
        return (size_t)(current_capacity * SCALING_FACTOR);
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

hz_vector *
hz_vector_new(void)
{
    hz_vector *vec = hz_malloc(1, sizeof(hz_vector));
    vec->size = 0;
    vec->capacity = 0;
    vec->elements = NULL;
    return vec;
}

hz_vector *
hz_vector_copy(const hz_vector *vec)
{
    hz_vector_check_null(vec);
    hz_vector *new_vec = hz_malloc(1, sizeof(hz_vector));
    new_vec->size = vec->size;
    new_vec->capacity = vec->capacity;
    new_vec->elements = hz_malloc(new_vec->capacity, sizeof(void *));
    if (vec->size > 0) {
        memcpy(new_vec->elements, vec->elements, vec->size * sizeof(void *));
    }
    return new_vec;
}

void
hz_vector_free(hz_vector *vec)
{
    hz_vector_check_null(vec);
    hz_free(vec->elements);
    hz_free(vec);
}

size_t
hz_vector_size(const hz_vector *vec)
{
    hz_vector_check_null(vec);
    return vec->size;
}

size_t
hz_vector_capacity(const hz_vector *vec)
{
    hz_vector_check_null(vec);
    return vec->capacity;
}

void
hz_vector_resize(hz_vector *vec, size_t size, void *fill)
{
    hz_vector_check_null(vec);
    hz_vector_reserve(vec, size);
    for (size_t i = vec->size; i < size; ++i) {
        vec->elements[i] = fill;
    }
    vec->size = size;
}

void
hz_vector_reserve(hz_vector *vec, size_t capacity)
{
    hz_vector_check_null(vec);
    if (capacity > vec->capacity) {
        hz_vector_resize_capacity(vec, capacity);
    }
}

void
hz_vector_trim(hz_vector *vec)
{
    hz_vector_check_null(vec);
    hz_vector_resize_capacity(vec, vec->size);
}

void
hz_vector_clear(hz_vector *vec)
{
    hz_vector_check_null(vec);
    vec->size = 0;
}

void *
hz_vector_get(const hz_vector *vec, size_t index)
{
    hz_vector_check_null(vec);
    hz_vector_check_index(vec, index);
    return vec->elements[index];
}

void *
hz_vector_set(hz_vector *vec, size_t index, void *value)
{
    hz_vector_check_null(vec);
    hz_vector_check_index(vec, index);
    void *old_value = vec->elements[index];
    vec->elements[index] = value;
    return old_value;
}

void
hz_vector_append(hz_vector *vec, void *value)
{
    hz_vector_check_null(vec);
    hz_vector_grow_if_full(vec);
    vec->elements[vec->size++] = value;
}

void
hz_vector_insert(hz_vector *vec, size_t index, void *value)
{
    hz_vector_check_null(vec);
    if (index == vec->size) {
        hz_vector_append(vec, value);
    } else {
        hz_vector_check_index(vec, index);
        hz_vector_grow_if_full(vec);
        void **elements = vec->elements;
        size_t num = vec->size - index;
        memmove(&elements[index + 1], &elements[index], num * sizeof(void *));
        vec->elements[index] = value;
        vec->size++;
    }
}

void *
hz_vector_remove(hz_vector *vec, size_t index)
{
    hz_vector_check_null(vec);
    hz_vector_check_index(vec, index);
    void **elements = vec->elements;
    void *old_value = elements[index];
    size_t num = vec->size - index - 1;
    memmove(&elements[index], &elements[index + 1], num * sizeof(void *));
    vec->size--;
    return old_value;
}

bool
hz_vector_find(const hz_vector *vec, const void *value, size_t *out_index)
{
    hz_vector_check_null(vec);
    for (size_t i = 0; i < vec->size; ++i) {
        if (vec->elements[i] == value) {
            if (out_index != NULL) {
                *out_index = i;
            }
            return true;
        }
    }
    return false;
}

void **
hz_vector_data(hz_vector *vec)
{
    hz_vector_check_null(vec);
    if (vec->size == 0) {
        return NULL;
    } else {
        return vec->elements;
    }
}
