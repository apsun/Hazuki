#include "hazuki/utils.h"
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

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
hz_alloc_check_size(size_t num, size_t size)
{
    if (size == 0) {
        hz_abort("Allocation unit size is zero");
    } else if (num > SIZE_MAX / size) {
        hz_abort("Allocation size is too large: %zu * %zu", num, size);
    }
}

static void
hz_alloc_check_ptr(void *ptr)
{
    if (ptr == NULL) {
        hz_abort("Allocation returned null");
    }
}

void *
hz_malloc(size_t num, size_t size)
{
    hz_alloc_check_size(num, size);
    if (num == 0) {
        return NULL;
    }
    void *ptr = malloc(num * size);
    hz_alloc_check_ptr(ptr);
    return ptr;
}

void *
hz_calloc(size_t num, size_t size)
{
    hz_alloc_check_size(num, size);
    if (num == 0) {
        return NULL;
    }
    void *ptr = calloc(num, size);
    hz_alloc_check_ptr(ptr);
    return ptr;
}

void *
hz_realloc(void *ptr, size_t num, size_t size)
{
    hz_alloc_check_size(num, size);
    if (num == 0) {
        free(ptr);
        return NULL;
    }
    ptr = realloc(ptr, num * size);
    hz_alloc_check_ptr(ptr);
    return ptr;
}

void
hz_free(void *ptr)
{
    free(ptr);
}
