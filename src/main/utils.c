#include "hazuki/utils.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

void hz_abort(const char *msg, ...)
{
    va_list args;
    va_start(args, msg);
    vfprintf(stderr, msg, args);
    va_end(args);
    fprintf(stderr, "\n");
    abort();
}
