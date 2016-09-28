#include <stdio.h>
#include <stdlib.h>

extern void test_utils(void);
extern void test_map(void);
extern void test_vector(void);

int
main()
{
    test_utils();
    printf("\n");

    test_map();
    printf("\n");

    test_vector();
    printf("\n");

    printf("All tests passed!\n");
    getchar();
    return 0;
}
