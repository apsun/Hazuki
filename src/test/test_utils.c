#include "hazuki/utils.h"
#include <stdio.h>
#include <stdlib.h>

static void
hz_assert_char(const char *buf, size_t i, char expected)
{
    if (buf[i] != expected) {
        hz_abort("Char at [%zu] equal to %d ('%c'), expected %d ('%c')", i, buf[i], buf[i], expected, expected);
    }
}

static void
hz_assert_strncpy_ok(char *dest, const char *src, size_t count)
{
    if (!hz_strncpy(dest, src, count)) {
        hz_abort("strncpy failed when it shouldn't have");
    }
}

static void
hz_assert_strncpy_err(char *dest, const char *src, size_t count)
{
    if (hz_strncpy(dest, src, count)) {
        hz_abort("strncpy succeeded when it shouldn't have");
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

    printf("All utils tests passed!\n");
}
