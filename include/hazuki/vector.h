#ifndef HAZUKI_VECTOR_H_INCLUDED
#define HAZUKI_VECTOR_H_INCLUDED

#include <stdbool.h>
#include <stddef.h>

typedef struct hz_vector hz_vector;

/**
 * Creates a new empty vector. You must free the vector using
 * hz_vector_free(hz_vector *).
 */
hz_vector *
hz_vector_new(void);

/**
 * Creates a new vector by copying an existing one. You must free the vector
 * using hz_vector_free(hz_vector *).
 */
hz_vector *
hz_vector_copy(const hz_vector *vec);

/**
 * Frees a vector created by hz_vector_new(void) or
 * hz_vector_copy(hz_vector *). This does *NOT* free the elements contained
 * in the vector. Using the vector after deletion results in undefined
 * behavior.
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
 * vector will be truncated.
 */
void
hz_vector_resize(hz_vector *vec, size_t size, void *fill);

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
 * Removes all elements from the vector. This does *NOT* free the elements
 * contained in the vector.
 */
void
hz_vector_clear(hz_vector *vec);

/**
 * Gets the element in the vector at the given index. The index must be less
 * than the size of the vector.
 */
void *
hz_vector_get(const hz_vector *vec, size_t index);

/**
 * Sets the element in the vector at the given index and returns the element
 * originally at that index. The index must be less than the size of the
 * vector.
 */
void *
hz_vector_set(hz_vector *vec, size_t index, void *value);

/**
 * Appends an element to the end of the vector.
 */
void
hz_vector_append(hz_vector *vec, void *value);

/**
 * Inserts an element into the vector at the given index. The index must be
 * less than or equal to the size of the vector. If the index equals the size,
 * this is equivalent to calling hz_vector_append(hz_vector *, void *).
 */
void
hz_vector_insert(hz_vector *vec, size_t index, void *value);

/**
 * Removes and returns the element from the vector at the given index. The
 * index must be less than the size of the vector.
 */
void *
hz_vector_remove(hz_vector *vec, size_t index);

/**
 * Gets the index of the first occurence of an element in the vector. Returns
 * true if the element was found; false otherwise. If out_index is not NULL,
 * it is set to the index of the element if it is found. This function
 * compares elements based on *reference* equality (by comparing pointers).
 */
bool
hz_vector_find(const hz_vector *vec, const void *value, size_t *out_index);

/**
 * Returns the internal array buffer that holds the items in the vector. If
 * the vector has a size of zero, NULL will be returned. Accessing
 * elements at an index >= size of the vector results in undefined behavior.
 * Any changes in the buffer will be reflected in the vector. If you modify
 * the vector (by calling any non-const vector function), the buffer is
 * invalidated and using it results in undefined behavior. You must not free
 * the buffer.
 */
void **
hz_vector_data(hz_vector *vec);

#endif
