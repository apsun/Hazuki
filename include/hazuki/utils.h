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
 * to calling free(void *) on the pointer directly. Calling hz_free
 * on a NULL pointer is a no-op.
 */
void
hz_free(void *ptr);

/**
 * Copies a non-overlapping block of memory from src to dest.
 * If num == 0, dest and/or src may be NULL. The unit size must
 * not be 0. If num != 0, hz_memcpy(dest, src, num, sizeof(T)) is
 * equivalent to memcpy(dest, src, num * sizeof(T)), except that if
 * dest or src is NULL or num * sizeof(T) overflows, the program is
 * aborted. Behavior is undefined if the memory regions overlap.
 */
void
hz_memcpy(void *dest, const void *src, size_t num, size_t size);

/**
 * Copies a potentially overlappting block of memory from src to
 * dest. If num == 0, dest and/or src may be NULL. The unit size must
 * not be 0. If num != 0, hz_memmove(dest, src, num, sizeof(T)) is
 * equivalent to memmove(dest, src, num * sizeof(T)), except that if
 * dest or src is NULL or num * sizeof(T) overflows, the program is
 * aborted.
 */
void
hz_memmove(void *dest, const void *src, size_t num, size_t size);

/**
 * Copies a non-overlapping string from src to dest. Unlike strncpy, this
 * function does *not* zero out extra space in the destination buffer beyond
 * the null terminator. If there is not enough space in the destination buffer
 * to hold the string, NULL is returned; otherwise, the address of the *null
 * terminator* is returned. You may therefore calculate the length of the
 * copied string using (return value) - (dest), which can be useful when
 * concatenating strings:
 *
 * char buf[100];
 * char *dest = buf;
 * while (...) {
 *     dest = hz_strncpy(dest, src, sizeof(buf) - (dest - buf));
 * }
 *
 * If dest or src is NULL, the program is aborted. Behavior is undefined if
 * the memory regions overlap.
 */
char *
hz_strncpy(char *dest, const char *src, size_t count);

#endif
