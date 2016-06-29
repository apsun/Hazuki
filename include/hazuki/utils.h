#ifndef HAZUKI_UTILS_H_INCLUDED
#define HAZUKI_UTILS_H_INCLUDED

#include <stddef.h>

/**
 * Prints a formatted message to stderr and aborts the program.
 */
void
hz_abort(const char *msg, ...);

/**
 * Allocates a block of memory, with overflow and failure checking.
 * If num == 0, NULL is returned. The unit size must not be 0.
 * If num != 0, hz_malloc(num, sizeof(T)) is equivalent to
 * malloc(num * sizeof(T)), except that if num * sizeof(T) overflows
 * or malloc returns NULL, the program is aborted.
 */
void *
hz_malloc(size_t num, size_t size);

/**
 * Allocates a zero-initialized block of memory, with overflow
 * and failure checking. If num == 0, NULL is returned. The unit
 * size must not be 0. If num != 0, hz_calloc(num, sizeof(T))
 * is equivalent to calloc(num, sizeof(T)), except that if
 * num * sizeof(T) overflows or calloc returns NULL, the program
 * is aborted.
 */
void *
hz_calloc(size_t num, size_t size);

/**
 * Reallocates a block of memory, with overflow and failure checking.
 * If num == 0, NULL is returned. The unit size must not be 0.
 * If num != 0, hz_realloc(ptr, num, sizeof(T)) is equivalent to
 * realloc(ptr, num * sizeof(T)), except that if num * sizeof(T)
 * overflows or realloc returns NULL, the program is aborted.
 */
void *
hz_realloc(void *ptr, size_t num, size_t size);

/**
 * Frees a block of memory allocated by hz_malloc, hz_calloc, or
 * hz_realloc. This is only provided for symmetry; it is equivalent
 * to calling free(void *) on the pointer directly.
 */
void
hz_free(void *ptr);

#endif
