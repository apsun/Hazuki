#include "hazuki/utils.h"
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void
hz_abort(const char *msg, ...)
{
    fprintf(stderr, "ABORT: ");
    va_list args;
    va_start(args, msg);
    vfprintf(stderr, msg, args);
    va_end(args);
    fprintf(stderr, "\n");
    fflush(stderr);
    abort();
}

static void
hz_check_size(size_t num, size_t size)
{
    if (size == 0) {
        hz_abort("Unit size is zero");
    } else if (num > SIZE_MAX / size) {
        hz_abort("Size is too large: %zu * %zu", num, size);
    }
}

static void
hz_check_alloc(const void *ptr)
{
    if (ptr == NULL) {
        hz_abort("Allocation returned null");
    }
}

static void
hz_check_copy(const void *dest, const void *src)
{
    if (dest == NULL) {
        hz_abort("Copy destination is null");
    } else if (src == NULL) {
        hz_abort("Copy source is null");
    }
}

void *
hz_malloc(size_t num, size_t size)
{
    hz_check_size(num, size);
    if (num == 0) {
        return NULL;
    }
    void *ptr = malloc(num * size);
    hz_check_alloc(ptr);
    return ptr;
}

void *
hz_calloc(size_t num, size_t size)
{
    hz_check_size(num, size);
    if (num == 0) {
        return NULL;
    }
    void *ptr = calloc(num, size);
    hz_check_alloc(ptr);
    return ptr;
}

void *
hz_realloc(void *ptr, size_t num, size_t size)
{
    hz_check_size(num, size);
    if (num == 0) {
        free(ptr);
        return NULL;
    }
    ptr = realloc(ptr, num * size);
    hz_check_alloc(ptr);
    return ptr;
}

void
hz_free(void *ptr)
{
    free(ptr);
}

void
hz_memcpy(void *dest, const void *src, size_t num, size_t size)
{
    hz_check_size(num, size);
    if (num == 0) {
        return;
    }
    hz_check_copy(dest, src);
    memcpy(dest, src, num * size);
}

void
hz_memmove(void *dest, const void *src, size_t num, size_t size)
{
    hz_check_size(num, size);
    if (num == 0) {
        return;
    }
    hz_check_copy(dest, src);
    memmove(dest, src, num * size);
}

char *
hz_strncpy(char *dest, const char *src, size_t count)
{
    hz_check_copy(dest, src);
    while (count-- > 0) {
        if ((*dest = *src) == '\0') {
            return dest;
        }
        dest++;
        src++;
    }
    return NULL;
}
