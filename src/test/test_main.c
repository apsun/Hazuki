#include <stdio.h>
#include <stdlib.h>

extern void test_utils(void);
extern void test_vector(void);
extern void test_map(void);

int
main(void)
{
    test_utils();
    test_vector();
    test_map();
    printf("All tests passed!\n");
    return 0;
}
