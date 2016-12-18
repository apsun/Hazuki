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

static void
test_utils_strncpy_basic(void)
{
    char buf[100] = "Some very long string";
    hz_assert_strncpy_ok(buf, "Test", 100);
    hz_assert_char(buf, 0, 'T');
    hz_assert_char(buf, 3, 't');
    hz_assert_char(buf, 4, '\0');
    hz_assert_char(buf, 5, 'v');
    hz_assert_char(buf, 6, 'e');
}

static void
test_utils_strncpy_overflow(void)
{
    char buf[10] = "AAAAAAAAA";
    hz_assert_strncpy_err(buf, "LONG LONG LONG STRING", 10);
}

static void
test_utils_strncpy_1(void)
{
    char buf[1];
    hz_assert_strncpy_ok(buf, "", 1);
    hz_assert_char(buf, 0, '\0');
}

static void
test_utils_strncpy_0(void)
{
    char buf[1];
    hz_assert_strncpy_err(buf, "", 0);
}

static void
test_utils_strncpy_concat(void)
{
    char buf[10] = "ABCDEFGHI";
    char *next = hz_assert_strncpy_ok(buf, "Megane", 10);
    hz_assert_strncpy_ok(next, "Poi", 10 - (next - buf));
    hz_assert_str_eq(buf, "MeganePoi");
}

static void
test_utils_strncpy_concat_loop(void)
{
    char buf[100];
    char *dest = buf;
    char *strs[4] = { "Alpha", "Beta", "Charlie", "Delta" };
    for (int i = 0; i < 4; ++i) {
        dest = hz_assert_strncpy_ok(dest, strs[i], sizeof(buf) - (dest - buf));
    }
    hz_assert_str_eq(buf, "AlphaBetaCharlieDelta");
}

void
test_utils(void)
{
    test_utils_strncpy_basic();
    test_utils_strncpy_overflow();
    test_utils_strncpy_1();
    test_utils_strncpy_0();
    test_utils_strncpy_concat();
    test_utils_strncpy_concat_loop();
    printf("All utils tests passed!\n");
}
