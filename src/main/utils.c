#include "hazuki/utils.h"
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void
hz_abort(const char *msg, ...)
{
    va_list args;
    fprintf(stderr, "ABORT: ");
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

void *
hz_malloc(size_t num, size_t size)
{
    hz_check_size(num, size);
    if (num == 0) {
        return NULL;
    }
    void *ptr = malloc(num * size);
    if (ptr == NULL) {
        hz_abort("malloc returned null");
    }
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
    if (ptr == NULL) {
        hz_abort("calloc returned null");
    }
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
    if (ptr == NULL) {
        hz_abort("realloc returned null");
    }
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
    } else if (dest == NULL) {
        hz_abort("memcpy destination is null");
    } else if (src == NULL) {
        hz_abort("memcpy source is null");
    }
    memcpy(dest, src, num * size);
}

void
hz_memmove(void *dest, const void *src, size_t num, size_t size)
{
    hz_check_size(num, size);
    if (num == 0) {
        return;
    } else if (dest == NULL) {
        hz_abort("memmove destination is null");
    } else if (src == NULL) {
        hz_abort("memmove source is null");
    }
    memmove(dest, src, num * size);
}

bool
hz_strncpy(char *dest, const char *src, size_t count)
{
    if (dest == NULL) {
        hz_abort("strncpy destination is null");
    } else if (src == NULL) {
        hz_abort("strncpy source is null");
    }
    while (count-- > 0) {
        if ((*dest++ = *src++) == '\0') {
            return true;
        }
    }
    return false;
}
