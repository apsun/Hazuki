#ifndef HAZUKI_VECTOR_H_INCLUDED
#define HAZUKI_VECTOR_H_INCLUDED

#include <stdbool.h>
#include <stddef.h>

/**
 * A self-growing array of items.
 *
 * To create a vector, call hz_vector_new() with the size of the element.
 * To free the vector, call hz_vector_free():
 *
 * hz_vector *vec = hz_vector_new(sizeof(T));
 * ...
 * hz_vector_free(vec);
 *
 * To access items in the vector, use hz_vector_get(), hz_vector_set(),
 * hz_vector_append(), hz_vector_insert(), and hz_vector_remove():
 *
 * T value = ...
 * T out_value;
 * hz_vector_append(vec, &value);
 * hz_vector_insert(vec, 0, &value);
 * hz_vector_remove(vec, 0);
 * hz_vector_set(vec, 0, &value);
 * hz_vector_get(vec, 0, &out_value);
 *
 * To iterate the vector, use hz_vector_get() in a for loop:
 *
 * T value;
 * size_t size = hz_vector_size(vec);
 * for (size_t i = 0; i < size; ++i) {
 *     hz_vector_get(vec, i, &value);
 *     ...
 * }
 *
 * You can import/export data from/to an array using hz_vector_resize()
 * and hz_vector_data():
 *
 * hz_vector *vec = hz_vector_new(sizeof(T));
 * hz_vector_resize(vec, N, NULL);
 * T *buf = hz_vector_data(vec);
 *
 * T src[N] = { ... };
 * hz_memcpy(buf, src, N, sizeof(T));
 *
 * T dest[N];
 * hz_memcpy(dest, buf, N, sizeof(T));
 */
typedef struct hz_vector hz_vector;

/**
 * Creates a new empty vector with the specified element size. You must free
 * the returned vector using hz_vector_free().
 */
hz_vector *
hz_vector_new(size_t element_size);

/**
 * Creates a new vector by copying an existing one. You must free the returned
 * vector using hz_vector_free().
 */
hz_vector *
hz_vector_copy(const hz_vector *vec);

/**
 * Frees a vector created by hz_vector_new() or hz_vector_copy(). Using the
 * vector after deletion results in undefined behavior.
 */
void
hz_vector_free(hz_vector *vec);

/**
 * Gets the number of elements in the vector.
 */
size_t
hz_vector_size(const hz_vector *vec);

/**
 * Gets the maximum number of elements the vector can hold before resizing.
 */
size_t
hz_vector_capacity(const hz_vector *vec);

/**
 * Resizes the vector to the specified size, filling in any new elements with
 * the specified value. If the new size is smaller than the current size, the
 * vector will be truncated. If fill is NULL, any new elements have undefined
 * value until written to for the first time. This behavior should only be used
 * in conjunction with hz_vector_data() to initialize the elements directly via
 * the buffer.
 */
void
hz_vector_resize(hz_vector *vec, size_t size, const void *fill);

/**
 * Increases the vector's capacity to at least the specified value. If the new
 * capacity is less than the current capacity, this function does nothing.
 * This does *NOT* add or remove any elements from the vector, it is only
 * useful for performance optimization.
 */
void
hz_vector_reserve(hz_vector *vec, size_t capacity);

/**
 * Shrinks the vector's capacity to match its size.
 */
void
hz_vector_trim(hz_vector *vec);

/**
 * Removes all elements from the vector.
 */
void
hz_vector_clear(hz_vector *vec);

/**
 * Gets the element in the vector at the given index. The index must be less
 * than the size of the vector.
 */
void
hz_vector_get(const hz_vector *vec, size_t index, void *out_value);

/**
 * Sets the element in the vector at the given index. The index must be less
 * than the size of the vector.
 */
void
hz_vector_set(hz_vector *vec, size_t index, const void *value);

/**
 * Appends an element to the end of the vector.
 */
void
hz_vector_append(hz_vector *vec, const void *value);

/**
 * Inserts an element into the vector at the given index. The index must be
 * less than or equal to the size of the vector. If the index equals the size,
 * this is equivalent to calling hz_vector_append().
 */
void
hz_vector_insert(hz_vector *vec, size_t index, const void *value);

/**
 * Removes the element from the vector at the given index. The
 * index must be less than the size of the vector.
 */
void
hz_vector_remove(hz_vector *vec, size_t index);

/**
 * Gets the index of the first occurence of an element in the vector. Returns
 * true if the element was found, and false otherwise. If the element was
 * found and out_index is not NULL, the index of the element is written to
 * out_index.
 */
bool
hz_vector_find(const hz_vector *vec, const void *value, size_t *out_index);

/**
 * Compares the two vectors. Returns true if all elements are equal, and false
 * otherwise. If either vector is NULL, the return value is true if and only if
 * the other vector is also NULL. If the vectors have different element types,
 * the behavior is undefined.
 */
bool
hz_vector_equals(const hz_vector *a, const hz_vector *b);

/**
 * Gets the internal array buffer that holds the items in the vector. Accessing
 * elements at an index >= size of the vector results in undefined behavior.
 * Any changes in the buffer will be reflected in the vector. If you modify
 * the vector (by calling any non-const vector function), the buffer is
 * invalidated and using it results in undefined behavior. You must not free
 * the buffer.
 */
void *
hz_vector_data(const hz_vector *vec);

#endif
