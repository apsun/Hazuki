#include "hazuki/vector.h"
#include "hazuki/utils.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define INITIAL_CAPACITY 8
#define SCALING_FACTOR 2

struct hz_vector
{
    size_t size;
    size_t capacity;
    void **elements;
};

static void hz_vector_check_null(const hz_vector *vec)
{
    if (vec == NULL) {
        hz_abort("Vector is null");
    }
}

static void hz_vector_check_index(const hz_vector *vec, size_t index)
{
    if (index >= vec->size) {
        hz_abort("Invalid vector index: %zu (size: %zu)", index, vec->size);
    }
}

static void **hz_vector_realloc_elements(void **old_elements, size_t new_capacity)
{
    if (new_capacity == 0) {
        free(old_elements);
        return NULL;
    }
    if (new_capacity > SIZE_MAX / sizeof(void *)) {
        hz_abort("New capacity is too large (%d)", new_capacity);
    }
    void **new_elements = realloc(old_elements, new_capacity * sizeof(void *));
    if (new_elements == NULL) {
        hz_abort("Failed to allocate new element buffer");
    }
    return new_elements;
}

static void hz_vector_resize(hz_vector *vec, size_t new_capacity)
{
    vec->elements = hz_vector_realloc_elements(vec->elements, new_capacity);
    vec->capacity = new_capacity;
}

static size_t hz_vector_next_capacity(size_t current_capacity)
{
    if (current_capacity == SIZE_MAX) {
        hz_abort("Cannot resize vector larger than SIZE_MAX elements");
        return 0;
    } else if (current_capacity == 0) {
        return INITIAL_CAPACITY;
    } else if (current_capacity > SIZE_MAX / SCALING_FACTOR) {
        return SIZE_MAX;
    } else {
        return current_capacity * SCALING_FACTOR;
    }
}

static void hz_vector_grow_if_full(hz_vector *vec)
{
    if (vec->size == vec->capacity) {
        size_t new_capacity = hz_vector_next_capacity(vec->capacity);
        hz_vector_resize(vec, new_capacity);
    }
}

hz_vector *hz_vector_new(void)
{
    hz_vector *vec = malloc(sizeof(hz_vector));
    vec->size = 0;
    vec->capacity = 0;
    vec->elements = NULL;
    return vec;
}

hz_vector *hz_vector_copy(const hz_vector *vec)
{
    hz_vector *new_vec = malloc(sizeof(hz_vector));
    new_vec->size = vec->size;
    new_vec->capacity = vec->capacity;
    new_vec->elements = hz_vector_realloc_elements(NULL, new_vec->capacity);
    if (vec->size > 0) {
        memcpy(new_vec->elements, vec->elements, vec->size * sizeof(void *));
    }
    return new_vec;
}

void hz_vector_free(hz_vector *vec)
{
    hz_vector_check_null(vec);
    free(vec->elements);
    free(vec);
}

size_t hz_vector_size(const hz_vector *vec)
{
    hz_vector_check_null(vec);
    return vec->size;
}

size_t hz_vector_capacity(const hz_vector *vec)
{
    hz_vector_check_null(vec);
    return vec->capacity;
}

void hz_vector_reserve(hz_vector *vec, size_t capacity)
{
    hz_vector_check_null(vec);
    if (capacity <= vec->capacity) {
        return;
    }
    hz_vector_resize(vec, capacity);
}

void hz_vector_trim(hz_vector *vec)
{
    hz_vector_check_null(vec);
    hz_vector_resize(vec, vec->size);
}

void hz_vector_clear(hz_vector *vec)
{
    hz_vector_check_null(vec);
    vec->size = 0;
}

void *hz_vector_get(const hz_vector *vec, size_t index)
{
    hz_vector_check_null(vec);
    hz_vector_check_index(vec, index);
    return vec->elements[index];
}

void hz_vector_set(hz_vector *vec, size_t index, void *value)
{
    hz_vector_check_null(vec);
    hz_vector_check_index(vec, index);
    vec->elements[index] = value;
}

void hz_vector_append(hz_vector *vec, void *value)
{
    hz_vector_check_null(vec);
    hz_vector_grow_if_full(vec);
    vec->elements[vec->size++] = value;
}

void hz_vector_insert(hz_vector *vec, size_t index, void *value)
{
    hz_vector_check_null(vec);
    if (index == vec->size) {
        hz_vector_append(vec, value);
    } else {
        hz_vector_check_index(vec, index);
        hz_vector_grow_if_full(vec);
        void **elements = vec->elements;
        size_t count = vec->size - index;
        memmove(&elements[index + 1], &elements[index], count * sizeof(void *));
        vec->elements[index] = value;
        vec->size++;
    }
}

void hz_vector_remove(hz_vector *vec, size_t index)
{
    hz_vector_check_null(vec);
    hz_vector_check_index(vec, index);
    void **elements = vec->elements;
    size_t count = vec->size - index - 1;
    memmove(&elements[index], &elements[index + 1], count * sizeof(void *));
    vec->size--;
}

bool hz_vector_find(const hz_vector *vec, void *value, size_t *out_index)
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
