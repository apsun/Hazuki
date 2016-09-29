#include "hazuki/utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void
hz_assert_char(const char *buf, size_t i, char expected)
{
    if (buf[i] != expected) {
        hz_abort("Char at [%zu] equal to %d ('%c'), expected %d ('%c')", i, buf[i], buf[i], expected, expected);
    }
}

static char *
hz_assert_strncpy_ok(char *dest, const char *src, size_t count)
{
    char *ret = hz_strncpy(dest, src, count);
    if (ret == NULL) {
        hz_abort("strncpy failed when it shouldn't have");
    }
    return ret;
}

static void
hz_assert_strncpy_err(char *dest, const char *src, size_t count)
{
    if (hz_strncpy(dest, src, count) != NULL) {
        hz_abort("strncpy succeeded when it shouldn't have");
    }
}

static void
hz_assert_str_eq(const char *a, const char *b)
{
    if (strcmp(a, b) != 0) {
        hz_abort("Strings not equal: '%s' vs. '%s'");
    }
}

void
test_utils(void)
{
    char buf1[100] = "Some very long string";
    hz_assert_strncpy_ok(buf1, "Test", 100);
    hz_assert_char(buf1, 0, 'T');
    hz_assert_char(buf1, 3, 't');
    hz_assert_char(buf1, 4, '\0');
    hz_assert_char(buf1, 5, 'v');
    hz_assert_char(buf1, 6, 'e');

    char buf2[10] = "AAAAAAAAA";
    hz_assert_strncpy_err(buf2, "LONG LONG LONG STRING", 10);

    char buf3[1];
    hz_assert_strncpy_ok(buf3, "", 1);
    hz_assert_char(buf3, 0, '\0');

    char buf4[1];
    hz_assert_strncpy_err(buf4, "", 0);

    char buf5[10] = "ABCDEFGHI";
    char *next = hz_assert_strncpy_ok(buf5, "Megane", 10);
    hz_assert_strncpy_ok(next, "Poi", 10 - (next - buf5));
    hz_assert_str_eq(buf5, "MeganePoi");

    char buf6[100];
    char *dest6 = buf6;
    char *strs6[4] = { "Alpha", "Beta", "Charlie", "Delta" };
    for (int i = 0; i < 4; ++i) {
        dest6 = hz_assert_strncpy_ok(dest6, strs6[i], sizeof(buf6) - (dest6 - buf6));
    }
    hz_assert_str_eq(buf6, "AlphaBetaCharlieDelta");

    printf("All utils tests passed!\n");
}
